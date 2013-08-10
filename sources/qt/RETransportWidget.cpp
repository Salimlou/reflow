#include "RETransportWidget.h"
#include "REDocumentView.h"
#include "ui_RETransportWidget.h"

#include "REAudioEngine.h"
#include "REAudioSettings.h"
#include "REBar.h"

#include <QPainter>
#include <QPushButton>

RETransportWidget::RETransportWidget(QWidget *parent) :
    QWidget(parent), _documentView(nullptr),
    ui(new Ui::RETransportWidget)
{
    ui->setupUi(this);

#ifdef MACOSX
    // Workaround the weird crash on Mac OSX ...
    ui->playButton->setFlat(false);
    ui->metronomeButton->setFlat(false);
    ui->preclickButton->setFlat(false);
    ui->trackingButton->setFlat(false);
    ui->loopButton->setFlat(false);
    ui->loopStartButton->setFlat(false);
    ui->loopStartButton->setFlat(false);
#endif

    setMinimumSize(60, 88);
    setMaximumSize(250, 88);
}

RETransportWidget::~RETransportWidget()
{
    delete ui;
}

void RETransportWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QBrush(QColor::fromRgb(0x33, 0x33, 0x33)));
}

void RETransportWidget::ConnectToDocument(REDocumentView *doc)
{
    _documentView = doc;
    if(_documentView == nullptr) return;

    ui->playButton->blockSignals(true);
    ui->playButton->setChecked(_documentView->IsPlaybackRunning());
    ui->playButton->blockSignals(false);

    ui->trackingButton->setChecked(_documentView->IsTrackingEnabled());

    QObject::connect(_documentView, SIGNAL(PlaybackStarted()), this, SLOT(OnPlaybackStarted()));
    QObject::connect(_documentView, SIGNAL(PlaybackStopped()), this, SLOT(OnPlaybackStopped()));
    QObject::connect(ui->trackingButton, SIGNAL(toggled(bool)), _documentView, SLOT(SetTrackingEnabled(bool)));
}

void RETransportWidget::DisconnectFromDocument()
{
    if(_documentView == nullptr) return;

    QObject::disconnect(_documentView, SIGNAL(PlaybackStarted()), this, SLOT(OnPlaybackStarted()));
    QObject::disconnect(_documentView, SIGNAL(PlaybackStopped()), this, SLOT(OnPlaybackStopped()));
    QObject::disconnect(ui->trackingButton, SIGNAL(toggled(bool)), _documentView, SLOT(SetTrackingEnabled(bool)));

    _documentView = nullptr;
}

void RETransportWidget::OnPlaybackStarted()
{
    if(_documentView == nullptr) return;

    RESequencer* sequencer = _documentView->Sequencer();

    ui->playButton->blockSignals(true);
    ui->playButton->setChecked(sequencer->IsRunning());
    ui->playButton->blockSignals(false);
}

void RETransportWidget::OnPlaybackStopped()
{
    if(_documentView == nullptr) return;

    RESequencer* sequencer = _documentView->Sequencer();

    ui->playButton->blockSignals(true);
    ui->playButton->setChecked(sequencer->IsRunning());
    ui->playButton->blockSignals(false);
}

void RETransportWidget::on_playButton_toggled(bool)
{
    if(_documentView == nullptr) return;

    _documentView->TogglePlayback();
}

void RETransportWidget::on_loopButton_toggled(bool enabled)
{
    if(_documentView == nullptr) return;

    RESequencer* sequencer = _documentView->Sequencer();
    if(sequencer) {
        sequencer->SetLoopPlaybackEnabled(enabled);
    }
}

void RETransportWidget::on_metronomeButton_toggled(bool enabled)
{
    REAudioEngine* engine = REAudioEngine::Instance();
    if(!engine) return;

    REAudioSettings newSettings = engine->AudioSettings();
    newSettings.SetMetronomeEnabled(enabled);
    engine->SetAudioSettings(newSettings);
}

void RETransportWidget::on_preclickButton_toggled(bool enabled)
{
    REAudioEngine* engine = REAudioEngine::Instance();
    if(!engine) return;

    REAudioSettings newSettings = engine->AudioSettings();
    newSettings.SetPreclickBarCount(enabled ? 1 : 0);
    engine->SetAudioSettings(newSettings);
}

void RETransportWidget::on_loopStartButton_pressed()
{
    if(_documentView == nullptr) return;

    const REScoreController* scoreController = _documentView->ScoreController();
    RESequencer* sequencer = _documentView->Sequencer();
    if(sequencer) {
        sequencer->SetLoopStartIndicator(scoreController->FirstSelectedBarIndex(), 0);
    }
}

void RETransportWidget::on_loopEndButton_pressed()
{
    if(_documentView == nullptr) return;

    const REScoreController* scoreController = _documentView->ScoreController();
    RESequencer* sequencer = _documentView->Sequencer();
    if(sequencer) {
        const REBar* lastBar = scoreController->LastSelectedBar();
        if(lastBar) {
            sequencer->SetLoopEndIndicator(lastBar->Index(), lastBar->TheoricDurationInTicks());
        }
    }
}

