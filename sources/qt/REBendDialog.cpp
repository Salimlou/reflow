#include "REBendDialog.h"
#include "ui_REBendDialog.h"

REBendDialog::REBendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REBendDialog)
{
    ui->setupUi(this);

    QStringList list;
    list << tr("No Bend") << tr("Bend") << tr("Bend and Release")
         << tr("Prebend") << tr("Prebend and Release");

    ui->bendType->addItems(list);
}

REBendDialog::~REBendDialog()
{
    delete ui;
}

void REBendDialog::SetBend(const REBend& bend)
{
    _bend = bend;

    ui->bendType->blockSignals(true);
    ui->bentPitch->blockSignals(true);
    ui->releasePitch->blockSignals(true);

    ui->bendType->setCurrentIndex(static_cast<int>(_bend.Type()));
    ui->bentPitch->setValue(_bend.BentPitch());
    ui->releasePitch->setValue(_bend.ReleasePitch());

    ui->bendType->blockSignals(false);
    ui->bentPitch->blockSignals(false);
    ui->releasePitch->blockSignals(false);
}

void REBendDialog::on_bendType_currentIndexChanged(int idx)
{
    _bend.SetType(static_cast<Reflow::BendType>(idx));
}

void REBendDialog::on_bentPitch_valueChanged(int val)
{
    _bend.SetBentPitch(val);
}

void REBendDialog::on_releasePitch_valueChanged(int val)
{
    _bend.SetReleasePitch(val);
}
