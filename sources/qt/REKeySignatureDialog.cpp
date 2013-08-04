#include "REKeySignatureDialog.h"
#include "ui_REKeySignatureDialog.h"

REKeySignatureDialog::REKeySignatureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REKeySignatureDialog)
{
    ui->setupUi(this);
}

REKeySignatureDialog::~REKeySignatureDialog()
{
    delete ui;
}

void REKeySignatureDialog::SetKeySignature(const REKeySignature &sig)
{
    _keySignature = sig;
    ui->preview->SetKeySignature(_keySignature);

    RefreshKeySignatureNames(_keySignature.minor);

    ui->minorCheckbox->blockSignals(true);
    ui->minorCheckbox->setChecked(_keySignature.minor);
    ui->minorCheckbox->blockSignals(false);
}

void REKeySignatureDialog::RefreshKeySignatureNames(bool minor)
{
    QStringList items;
    if(minor) {
        items << "Abm" << "Ebm" << "Bbm" << "Fm"  << "Cm" << "Gm" << "Dm" << "Am" << "Em" << "Bm" << "F#m" << "C#m" << "G#m" << "D#m" << "A#m";
    }
    else {
        items << "Cb" << "Gb" << "Db" << "Ab" << "Eb" << "Bb" << "F" << "C" << "G" << "D" << "A" << "E" << "B" << "F#" << "C#";
    }

    ui->keySignatureCombo->blockSignals(true);
    ui->keySignatureCombo->clear();
    ui->keySignatureCombo->addItems(items);
    ui->keySignatureCombo->setCurrentIndex(_keySignature.key);
    ui->keySignatureCombo->blockSignals(false);
}

void REKeySignatureDialog::on_minorCheckbox_toggled(bool minor)
{
    _keySignature.minor = minor;
    ui->preview->SetKeySignature(_keySignature);

    RefreshKeySignatureNames(minor);
}

void REKeySignatureDialog::on_keySignatureCombo_currentIndexChanged(int index)
{
    _keySignature.key = index;
    ui->preview->SetKeySignature(_keySignature);
}
