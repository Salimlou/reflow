#include "RERepeatDialog.h"
#include "ui_RERepeatDialog.h"

#include "REBar.h"

#include <QPushButton>

RERepeatDialog::RERepeatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RERepeatDialog)
{
    ui->setupUi(this);

    // Create an additional Clear button
    QPushButton* btn = ui->buttonBox->addButton(QObject::tr("Clear"), QDialogButtonBox::DestructiveRole);
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(cleared()));

    ui->jumpTypeCombo->addItem(tr("No Jump"));
    ui->jumpTypeCombo->addItem("D.C.");
    ui->jumpTypeCombo->addItem("D.C. al Fine");
    ui->jumpTypeCombo->addItem("D.C. al Coda");
    ui->jumpTypeCombo->addItem("D.C. al Double Coda");
    ui->jumpTypeCombo->addItem("D.S.");
    ui->jumpTypeCombo->addItem("D.S. al Fine");
    ui->jumpTypeCombo->addItem("D.S. al Coda");
    ui->jumpTypeCombo->addItem("D.S. al Double Coda");
    ui->jumpTypeCombo->addItem("D.S.S.");
    ui->jumpTypeCombo->addItem("D.S.S. al Fine");
    ui->jumpTypeCombo->addItem("D.S.S. al Coda");
    ui->jumpTypeCombo->addItem("D.S.S. al Double Coda");

    _repeatStartGroup = new QButtonGroup(this);
    _repeatStartGroup->setExclusive(true);
    _repeatStartGroup->addButton(ui->barlineStart0, 0);
    _repeatStartGroup->addButton(ui->barlineStart1, 1);

    _repeatEndGroup = new QButtonGroup(this);
    _repeatEndGroup->setExclusive(true);
    _repeatEndGroup->addButton(ui->barlineEnd0, 0);
    _repeatEndGroup->addButton(ui->barlineEnd1, 1);
}

RERepeatDialog::~RERepeatDialog()
{
    delete ui;
}

void RERepeatDialog::cleared()
{
    this->done(2);
}

void RERepeatDialog::Initialize(const REBar* firstBar, const REBar* lastBar)
{
    ui->barlineStart0->setChecked(!firstBar->HasFlag(REBar::RepeatStart));
    ui->barlineStart1->setChecked(firstBar->HasFlag(REBar::RepeatStart));
    ui->barlineEnd0->setChecked(!lastBar->HasFlag(REBar::RepeatEnd));
    ui->barlineEnd1->setChecked(lastBar->HasFlag(REBar::RepeatEnd));
    ui->repeatCount->setValue(lastBar->RepeatCount() < 2 ? 2 : lastBar->RepeatCount());

    ui->coda->setChecked(firstBar->HasDirectionTarget(Reflow::Coda));
    ui->doubleCoda->setChecked(firstBar->HasDirectionTarget(Reflow::DoubleCoda));
    ui->segno->setChecked(firstBar->HasDirectionTarget(Reflow::Segno));
    ui->segnoSegno->setChecked(firstBar->HasDirectionTarget(Reflow::SegnoSegno));
    ui->fine->setChecked(lastBar->HasDirectionTarget(Reflow::Fine));

         if(lastBar->HasDirectionJump(Reflow::DaCapo))                     {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DaCapo + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DaCapo_AlFine))              {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DaCapo_AlFine + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DaCapo_AlCoda))              {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DaCapo_AlCoda + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DaCapo_AlDoubleCoda))        {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DaCapo_AlDoubleCoda + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegno))                   {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegno + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegno_AlFine))            {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegno_AlFine + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegno_AlCoda))            {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegno_AlCoda + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegno_AlDoubleCoda))      {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegno_AlDoubleCoda + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegnoSegno))              {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegnoSegno + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegnoSegno_AlFine))       {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegnoSegno_AlFine + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegnoSegno_AlCoda))       {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegnoSegno_AlCoda + 1);}
    else if(lastBar->HasDirectionJump(Reflow::DalSegnoSegno_AlDoubleCoda)) {ui->jumpTypeCombo->setCurrentIndex((int)Reflow::DalSegnoSegno_AlDoubleCoda + 1);}
    else {ui->jumpTypeCombo->setCurrentIndex(0);}

    ui->toCoda->setChecked(lastBar->HasDirectionJump(Reflow::ToCoda));
    ui->toDoubleCoda->setChecked(lastBar->HasDirectionJump(Reflow::ToDoubleCoda));

    ui->alternateEnding1->setChecked(lastBar->HasAlternateEnding(0));
    ui->alternateEnding2->setChecked(lastBar->HasAlternateEnding(1));
    ui->alternateEnding3->setChecked(lastBar->HasAlternateEnding(2));
    ui->alternateEnding4->setChecked(lastBar->HasAlternateEnding(3));
    ui->alternateEnding5->setChecked(lastBar->HasAlternateEnding(4));
    ui->alternateEnding6->setChecked(lastBar->HasAlternateEnding(5));
    ui->alternateEnding7->setChecked(lastBar->HasAlternateEnding(6));
    ui->alternateEnding8->setChecked(lastBar->HasAlternateEnding(7));
}

int RERepeatDialog::RepeatStartType() const
{
    return _repeatStartGroup->checkedId();
}
int RERepeatDialog::RepeatEndType() const
{
    return _repeatEndGroup->checkedId();
}
int RERepeatDialog::RepeatCount() const
{
    return ui->repeatCount->value();
}
bool RERepeatDialog::HasAlternateEnding(int idx) const
{
    switch(idx)
    {
        case 0: return ui->alternateEnding1->isChecked();
        case 1: return ui->alternateEnding2->isChecked();
        case 2: return ui->alternateEnding3->isChecked();
        case 3: return ui->alternateEnding4->isChecked();
        case 4: return ui->alternateEnding5->isChecked();
        case 5: return ui->alternateEnding6->isChecked();
        case 6: return ui->alternateEnding7->isChecked();
        case 7: return ui->alternateEnding8->isChecked();
    }
    return false;
}
bool RERepeatDialog::Coda() const
{
    return ui->coda->isChecked();
}
bool RERepeatDialog::DoubleCoda() const
{
    return ui->doubleCoda->isChecked();
}
bool RERepeatDialog::Segno() const
{
    return ui->segno->isChecked();
}
bool RERepeatDialog::SegnoSegno() const
{
    return ui->segnoSegno->isChecked();
}
bool RERepeatDialog::Fine() const
{
    return ui->fine->isChecked();
}
int RERepeatDialog::JumpType() const
{
    return ui->jumpTypeCombo->currentIndex();
}
bool RERepeatDialog::ToCoda() const
{
    return ui->toCoda->isChecked();
}
bool RERepeatDialog::ToDoubleCoda() const
{
    return ui->toDoubleCoda->isChecked();
}
