#include "RETimeSignatureDialog.h"
#include "ui_RETimeSignatureDialog.h"

#include "REBar.h"

RETimeSignatureDialog::RETimeSignatureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RETimeSignatureDialog)
{
    ui->setupUi(this);
}

RETimeSignatureDialog::~RETimeSignatureDialog()
{
    delete ui;
}

void RETimeSignatureDialog::Initialize(const REBar* firstBar)
{
    ui->numerator->setValue(firstBar ? firstBar->TimeSignature().numerator : 4);
    ui->denominator->setValue(firstBar ? firstBar->TimeSignature().denominator : 4);

    std::string patternString = (firstBar ? firstBar->BeamingPattern().ToString() : "2");
    ui->beaming->setText(QString::fromStdString(patternString));
}

int RETimeSignatureDialog::Numerator() const
{
    return ui->numerator->value();
}

int RETimeSignatureDialog::Denominator() const
{
    return ui->denominator->value();
}

REBeamingPattern RETimeSignatureDialog::BeamingPattern() const
{
    REBeamingPattern pattern;
    QString text = ui->beaming->text();
    if(text.isEmpty()) {
        pattern.Clear();
    }
    else {
        std::string str = text.toStdString();
        if(!pattern.Parse(str.data(), str.length())) {
            pattern.Clear();
        }
    }
    return pattern;
}
