#include "REDocumentView.h"
#include "REMainWindow.h"
#include "REScoreScene.h"
#include "REScoreSceneView.h"
#include "REGraphicsPageItem.h"
#include "REGraphicsSystemItem.h"
#include "REGraphicsSliceItem.h"
#include "REPartListModel.h"
#include "RETrackListModel.h"
#include "RESectionListModel.h"
#include "REQtViewport.h"
#include "REUndoCommand.h"

#include <REGuitarProParser.h>
#include <REInputStream.h>
#include <REOutputStream.h>
#include <RESong.h>
#include <RESongController.h>
#include <RESystem.h>
#include <RENote.h>
#include <REChord.h>
#include <REBar.h>
#include <REPhrase.h>

#include "REPropertiesDialog.h"
#include "RERehearsalDialog.h"
#include "RERepeatDialog.h"
#include "RETimeSignatureDialog.h"
#include "REFilePropertiesDialog.h"

#include <QGraphicsTextItem>
#include <QVBoxLayout>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

using std::bind;

REDocumentView::REDocumentView(QWidget *parent) :
    QWidget(parent),
    _song(NULL), _songController(NULL), _scoreController(NULL), _scoreView(NULL), _scene(NULL), _viewport(NULL), _undoStack(NULL), _viewportUpdateTimer(NULL), _trackingEnabled(false)
{
	_undoStack = new QUndoStack(this);
    _viewportUpdateTimer = new QTimer(this);
    QObject::connect(_viewportUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateViewport()));
}

void REDocumentView::Save()
{
    if(_filename.isEmpty()) {
        SaveAs();
    }
    else if(WriteFLOW(_filename)) {
        _undoStack->setClean();
        emit FileStatusChanged();
    }
}

void REDocumentView::SaveAs()
{
    QString home = QDir::homePath();
    QString filter = "Reflow Files (*.flow)";
    QString path = QFileDialog::getSaveFileName(this, tr("Save File"), home, filter);
    if(!path.isEmpty()) {
        if(WriteFLOW(path))
        {
            _filename = path;
            _undoStack->setClean();
            emit FileStatusChanged();
        }
    }
}

void REDocumentView::InitializeWithEmptyDocument()
{
    // Create a default song
    _filename = "";
    _song = RESong::CreateDefaultSong();

    // Create controllers
    CreateControllers();
}

void REDocumentView::InitializeWithFile(const QString& filename)
{
    _filename = filename;
    _song = NULL;

    if(filename.endsWith(".gp3", Qt::CaseInsensitive) ||
        filename.endsWith(".gp4", Qt::CaseInsensitive) ||
        filename.endsWith(".gp5", Qt::CaseInsensitive))
    {
        _song = new RESong;
        LoadGP(*_song, filename);
        CreateControllers();
    }
	else if(filename.endsWith(".flow", Qt::CaseInsensitive))
    {
        _song = new RESong;
        LoadFLOW(*_song, filename);
        CreateControllers();
    }
}

void REDocumentView::LoadGP(RESong& song, QString filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    file.close();
    REBufferInputStream decoder(bytes.data(), bytes.size());
    decoder.SetVersion(REFLOW_IO_VERSION);
    REGuitarProParser parser;
    parser.Parse(&decoder, &song);

    for(unsigned int i=0; i<song.TrackCount(); ++i) {
        song.CreatePart(i);
    }
}

bool REDocumentView::LoadFLOW(RESong& song, QString filename)
{
	QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    file.close();
    REBufferInputStream decoder(bytes.data(), bytes.size());
    decoder.SetVersion(REFLOW_IO_VERSION);

	try 
    {
        decoder.SetSubType(REFLOW_IO_GENERIC);
        
        std::string header = decoder.ReadBytes(4);
        uint32_t version = decoder.ReadUInt32();
        
        if(header == "FLOW") {
            decoder.SetSubType(REFLOW_IO_REFLOW2);
        }
        else {
            REPrintf("This is not a valid file");
            return false;
        }
        
        decoder.SetVersion(version);
        
        if(version > REFLOW_IO_VERSION) {
            REPrintf("This file was created with a more recent file version (%d). Please update. (Current Version: %d)", version, REFLOW_IO_VERSION);
            return false;
        }
        
        REPrintf("Parsing File (subtype: %d - version: %d)...", decoder.SubType(), version);
        
        song.DecodeFrom(decoder);
        return true;
    }
    catch(std::exception& e) {
        REPrintf("Reflow File Loading Error: %s", e.what());
        return false;
    }
}

