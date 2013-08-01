#include "RETuningDialog.h"
#include "ui_RETuningDialog.h"

#include "REStringTuningWidget.h"

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "RETrack.h"

RETuningDialog::RETuningDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RETuningDialog)
{
    ui->setupUi(this);
}

RETuningDialog::~RETuningDialog()
{
    delete ui;
}

void RETuningDialog::UpdateUIFromTrack(const RETrack* track)
{
    _tuningArray.clear();

    if(!track->IsTablature()) return;

    int stringCount = track->StringCount();
    const uint8_t* tuning = track->TuningArray();
    for(int i=0; i<stringCount; ++i)
    {
        _tuningArray.append(static_cast<int>(tuning[i]));
    }

    CreateScrollAreaContents();
}

void RETuningDialog::CreateScrollAreaContents()
{
    QVBoxLayout* vbox = new QVBoxLayout;
    int h = 0;
    vbox->setMargin(0);
    for(int string=0; string < _tuningArray.size(); ++string)
    {
        int pitch = _tuningArray[string];

        REStringTuningWidget* w = new REStringTuningWidget;
        w->SetStringIndex(string);
        w->SetPitch(pitch);
        QObject::connect(w, SIGNAL(stringTuningChanged(int,int)), this, SLOT(changeTuningOfString(int,int)));
        vbox->addWidget(w);

        h += w->height();
    }

    QWidget* contents = new QWidget;
    contents->setMinimumHeight(h);
    contents->setMaximumHeight(h);
    contents->setLayout(vbox);
    ui->scrollArea->setWidget(contents);
}

void RETuningDialog::on_addOneLowStringButton_clicked()
{
    if(_tuningArray.size() >= 12 || _tuningArray.size() == 0) return;

    int midi = _tuningArray.last() - 5;
    if(midi < 0) midi = 0;

    _tuningArray.push_back(midi);
    CreateScrollAreaContents();
}

void RETuningDialog::on_removeOneLowStringButton_clicked()
{
    if(_tuningArray.size() <= 1) return;

    _tuningArray.pop_back();
    CreateScrollAreaContents();
}

void RETuningDialog::on_allStringsDownButton_clicked()
{
    for(int& pitch : _tuningArray)
    {
        if(pitch > 0) {
            --pitch;
        }
    }
    CreateScrollAreaContents();
}

void RETuningDialog::on_allStringsUpButton_clicked()
{
    for(int& pitch : _tuningArray) {
        if(pitch < 99) ++pitch;
    }
    CreateScrollAreaContents();
}

void RETuningDialog::changeTuningOfString(int stringIndex, int pitch)
{
    _tuningArray[stringIndex] = pitch;
    CreateScrollAreaContents();
}
