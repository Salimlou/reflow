#include "REPreferencesDialog.h"
#include "ui_REPreferencesDialog.h"

#include <QSettings>

REPreferencesDialog::REPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REPreferencesDialog)
{
    ui->setupUi(this);

    QSettings settings;
    ui->playNotesOnInput->setChecked(settings.value("edit/playNotesOnInput", QVariant(true)).toBool());
    ui->grayOutInactiveVoice->setChecked(settings.value("edit/grayOutInactiveVoice", QVariant(false)).toBool());
    ui->invertPlusMinusKeys->setChecked(settings.value("edit/invertPlusMinusKeys", QVariant(false)).toBool());
}

REPreferencesDialog::~REPreferencesDialog()
{
    QSettings settings;
    settings.setValue("edit/playNotesOnInput", QVariant(ui->playNotesOnInput->isChecked()));
    settings.setValue("edit/grayOutInactiveVoice", QVariant(ui->grayOutInactiveVoice->isChecked()));
    settings.setValue("edit/invertPlusMinusKeys", QVariant(ui->invertPlusMinusKeys->isChecked()));

    delete ui;
}