bool REDocumentView::WriteFLOW(QString filename)
{
    QFile file(filename);
    if(!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("Reflow Error"), tr("Failed to write file"));
        return false;
    }

    REBufferOutputStream buffer;
    buffer.SetVersion(REFLOW_IO_VERSION);
    buffer.SetSubType(REFLOW_IO_REFLOW2);

    buffer.Write("FLOW", 4);
    buffer.WriteUInt32(REFLOW_IO_VERSION);
    _song->EncodeTo(buffer);

    file.write(buffer.Data(), buffer.Size());
    file.close();
    return true;
}

void REDocumentView::CreateControllers()
{
    // Then create the song controller to edit it
    _songController = new RESongController(_song);
    _songController->AddDelegate(this);

    // Create a score scene
    _scene = new REScoreScene(this, this);
    _scene->setBackgroundBrush(QBrush(QColor(64, 64, 64)));

    // Add the score scene to the view
    _scoreView = new REScoreSceneView(_scene, 0);
    _scoreView->setStyleSheet( "QGraphicsView { border-style: none; }" );
    _scoreView->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(_scoreView);
    this->setLayout(layout);

    // Create score controller
    _scoreController = new REScoreController(_songController);
    _scoreController->SetDelegate(this);

    // Create Viewport
    _viewport = new REQtViewport(this);
    _scoreController->SetViewport(_viewport);
    _viewport->_CreateControllerItems();

    // Create the models
    _partListModel = new REPartListModel(this);
    _trackListModel = new RETrackListModel(this);
    _sectionListModel = new RESectionListModel(this);

    // Select first score
    _scoreController->SetScoreIndex(0);
}

void REDocumentView::DestroyControllers()
{
    StopPlayback();

    delete _scoreController; _scoreController = nullptr;
    delete _songController; _songController = nullptr;
}

bool REDocumentView::IsPlaybackRunning() const
{
	const RESequencer* seq = _songController->Sequencer();
	return seq && seq->IsRunning();
}

void REDocumentView::StartPlayback()
{
	RESequencer* sequencer = _songController->Sequencer();
	if(sequencer == NULL) return;

	_song->RefreshPlaylist();
	sequencer->Build(_song);
	sequencer->SetPlaybackRate(1.0);

	const RECursor& cursor = _scoreController->Cursor();
	int barIndex = cursor.BarIndex();
	int tickInBar = cursor.Tick();
	sequencer->JumpTo(barIndex, tickInBar);

	sequencer->StartPlayback();

    _viewportUpdateTimer->start(30);

    emit PlaybackStarted();
}
void REDocumentView::StopPlayback()
{
	RESequencer* sequencer = _songController->Sequencer();
	if(sequencer == NULL) return;

	sequencer->StopPlayback();
	sequencer->Shutdown();

    _viewportUpdateTimer->stop();

    UpdateViewport();
    emit PlaybackStopped();
}
void REDocumentView::TogglePlayback()
{
	if(IsPlaybackRunning()) {
		StopPlayback();
	}
	else {
		StartPlayback();
	}
}

void REDocumentView::SetTrackingEnabled(bool enabled)
{
    _trackingEnabled = enabled;
}

bool REDocumentView::IsEditingLowVoice() const
{
    return _scoreController->IsEditingLowVoice();
}

void REDocumentView::SetEditLowVoice(bool b)
{
    _scoreController->SetEditLowVoice(b);
    emit CursorOrSelectionChanged();
}

void REDocumentView::UpdateViewport()
{
    if(_scoreController == NULL) return;

    REViewport* vp = _scoreController->Viewport();
    if(vp) {
        static_cast<REQtViewport*>(vp)->UpdatePlaybackCursor(1.0 / 30.0);
    }
}

void REDocumentView::ActionAddChord()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::AddChordAfterCursor, std::placeholders::_1)));
}

void REDocumentView::ActionInsertChord()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::InsertChordBeforeCursor, std::placeholders::_1)));
}

void REDocumentView::ActionDuplicateChord()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::DuplicateSelectedChordsAtEnd, std::placeholders::_1)));
}

void REDocumentView::ActionDeleteChord()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::DeleteSelectedChords, std::placeholders::_1)));
}

void REDocumentView::ActionAddBar()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::AddBarAfterSelection, std::placeholders::_1)));
}

void REDocumentView::ActionInsertBar()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::InsertBarBeforeSelection, std::placeholders::_1)));
}

