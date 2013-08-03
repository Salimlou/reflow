#include "RETextDialog.h"
#include "ui_RETextDialog.h"

#include "REChord.h"

RETextDialog::RETextDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RETextDialog)
{
    ui->setupUi(this);

    QStringList items;
    items << tr("Text Above Standard Staff")
          << tr("Text Below Standard Staff")
          << tr("Text Above Tablature Staff")
          << tr("Text Below Tablature Staff");
    ui->positioning->addItems(items);
}

RETextDialog::~RETextDialog()
{
    delete ui;
}

void RETextDialog::InitializeFromChord(const REChord* chord)
{
    ui->text->blockSignals(true);
    ui->positioning->blockSignals(true);

    QString beatText = (chord->HasFlag(REChord::Text) ? QString::fromStdString(chord->TextAttached()) : "");
    int beatTextPosition = (chord->HasFlag(REChord::Text) ? static_cast<int>(chord->TextPositioning()) : 0);

    ui->text->setText(beatText);
    ui->positioning->setCurrentIndex(beatTextPosition);

    ui->text->blockSignals(false);
    ui->positioning->blockSignals(false);

    ui->text->setFocus();
    ui->text->selectAll();
}

QString RETextDialog::Text() const
{
    return ui->text->text();
}
int RETextDialog::PositioningIndex() const
{
    return ui->positioning->currentIndex();
}
