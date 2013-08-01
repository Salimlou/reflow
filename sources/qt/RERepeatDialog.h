#ifndef REREPEATDIALOG_H
#define REREPEATDIALOG_H

#include <QDialog>
#include <QButtonGroup>

#include "RETypes.h"

namespace Ui {
class RERepeatDialog;
}

class RERepeatDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RERepeatDialog(QWidget *parent = 0);
    ~RERepeatDialog();

    void Initialize(const REBar* firstBar, const REBar* lastBar);
    
protected slots:
    void cleared();

public:
    int RepeatStartType() const;
    int RepeatEndType() const;
    int RepeatCount() const;
    bool HasAlternateEnding(int idx) const;
    bool Coda() const;
    bool DoubleCoda() const;
    bool Segno() const;
    bool SegnoSegno() const;
    bool Fine() const;
    int JumpType() const;
    bool ToCoda() const;
    bool ToDoubleCoda() const;

private:
    Ui::RERepeatDialog *ui;
    QButtonGroup* _repeatStartGroup;
    QButtonGroup* _repeatEndGroup;
};

#endif // REREPEATDIALOG_H
