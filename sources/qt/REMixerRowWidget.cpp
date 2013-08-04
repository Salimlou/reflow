#include "REMixerRowWidget.h"
#include "REMixerWidget.h"
#include "REDocumentView.h"
#include "REScoreController.h"
#include "RESongController.h"
#include "REUndoCommand.h"
#include "RETrack.h"
#include "RESong.h"
#include "REScore.h"
#include "RESequencer.h"

#include "ui_REMixerRowWidget.h"

#include <QMouseEvent>

REMixerRowWidget::REMixerRowWidget(REMixerWidget* parent, int trackIndex)
    : QWidget(parent),
      _trackIndex(trackIndex),
    ui(new Ui::REMixerRowWidget)
{
    ui->setupUi(this);

    REDocumentView* doc = DocumentView();
    const REScore* score = doc->ScoreController()->Score();
    const RESong* song = doc->Song();
    const RETrack* track = song->Track(_trackIndex);

    if(track != nullptr)
    {
        ui->trackNameLabel->setText(QString::fromStdString(track->Name()));

        QString instName = "electric_guitar";
        int midi = track->MIDIProgram();
        if(track->Type() == Reflow::TablatureTrack)
        {
            if(33 <= midi && midi <= 38) {
                instName = "bass";
            }
            else if(27 <= midi && midi <= 32){
                instName = "electric_guitar";
            }
            else {
                instName = "accoustic_guitar";
            }
        }
        else if(track->Type() == Reflow::DrumsTrack) {
            instName = "drums";
        }
        else {
            instName = "piano";
        }

        QString iconName = QString(":/instrument_%1_mac.png").arg(instName);
        ui->trackIconLabel->setPixmap(QPixmap(iconName));

        int volume = 100.0f * track->Volume();
        int pan = 100.0f * track->Pan();

        ui->viewButton->blockSignals(true);
        ui->muteButton->blockSignals(true);
        ui->soloButton->blockSignals(true);
        ui->volumeSlider->blockSignals(true);
        ui->panSlider->blockSignals(true);

        ui->viewButton->setChecked(score->ContainsTrack(track));
        ui->muteButton->setChecked(track->IsMute());
        ui->soloButton->setChecked(track->IsSolo());
        ui->volumeSlider->setValue(volume);
        ui->panSlider->setValue(pan);

        ui->viewButton->blockSignals(false);
        ui->muteButton->blockSignals(false);
        ui->soloButton->blockSignals(false);
        ui->volumeSlider->blockSignals(false);
        ui->panSlider->blockSignals(false);
    }
}

REMixerRowWidget::~REMixerRowWidget()
{
    delete ui;
}

REDocumentView* REMixerRowWidget::DocumentView()
{
    REMixerWidget* mix = qobject_cast<REMixerWidget*>(parent());
    return mix->DocumentView();
}

void REMixerRowWidget::on_viewButton_toggled(bool present)
{
    REDocumentView* doc = DocumentView();
    REScoreController* scoreController = doc->ScoreController();
    RESequencer* sequencer = doc->Sequencer();
    RESong* song = doc->Song();
    RETrack* track = song->Track(_trackIndex);

    doc->UndoStack()->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::SetTrackPresentInCurrentScore, std::placeholders::_1, _trackIndex, present)));
}

void REMixerRowWidget::on_soloButton_toggled(bool state)
{
    REDocumentView* doc = DocumentView();
    RESequencer* sequencer = doc->Sequencer();
    RESong* song = doc->Song();
    RETrack* track = song->Track(_trackIndex);

    if(track == nullptr) return;

    track->SetSolo(state);

    // TODO: Notify document has been changed

    // Hot change
    if(sequencer) {
        sequencer->SetTrackSolo(_trackIndex, state);
    }
}

void REMixerRowWidget::on_muteButton_toggled(bool state)
{
    REDocumentView* doc = DocumentView();
    RESequencer* sequencer = doc->Sequencer();
    RESong* song = doc->Song();
    RETrack* track = song->Track(_trackIndex);

    if(track == nullptr) return;

    track->SetMute(state);

    // TODO: Notify document has been changed

    // Hot change
    if(sequencer) {
        sequencer->SetTrackMute(_trackIndex, state);
    }
}

void REMixerRowWidget::on_volumeSlider_valueChanged(int newValue)
{
    REDocumentView* doc = DocumentView();
    RESequencer* sequencer = doc->Sequencer();
    RESong* song = doc->Song();
    RETrack* track = song->Track(_trackIndex);

    if(track == nullptr) return;

    float volume = (float)newValue / 100.0f;
    track->SetVolume(volume);

    // TODO: Notify document has been changed

    // Hot change
    if(sequencer) {
        sequencer->SetTrackVolume(_trackIndex, volume);
    }
}

void REMixerRowWidget::on_panSlider_valueChanged(int newValue)
{
    REDocumentView* doc = DocumentView();
    RESequencer* sequencer = doc->Sequencer();
    RESong* song = doc->Song();
    RETrack* track = song->Track(_trackIndex);

    if(track == nullptr) return;

    float pan = (float)newValue / 100.0f;
    track->SetPan(pan);

    // TODO: Notify document has been changed

    // Hot change
    if(sequencer) {
        sequencer->SetTrackPan(_trackIndex, pan);
    }
}

void REMixerRowWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    DocumentView()->ShowTracksAndPartsDialogSelectingTrack(_trackIndex);
}
