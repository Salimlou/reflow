#include "REStringTuningWidget.h"
#include "ui_REStringTuningWidget.h"

#include "REFunctions.h"

REStringTuningWidget::REStringTuningWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::REStringTuningWidget), _string(0), _pitch(60)
{
    ui->setupUi(this);
}

REStringTuningWidget::~REStringTuningWidget()
{
    delete ui;
}

void REStringTuningWidget::SetPitch(int p)
{
    _pitch = p;
    UpdateUI();
}

void REStringTuningWidget::UpdateUI()
{
    ui->noteNameLabel->setText(QString::fromStdString(Reflow::FindNoteName(_pitch, true)));
    ui->midiNoteLabel->setText(QString("(%1)").arg(_pitch));
}

void REStringTuningWidget::on_tuneDownButton_clicked()
{
    if(_pitch > 0) {
        --_pitch;
        emit stringTuningChanged(_string, _pitch);
    }
}

void REStringTuningWidget::on_tuneUpButton_clicked()
{
    if(_pitch < 99) {
        ++_pitch;
        emit stringTuningChanged(_string, _pitch);
    }
}
