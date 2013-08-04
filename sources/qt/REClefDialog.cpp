#include "REClefDialog.h"
#include "ui_REClefDialog.h"

#include "REScoreController.h"
#include "RETrack.h"
#include "REStaff.h"

REClefDialog::REClefDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REClefDialog)
{
    ui->setupUi(this);

    QStringList clefItems; clefItems << tr("Treble Clef") << tr("Bass Clef") << tr("Neutral Clef");
    ui->clefType->addItems(clefItems);

    QStringList ottaviaItems; ottaviaItems << tr("No Ottavia") << "8vb" << "8va" << "15mb" << "15ma";
    ui->ottaviaType->addItems(ottaviaItems);
}

REClefDialog::~REClefDialog()
{
    delete ui;
}

void REClefDialog::InitializeWithScoreController(const REScoreController *scoreController)
{
    const RETrack* firstTrack = scoreController->FirstSelectedTrack();
    const REStaff* firstStaff = scoreController->FirstSelectedStaff();
    if(firstTrack == NULL || firstStaff == NULL) return;

    bool leftHand = (firstStaff->FirstVoiceIndex() != 0);
    const REClefItem* clefItem = firstTrack->ClefTimeline(leftHand).ItemAt(scoreController->FirstSelectedBarIndex(), RETimeDiv(0));

    Reflow::ClefType clef = (clefItem ? clefItem->clef : Reflow::TrebleClef);
    Reflow::OttaviaType ottavia = (clefItem ? clefItem->ottavia : Reflow::NoOttavia);

    ui->clefType->setCurrentIndex(static_cast<int>(clef));
    ui->ottaviaType->setCurrentIndex(static_cast<int>(ottavia));
}

Reflow::ClefType REClefDialog::Clef() const
{
    return static_cast<Reflow::ClefType>(ui->clefType->currentIndex());
}
Reflow::OttaviaType REClefDialog::Ottavia() const
{
    return static_cast<Reflow::OttaviaType>(ui->ottaviaType->currentIndex());
}

void REClefDialog::on_clefType_currentIndexChanged(int idx)
{
    ui->clefPreview->SetClef(static_cast<Reflow::ClefType>(idx));
}

void REClefDialog::on_ottaviaType_currentIndexChanged(int idx)
{
    ui->clefPreview->SetOttavia(static_cast<Reflow::OttaviaType>(idx));
}
