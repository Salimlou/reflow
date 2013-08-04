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
#include "REScoreController.h"
#include "RESongController.h"
#include "REScoreSettings.h"
#include "REScore.h"

#include <QMessageBox>

REPropertiesDialog::REPropertiesDialog(REDocumentView *parent) :
    QDialog(parent),
    ui(new Ui::REPropertiesDialog), _backupTrackIndex(0), _backupPartIndex(0)
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

    // Track List Model
    {
        RETrackListModel* model = doc->TrackListModel();
        ui->trackList->setModel(model);
        QObject::connect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(trackListModelAboutToBeReset()));
        QObject::connect(model, SIGNAL(modelReset()), this, SLOT(trackListModelReset()));
    }

    // Part List Model
    {
        REPartListModel* model = doc->PartListModel();
        ui->partList->setModel(model);
        QObject::connect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(partListModelAboutToBeReset()));
        QObject::connect(model, SIGNAL(modelReset()), this, SLOT(partListModelReset()));
    }

    ui->trackList->setCurrentIndex(ui->trackList->model()->index(0,0));

    int scoreIndex = scoreController->ScoreIndex();
    ui->partList->blockSignals(true);
    ui->partList->setCurrentIndex(ui->partList->model()->index(scoreIndex,0));
    ui->partList->blockSignals(false);
    UpdatePartUI(song->Score(scoreIndex));
}

void REPropertiesDialog::Cleanup()
{
    REDocumentView* doc = DocumentView();

    // Track List Model
    {
        RETrackListModel* model = doc->TrackListModel();
        QObject::disconnect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(trackListModelAboutToBeReset()));
        QObject::disconnect(model, SIGNAL(modelReset()), this, SLOT(trackListModelReset()));
    }

    // Part List Model
    {
        REPartListModel* model = doc->PartListModel();
        QObject::disconnect(model, SIGNAL(modelAboutToBeReset()), this, SLOT(partListModelAboutToBeReset()));
        QObject::disconnect(model, SIGNAL(modelReset()), this, SLOT(partListModelReset()));
    }
}

void REPropertiesDialog::UpdateTrackUI(const RETrack* track)
{
    bool isTablature = (track && track->IsTablature());

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

    ui->capoSpinBox->setVisible(isTablature);
    ui->capoLabel->setVisible(isTablature);
    ui->tuningButton->setVisible(isTablature);
    ui->tuningLabel->setVisible(isTablature);
    ui->tuningText->setVisible(isTablature);
    ui->stringCountLabel->setVisible(isTablature);

    ui->nameText->blockSignals(false);
    ui->shortNameText->blockSignals(false);
    ui->midiProgramList->blockSignals(false);
    ui->tuningText->blockSignals(false);
    ui->capoSpinBox->blockSignals(false);
}

void REPropertiesDialog::UpdatePartUI(const REScoreSettings *scoreSettings)
{
    ui->partNameText->blockSignals(true);
    ui->showStandardNotation->blockSignals(true);
    ui->showTablature->blockSignals(true);
    ui->hideDynamics->blockSignals(true);
    ui->useMultibarRests->blockSignals(true);

    if(scoreSettings)
    {
        ui->partNameText->setText(QString::fromStdString(scoreSettings->Name()));
        ui->showStandardNotation->setChecked(!scoreSettings->HasFlag(REScoreSettings::HideStandard));
        ui->showTablature->setChecked(!scoreSettings->HasFlag(REScoreSettings::HideTablature));
        ui->hideDynamics->setChecked(scoreSettings->HasFlag(REScoreSettings::HideDynamics));
        ui->useMultibarRests->setChecked(scoreSettings->HasFlag(REScoreSettings::UseMultiRests));
    }
    else
    {
        ui->partNameText->setText("");
        ui->showStandardNotation->setChecked(false);
        ui->showTablature->setChecked(false);
        ui->hideDynamics->setChecked(false);
        ui->useMultibarRests->setChecked(false);
    }

    ui->partNameText->blockSignals(false);
    ui->showStandardNotation->blockSignals(false);
    ui->showTablature->blockSignals(false);
    ui->hideDynamics->blockSignals(false);
    ui->useMultibarRests->blockSignals(false);
}

