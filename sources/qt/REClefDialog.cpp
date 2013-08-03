#include "REClefDialog.h"
#include "ui_REClefDialog.h"

REClefDialog::REClefDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REClefDialog)
{
    ui->setupUi(this);
}

REClefDialog::~REClefDialog()
{
    delete ui;
}