void REDocumentView::ActionDeleteBar()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::DeleteSelectedBars, std::placeholders::_1)));
}

void REDocumentView::ShowFilePropertiesDialog()
{
    REFilePropertiesDialog dlg(this);
    dlg.Initialize(Song());
    if(QDialog::Accepted == dlg.exec())
    {
        RESongOperationVector operations;
        operations.push_back(std::bind(&RESong::SetTitle, std::placeholders::_1, dlg.Title().toStdString()));
        operations.push_back(std::bind(&RESong::SetSubTitle, std::placeholders::_1, dlg.Subtitle().toStdString()));
        operations.push_back(std::bind(&RESong::SetArtist, std::placeholders::_1, dlg.Artist().toStdString()));
        operations.push_back(std::bind(&RESong::SetAlbum, std::placeholders::_1, dlg.Album().toStdString()));
        operations.push_back(std::bind(&RESong::SetMusicBy, std::placeholders::_1, dlg.MusicBy().toStdString()));
        operations.push_back(std::bind(&RESong::SetLyricsBy, std::placeholders::_1, dlg.LyricsBy().toStdString()));
        operations.push_back(std::bind(&RESong::SetTranscriber, std::placeholders::_1, dlg.Transcriber().toStdString()));
        operations.push_back(std::bind(&RESong::SetCopyright, std::placeholders::_1, dlg.Copyright().toStdString()));
        _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::PerformTasksOnSong, std::placeholders::_1, operations)));
    }
}

void REDocumentView::ShowTracksAndPartsDialog()
{
    REPropertiesDialog dlg(this);
    dlg.exec();
}

void REDocumentView::ShowTimeSignatureDialog()
{
    const REBar* bar = _scoreController->FirstSelectedBar();
    if(bar == nullptr) return;

    RETimeSignatureDialog dlg(this);
    dlg.Initialize(bar);

    if(dlg.exec() == QDialog::Accepted)
    {
        int den = dlg.Denominator();
        if(!Reflow::IsPowerOfTwo(dlg.Denominator()))
        {
            QMessageBox::critical(this, tr("Reflow Error"), tr("Time Signature Beat Type must be power of two"));
        }
        else
        {
            RETimeSignature timeSignature(dlg.Numerator(), dlg.Denominator());
            REBeamingPattern pattern = dlg.BeamingPattern();

            auto op = std::bind(&REScoreController::SetTimeSignatureAndBeamingPatternOfSelectedBars, std::placeholders::_1, timeSignature, pattern);
            _undoStack->push(new REScoreUndoCommand(_scoreController, op));
        }
    }
}

void REDocumentView::ShowClefDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ShowKeySignatureDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ShowRehearsalDialog()
{
    int barIndex = _scoreController->FirstSelectedBarIndex();
    const REBar* bar = _scoreController->FirstSelectedBar();
    if(bar == nullptr) return;

    RERehearsalDialog dlg(this);
    if(bar->HasFlag(REBar::RehearsalSign)) {
        dlg.setRehearsalText(QString::fromStdString(bar->RehearsalSignText()));
    }

    // Start dialog
    int code = dlg.exec();
    if(code == QDialog::Accepted)
    {
        QString text = dlg.rehearsalText();
        _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetRehearsalSignOnSelectedBar, std::placeholders::_1, text.toStdString())));
    }
    else if(code == 2) {
        _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::UnsetRehearsalSignOnSelectedBar, std::placeholders::_1)));
    }
}