void REPropertiesDialog::SetActiveTrack(int trackIndex)
{
    ui->tabWidget->setCurrentIndex(0);
    ui->trackList->setCurrentIndex(ui->trackList->model()->index(trackIndex, 0));
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

void REPropertiesDialog::on_addPartButton_clicked()
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();

    RESongControllerOperation op = std::bind(&RESongController::CreateEmptyScoreAtEnd, std::placeholders::_1);
    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
}

void REPropertiesDialog::on_deletePartButton_clicked()
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();
    const RESong* song = doc->Song();

    if(song->ScoreCount() <= 1) return;

    int scoreIndex = ui->partList->currentIndex().row();
    if(0 <= scoreIndex && scoreIndex < song->ScoreCount())
    {
        RESongControllerOperation op = std::bind(&RESongController::RemoveScore, std::placeholders::_1, scoreIndex);
        doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
    }
}

void REPropertiesDialog::on_partNameText_editingFinished()
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = ScoreController();

    int scoreIndex = ui->partList->currentIndex().row();
    if(0 <= scoreIndex && scoreIndex < doc->Song()->ScoreCount())
    {
        QString partName = ui->partNameText->text();

        RESongControllerOperation op = std::bind(&RESongController::SetNameOfScore, std::placeholders::_1, scoreIndex, partName.toStdString());
        doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
    }
}

void REPropertiesDialog::on_partList_partSelected(int partIndex)
{
    REDocumentView* doc = DocumentView();
    const RESong* song = doc->Song();
    UpdatePartUI(song->Score(partIndex));

    doc->ScoreController()->SetScoreIndex(partIndex);
}

void REPropertiesDialog::on_showStandardNotation_toggled(bool enabled)
{
    REDocumentView* doc = DocumentView();
    RESongController* songController = doc->SongController();
    REScoreController* scoreController = doc->ScoreController();
    const RESong* song = doc->Song();

    int scoreIndex = ui->partList->currentIndex().row();
    if(scoreIndex < 0 || scoreIndex >= song->ScoreCount()) return;

    RESongControllerOperation op = std::bind(&RESongController::SetScoreFlag, std::placeholders::_1, scoreIndex, REScoreSettings::HideStandard, !enabled);
    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
}

void REPropertiesDialog::on_showTablature_toggled(bool enabled)
{
    REDocumentView* doc = DocumentView();
    RESongController* songController = doc->SongController();
    REScoreController* scoreController = doc->ScoreController();
    const RESong* song = doc->Song();

    int scoreIndex = ui->partList->currentIndex().row();
    if(scoreIndex < 0 || scoreIndex >= song->ScoreCount()) return;

    RESongControllerOperation op = std::bind(&RESongController::SetScoreFlag, std::placeholders::_1, scoreIndex, REScoreSettings::HideTablature, !enabled);
    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
}

void REPropertiesDialog::on_useMultibarRests_toggled(bool enabled)
{
    REDocumentView* doc = DocumentView();
    RESongController* songController = doc->SongController();
    REScoreController* scoreController = doc->ScoreController();
    const RESong* song = doc->Song();

    int scoreIndex = ui->partList->currentIndex().row();
    if(scoreIndex < 0 || scoreIndex >= song->ScoreCount()) return;

    RESongControllerOperation op = std::bind(&RESongController::SetScoreFlag, std::placeholders::_1, scoreIndex, REScoreSettings::UseMultiRests, enabled);
    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
}

void REPropertiesDialog::on_hideDynamics_toggled(bool enabled)
{
    REDocumentView* doc = DocumentView();
    RESongController* songController = doc->SongController();
    REScoreController* scoreController = doc->ScoreController();
    const RESong* song = doc->Song();

    int scoreIndex = ui->partList->currentIndex().row();
    if(scoreIndex < 0 || scoreIndex >= song->ScoreCount()) return;

    RESongControllerOperation op = std::bind(&RESongController::SetScoreFlag, std::placeholders::_1, scoreIndex, REScoreSettings::HideDynamics, enabled);
    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::PerformTaskOnSongController, std::placeholders::_1, op)));
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

void REPropertiesDialog::partListModelAboutToBeReset()
{
    int selectedPartBefore = ui->partList->currentIndex().row();
    qDebug() << "selected part before " << selectedPartBefore;

    _backupPartIndex = selectedPartBefore;
}

void REPropertiesDialog::partListModelReset()
{
    ui->partList->setCurrentIndex(ui->partList->model()->index(_backupPartIndex,0));
}
