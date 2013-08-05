#include "RETempoMarkerDialog.h"
#include "ui_RETempoMarkerDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

RETempoMarkerDialog::RETempoMarkerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RETempoMarkerDialog)
{
    ui->setupUi(this);

    // Create an additional Clear button
    QPushButton* btn = ui->buttonBox->addButton(QObject::tr("Clear"), QDialogButtonBox::DestructiveRole);
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(cleared()));

    QStringList list;
    list << tr("Quarter") << tr("Quarter Dotted");
    ui->tempoTypeCombo->addItems(list);
}

RETempoMarkerDialog::~RETempoMarkerDialog()
{
    delete ui;
}

void RETempoMarkerDialog::cleared()
{
    done(2);
}

void RETempoMarkerDialog::Initialize(int tempo, Reflow::TempoUnitType tempoUnitType)
{
    ui->tempoSpin->blockSignals(true);
    ui->tempoTypeCombo->blockSignals(true);

    ui->tempoSpin->setValue(tempo);
    ui->tempoTypeCombo->setCurrentIndex(static_cast<int>(tempoUnitType));

    ui->tempoSpin->blockSignals(false);
    ui->tempoTypeCombo->blockSignals(false);
}

int RETempoMarkerDialog::Tempo() const
{
    return ui->tempoSpin->value();
}

Reflow::TempoUnitType RETempoMarkerDialog::TempoUnitType() const
{
    return static_cast<Reflow::TempoUnitType>(ui->tempoTypeCombo->currentIndex());
}