void REDocumentView::ShowChordNameDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ShowChordDiagramDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ShowTextDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ShowRepeatDialog()
{
    const REBar* firstBar = _scoreController->FirstSelectedBar();
    const REBar* lastBar = _scoreController->LastSelectedBar();
    if(firstBar == nullptr || lastBar == nullptr) return;

    RERepeatDialog dlg(this);
    dlg.Initialize(firstBar, lastBar);

    int code = dlg.exec();
    if(code == QDialog::Accepted)
    {
        REBarOperationVector operationsOnFirstBar;
        REBarOperationVector operationsOnLastBar;

        // Repeats start / end / count
        operationsOnFirstBar.push_back(bind((dlg.RepeatStartType() == 1 ? &REBar::SetFlag : &REBar::UnsetFlag), std::placeholders::_1, REBar::RepeatStart));
        operationsOnLastBar.push_back(bind((dlg.RepeatEndType() == 1 ? &REBar::SetFlag : &REBar::UnsetFlag), std::placeholders::_1, REBar::RepeatEnd));
        operationsOnLastBar.push_back(bind(&REBar::SetRepeatCount, std::placeholders::_1, dlg.RepeatCount()));

        // Alternate Endings
        for(int i=0; i<8; ++i) {
            operationsOnLastBar.push_back(bind((dlg.HasAlternateEnding(i) ? &REBar::SetAlternateEnding : &REBar::UnsetAlternateEnding), std::placeholders::_1, i));
        }

        // Direction Targets
        operationsOnFirstBar.push_back(bind(dlg.Coda() ? &REBar::SetDirectionTarget : &REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Coda));
        operationsOnFirstBar.push_back(bind(dlg.DoubleCoda() ? &REBar::SetDirectionTarget : &REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::DoubleCoda));
        operationsOnFirstBar.push_back(bind(dlg.Segno() ? &REBar::SetDirectionTarget : &REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Segno));
        operationsOnFirstBar.push_back(bind(dlg.SegnoSegno() ? &REBar::SetDirectionTarget : &REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::SegnoSegno));
        operationsOnLastBar.push_back(bind(dlg.Fine() ? &REBar::SetDirectionTarget : &REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Fine));

        // Direction Jumps
        operationsOnLastBar.push_back(bind(&REBar::ClearDirectionJumps, std::placeholders::_1));
        if(dlg.JumpType() != 0) {
            operationsOnLastBar.push_back(bind(&REBar::SetDirectionJump, std::placeholders::_1, static_cast<Reflow::DirectionJump>(dlg.JumpType()-1)));
        }
        operationsOnLastBar.push_back(bind(dlg.ToCoda() ? &REBar::SetDirectionJump : &REBar::UnsetDirectionJump, std::placeholders::_1, Reflow::ToCoda));
        operationsOnLastBar.push_back(bind(dlg.ToDoubleCoda() ? &REBar::SetDirectionJump : &REBar::UnsetDirectionJump, std::placeholders::_1, Reflow::ToDoubleCoda));

        // Perform Operations
        auto op = std::bind(&REScoreController::PerformOperationsOnSelectedBars, std::placeholders::_1, operationsOnFirstBar, operationsOnLastBar, "Change Bar Repeats");
        _undoStack->push(new REScoreUndoCommand(_scoreController, op));
    }
    else if(code == 2)
    {
        REBarOperationVector operationsOnFirstBar;
        REBarOperationVector operationsOnLastBar;

        // Repeats start / end / count
        operationsOnFirstBar.push_back(bind(&REBar::UnsetFlag, std::placeholders::_1, REBar::RepeatStart));
        operationsOnLastBar.push_back(bind(&REBar::UnsetFlag, std::placeholders::_1, REBar::RepeatEnd));
        operationsOnLastBar.push_back(bind(&REBar::SetRepeatCount, std::placeholders::_1, 0));

        // Alternate Endings
        for(int i=0; i<8; ++i) {
            operationsOnLastBar.push_back(bind(&REBar::UnsetAlternateEnding, std::placeholders::_1, i));
        }

        // Direction Targets
        operationsOnFirstBar.push_back(bind(&REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Coda));
        operationsOnFirstBar.push_back(bind(&REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::DoubleCoda));
        operationsOnFirstBar.push_back(bind(&REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Segno));
        operationsOnFirstBar.push_back(bind(&REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::SegnoSegno));
        operationsOnLastBar.push_back(bind(&REBar::UnsetDirectionTarget, std::placeholders::_1, Reflow::Fine));

        // Direction Jumps
        operationsOnLastBar.push_back(bind(&REBar::ClearDirectionJumps, std::placeholders::_1));
        operationsOnLastBar.push_back(bind(&REBar::UnsetDirectionJump, std::placeholders::_1, Reflow::ToCoda));
        operationsOnLastBar.push_back(bind( &REBar::UnsetDirectionJump, std::placeholders::_1, Reflow::ToDoubleCoda));

        // Perform Operations
        auto op = std::bind(&REScoreController::PerformOperationsOnSelectedBars, std::placeholders::_1, operationsOnFirstBar, operationsOnLastBar, "Clear Bar Repeats");
        _undoStack->push(new REScoreUndoCommand(_scoreController, op));
    }
}

void REDocumentView::ActionSystemBreak()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSystemBreakOnSelection, std::placeholders::_1)));
}

void REDocumentView::ShowTempoMarkerDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ActionDoubleFlat()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAlterationOnSelection, std::placeholders::_1, -2)));
}

void REDocumentView::ActionFlat()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAlterationOnSelection, std::placeholders::_1, -1)));
}

