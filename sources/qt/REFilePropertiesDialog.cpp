#include "REFilePropertiesDialog.h"
#include "ui_REFilePropertiesDialog.h"

#include "RESong.h"

REFilePropertiesDialog::REFilePropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::REFilePropertiesDialog)
{
    ui->setupUi(this);
}

REFilePropertiesDialog::~REFilePropertiesDialog()
{
    delete ui;
}

void REFilePropertiesDialog::Initialize(const RESong* song)
{
    ui->title->setText(QString::fromStdString(song->Title()));
    ui->subtitle->setText(QString::fromStdString(song->SubTitle()));
    ui->artist->setText(QString::fromStdString(song->Artist()));
    ui->album->setText(QString::fromStdString(song->Album()));
    ui->musicBy->setText(QString::fromStdString(song->MusicBy()));
    ui->lyricsBy->setText(QString::fromStdString(song->LyricsBy()));
    ui->transcribedBy->setText(QString::fromStdString(song->Transcriber()));
    ui->copyright->setText(QString::fromStdString(song->Copyright()));
}

QString REFilePropertiesDialog::Title() const {return ui->title->text();}
QString REFilePropertiesDialog::Subtitle() const {return ui->subtitle->text();}
QString REFilePropertiesDialog::Artist() const {return ui->artist->text();}
QString REFilePropertiesDialog::Album() const {return ui->album->text();}
QString REFilePropertiesDialog::MusicBy() const {return ui->musicBy->text();}
QString REFilePropertiesDialog::LyricsBy() const {return ui->lyricsBy->text();}
QString REFilePropertiesDialog::Transcriber() const {return ui->transcribedBy->text();}
QString REFilePropertiesDialog::Copyright() const {return ui->copyright->text();}
