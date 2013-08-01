#include "RERehearsalDialog.h"
#include "ui_RERehearsalDialog.h"

#include <QObject>
#include <QPushButton>

RERehearsalDialog::RERehearsalDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RERehearsalDialog)
{
    ui->setupUi(this);

    // Create an additional Clear button
    QPushButton* btn = ui->buttonBox->addButton(QObject::tr("Clear"), QDialogButtonBox::DestructiveRole);
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(cleared()));

    ui->rehearsalText->setFocus();
}

RERehearsalDialog::~RERehearsalDialog()
{
    delete ui;
}

void RERehearsalDialog::cleared()
{
    this->done(2);
}

void RERehearsalDialog::setRehearsalText(QString text)
{
    ui->rehearsalText->setText(text);
}

QString RERehearsalDialog::rehearsalText() const
{
    return ui->rehearsalText->text();
}