void REDocumentView::ActionNatural()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAlterationOnSelection, std::placeholders::_1, 0)));
}

void REDocumentView::ActionSharp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAlterationOnSelection, std::placeholders::_1, 1)));
}

void REDocumentView::ActionDoubleSharp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAlterationOnSelection, std::placeholders::_1, 2)));
}

void REDocumentView::ActionSemitoneUp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::PitchUpOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionSemitoneDown()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::PitchDownOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionWholeNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::WholeNote)));
}

void REDocumentView::ActionHalfNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::HalfNote)));
}

void REDocumentView::ActionQuarterNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::QuarterNote)));
}

void REDocumentView::Action8thNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::EighthNote)));
}

void REDocumentView::Action16thNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::SixteenthNote)));
}

void REDocumentView::Action32ndNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::ThirtySecondNote)));
}

void REDocumentView::Action64thNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetNoteValueOnSelection, std::placeholders::_1, Reflow::SixtyFourthNote)));
}

void REDocumentView::ActionTuplet2()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 2)));
}

void REDocumentView::ActionTuplet3()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 3)));
}

void REDocumentView::ActionTuplet4()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 4)));
}

void REDocumentView::ActionTuplet5()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 5)));
}

void REDocumentView::ActionTuplet6()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 6)));
}

void REDocumentView::ActionTuplet7()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 7)));
}

void REDocumentView::ActionTuplet9()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTupletOnSelection, std::placeholders::_1, 9)));
}

void REDocumentView::ActionDottedNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleDottedNoteOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionDoubleDottedNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleDoubleDottedNoteOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionTiedNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTiedNoteOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionSplitChord()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SplitSelectedChords, std::placeholders::_1)));
}

void REDocumentView::ActionForceStemUp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleForceStemUpOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionForceStemDown()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleForceStemDownOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionToggleEnharmonicEquivalent()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleEnharmonicOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionDynamicPPP()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsPPP)));
}
void REDocumentView::ActionDynamicPP()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsPP)));
}
void REDocumentView::ActionDynamicP()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsP)));
}
void REDocumentView::ActionDynamicMP()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsMP)));
}
void REDocumentView::ActionDynamicMF()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsMF)));
}
void REDocumentView::ActionDynamicF()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsF)));
}
void REDocumentView::ActionDynamicFF()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsFF)));
}
void REDocumentView::ActionDynamicFFF()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::SetDynamicsOnSelection, std::placeholders::_1, Reflow::DynamicsFFF)));
}

void REDocumentView::ActionGhostNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleNoteFlagOnSelectedNotes, std::placeholders::_1, RENote::GhostNote)));
}

void REDocumentView::ActionAccent()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleAccentOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionStrongAccent()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleStrongAccentOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionStaccato()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleStaccatoOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionLeftHandSticking()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleNoteFlagOnSelectedNotes, std::placeholders::_1, RENote::LeftStick)));
}

void REDocumentView::ActionRightHandSticking()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleNoteFlagOnSelectedNotes, std::placeholders::_1, RENote::RightStick)));
}

void REDocumentView::ActionBrushUp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleBrushUpOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionBrushDown()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleBrushDownOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionArpeggioUp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleArpeggioUpOnSelection, std::placeholders::_1)));
}
void REDocumentView::ActionArpeggioDown()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleArpeggioDownOnSelection, std::placeholders::_1)));
}
void REDocumentView::ActionDeadNote()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::TypeDeadNote, std::placeholders::_1)));
}

void REDocumentView::ActionPickstrokeUp()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::TogglePickstrokeUpOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionPickstrokeDown()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::TogglePickstrokeDownOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionHammerOnPullOff()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleLegatoOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionTap()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleTapOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionSlap()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlapOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionPop()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::TogglePopOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionVibrato()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleVibratoOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionPalmMute()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::TogglePalmMuteOnSelection, std::placeholders::_1)));
}

void REDocumentView::ActionLetRing()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleLetRingOnSelection, std::placeholders::_1)));
}

void REDocumentView::ShowBendDialog()
{
    QMessageBox::critical(this, tr("Reflow Error"), tr("This feature is not yet implemented"));
}

