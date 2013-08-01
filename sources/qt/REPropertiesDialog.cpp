#include "REPropertiesDialog.h"
#include "ui_REPropertiesDialog.h"

#include "REDocumentView.h"
#include "RETrackListModel.h"
#include "REPartListModel.h"

#include "RECreateTrackDialog.h"
#include "RETuningDialog.h"
#include "REUndoCommand.h"
#include "RESong.h"
#include "RETrack.h"
#include "REFunctions.h"
#include "RESequencer.h"

REPropertiesDialog::REPropertiesDialog(REDocumentView *parent) :
    QDialog(parent),
    ui(new Ui::REPropertiesDialog), _backupTrackIndex(0)
{
    ui->setupUi(this);

    InitializeMidiTable();

    InitializeFromDocument();
}

REPropertiesDialog::~REPropertiesDialog()
{
    Cleanup();
    delete ui;
}

REDocumentView* REPropertiesDialog::DocumentView()
{
    return qobject_cast<REDocumentView*>(parent());
}

REScoreController* REPropertiesDialog::ScoreController()
{
    return DocumentView()->ScoreController();
}

void REPropertiesDialog::InitializeMidiTable()
{
    for(int prog=0; prog<=127; ++prog)
    {
        std::string programName = Reflow::NameOfMidiProgram(prog);
        ui->midiProgramList->addItem(QString("%1 - %2").arg(prog+1, 3).arg(QString::fromStdString(programName)));
    }
}

void REPropertiesDialog::InitializeFromDocument()
{
    REDocumentView* doc = DocumentView();

    REScoreController* scoreController = ScoreController();
    const RESong* song = scoreController->Score()->Song();

    RETrackListModel* model = doc->TrackListModel();
    ui->trackList->setModel(model);
    QObject::connect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(trackListModelAboutToBeReset()));
    QObject::connect(model, SIGNAL(modelReset()), this, SLOT(trackListModelReset()));

    ui->partList->setModel(doc->PartListModel());

    ui->trackList->setCurrentIndex(ui->trackList->model()->index(0,0));
}

void REPropertiesDialog::Cleanup()
{
    REDocumentView* doc = DocumentView();
    RETrackListModel* model = doc->TrackListModel();
    QObject::disconnect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(trackListModelAboutToBeReset()));
    QObject::disconnect(model, SIGNAL(modelReset()), this, SLOT(trackListModelReset()));
}

void REPropertiesDialog::UpdateTrackUI(const RETrack* track)
{
    ui->nameText->blockSignals(true);
    ui->shortNameText->blockSignals(true);
    ui->midiProgramList->blockSignals(true);
    ui->tuningText->blockSignals(true);
    ui->capoSpinBox->blockSignals(true);

    if(track)
    {
        ui->nameText->setText(QString::fromStdString(track->Name()));
        ui->shortNameText->setText(QString::fromStdString(track->ShortName()));
        ui->midiProgramList->setCurrentRow(track->MIDIProgram());
        ui->tuningText->setText(QString::fromStdString(track->TuningNoteNames()));
        ui->capoSpinBox->setValue(track->Capo());
    }
    else
    {
        ui->nameText->setText("");
        ui->shortNameText->setText("");
        ui->midiProgramList->setCurrentRow(0);
        ui->tuningText->setText("");
        ui->capoSpinBox->setValue(0);
    }

    ui->nameText->blockSignals(false);
    ui->shortNameText->blockSignals(false);
    ui->midiProgramList->blockSignals(false);
    ui->tuningText->blockSignals(false);
    ui->capoSpinBox->blockSignals(false);
}

void REPropertiesDialog::on_nameText_editingFinished()
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();

    int trackIndex = ui->trackList->currentIndex().row();
    QString trackName = ui->nameText->text();

    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::SetNameOfTrack, std::placeholders::_1, trackIndex, trackName.toStdString())));
}

void REPropertiesDialog::on_shortNameText_editingFinished()
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();

    int trackIndex = ui->trackList->currentIndex().row();
    QString trackShortName = ui->shortNameText->text();

    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::SetShortNameOfTrack, std::placeholders::_1, trackIndex, trackShortName.toStdString())));
}

void REPropertiesDialog::on_midiProgramList_currentRowChanged(int midiProgram)
{
    int trackIndex = ui->trackList->currentIndex().row();

    REDocumentView* doc = DocumentView();
    RESequencer* sequencer = doc->Sequencer();
    REScoreController* scoreController = ScoreController();
    RESong* song = doc->Song();
    RETrack* track = song->Track(trackIndex);

    if(track == nullptr) return;

    track->SetMIDIProgram(midiProgram);

    // TODO: Notify document has been changed

    // Hot change
    if(sequencer) {
        sequencer->SetTrackMidiProgram(trackIndex, midiProgram);
    }
}

void REPropertiesDialog::on_addTrackButton_clicked()
{
    RECreateTrackDialog dlg(this);
    if(dlg.exec())
    {
        RECreateTrackOptions opts = dlg.SelectedTrackTemplate();

        opts.addToFullScore = dlg.AddToFirstPartChecked();
        opts.createPart = dlg.CreatePartChecked();

        REDocumentView* doc = DocumentView();
        REScoreController* scoreController = ScoreController();

        doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::CreateTrack, std::placeholders::_1, opts)));
    }
}

void REPropertiesDialog::on_deleteTrackButton_clicked()
{
    int trackIndex = ui->trackList->currentIndex().row();

    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();
    const RESong* song = doc->Song();
    if(song->TrackCount() == 1)
    {
        QMessageBox::critical(this, tr("Operation not permitted"), tr("You can't delete the last track"), QMessageBox::Ok);
    }
    else if(0 <= trackIndex && trackIndex < song->TrackCount())
    {
        if(QMessageBox::Yes == QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to delete selected track ? (This is undoable)"), QMessageBox::Yes, QMessageBox::No))
        {
            doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::RemoveTrack, std::placeholders::_1, trackIndex)));
        }
    }
}

void REPropertiesDialog::on_tuningButton_clicked()
{
    int trackIndex = ui->trackList->currentIndex().row();

    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();
    const RESong* song = doc->Song();
    const RETrack* track = song->Track(trackIndex);

    if(track == nullptr || !track->IsTablature()) return;

    RETuningDialog dlg(this);
    dlg.UpdateUIFromTrack(track);
    if(dlg.exec())
    {
        auto tuningArray = dlg.TuningArray();

        doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::SetTuningOfTrack, std::placeholders::_1, trackIndex, tuningArray.data(), tuningArray.size())));
    }
}

void REPropertiesDialog::on_trackList_trackSelected(int trackIndex)
{
    REDocumentView* doc = DocumentView();
    const RESong* song = doc->Song();
    UpdateTrackUI(song->Track(trackIndex));
}

void REPropertiesDialog::trackListModelAboutToBeReset()
{
    int selectedTrackBefore = ui->trackList->currentIndex().row();
    qDebug() << "selected track before " << selectedTrackBefore;

    _backupTrackIndex = selectedTrackBefore;
}

void REPropertiesDialog::trackListModelReset()
{
    ui->trackList->setCurrentIndex(ui->trackList->model()->index(_backupTrackIndex,0));
}