void REDocumentView::ActionShiftSlide()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlideOutOnSelection, std::placeholders::_1, Reflow::ShiftSlide)));
}
void REDocumentView::ActionSlideOutHigh()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlideOutOnSelection, std::placeholders::_1, Reflow::SlideOutHigh)));
}
void REDocumentView::ActionSlideOutLow()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlideOutOnSelection, std::placeholders::_1, Reflow::SlideOutLow)));
}
void REDocumentView::ActionSlideInHigh()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlideInOnSelection, std::placeholders::_1, Reflow::SlideInFromAbove)));
}
void REDocumentView::ActionSlideInLow()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::ToggleSlideInOnSelection, std::placeholders::_1, Reflow::SlideInFromBelow)));
}

void REDocumentView::ActionCut()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::Cut, std::placeholders::_1, false)));
}

void REDocumentView::ActionCopy()
{
    _scoreController->Copy(false);
}

void REDocumentView::ActionPaste()
{
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if(mimeData->hasFormat("application/x-reflow-phrase"))
    {
        QByteArray data = mimeData->data("application/x-reflow-phrase");
        std::string encodedPhrase(data.data(), data.size());

        _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::PasteEncodedPhrase, std::placeholders::_1, encodedPhrase)));
    }

    else if(mimeData->hasFormat("application/x-reflow-psong")) {
        QByteArray data = mimeData->data("application/x-reflow-psong");
        std::string encodedPartialSong(data.data(), data.size());

        _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::PasteEncodedPartialSong, std::placeholders::_1, encodedPartialSong)));
    }
}

void REDocumentView::ActionDelete()
{
    _undoStack->push(new REScoreUndoCommand(_scoreController, std::bind(&REScoreController::DeleteSelection, std::placeholders::_1)));
}

void REDocumentView::ClickedOnPart(QModelIndex idx)
{
    int partIndex = idx.row();
    _scoreController->SetScoreIndex(partIndex);
}

// ----------------------------------------------------------------------------
//
//    RESongControllerDelegate
//
// ----------------------------------------------------------------------------
void REDocumentView::SongControllerWillModifySong(const RESongController* controller, const RESong* song)
{
    _trackListModel->beginResetModel();
    _partListModel->beginResetModel();
    _sectionListModel->beginResetModel();
}

void REDocumentView::SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully)
{
    _trackListModel->endResetModel();
    _partListModel->endResetModel();
    _sectionListModel->endResetModel();
}

void REDocumentView::SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully)
{
}

void REDocumentView::SongControllerWantsToBackup(const RESongController* controller, const RESong* song)
{
}

// ----------------------------------------------------------------------------
//
//    REScoreControllerDelegate
//
// ----------------------------------------------------------------------------
void REDocumentView::ScoreControllerSelectionDidChange(const REScoreController* scoreController)
{
    emit CursorOrSelectionChanged();
}

void REDocumentView::ScoreControllerScoreDidChange(const REScoreController* scoreController, const REScore* score)
{
    qDebug() << "score did change to " << scoreController->ScoreIndex();
}

void REDocumentView::ScoreControllerRefreshPresentation(const REScoreController* scoreController, const REScore* score)
{
    emit DataChanged();
}

void REDocumentView::OnShouldCenterOnCursor(const REScoreController* scoreController)
{
    emit CursorOrSelectionChanged();
}

void REDocumentView::OnCopyPhraseToPasteboard(const REPhrase* phrase, Reflow::TrackType trackType)
{
    qDebug() << "Copying phrase with " << phrase->ChordCount() << " chords to clipboard";

    REBufferOutputStream buffer;
    buffer.WriteUInt8(trackType);
    phrase->EncodeTo(buffer);
    QByteArray bytes = QByteArray(buffer.Data(), buffer.Size());

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* data = new QMimeData();
    data->setData("application/x-reflow-phrase", bytes);
    clipboard->setMimeData(data);
}

void REDocumentView::OnCopyPartialSongToPasteboard(const RESong* song)
{
    qDebug() << "Copying song with " << song->TrackCount() << " tracks to clipboard";

    REBufferOutputStream buffer;
    buffer.WriteUInt32(song->BarCount());
    buffer.WriteUInt32(song->TrackCount());
    song->EncodeTo(buffer);
    QByteArray bytes = QByteArray(buffer.Data(), buffer.Size());

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* data = new QMimeData();
    data->setData("application/x-reflow-psong", bytes);
    clipboard->setMimeData(data);
}

void REDocumentView::ScoreControllerWillRebuildWithSettings(REScoreSettings& settings)
{
}

void REDocumentView::ScoreControllerInspectorWillReload(const REScoreController *scoreController)
{}

void REDocumentView::ScoreControllerInspectorDidReload(const REScoreController *scoreController)
{}

