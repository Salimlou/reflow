#ifndef RECLEFDIALOG_H
#define RECLEFDIALOG_H

#include <QDialog>

#include "RETypes.h"

namespace Ui {
class REClefDialog;
}

class REClefDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REClefDialog(QWidget *parent = 0);
    ~REClefDialog();

    void InitializeWithScoreController(const REScoreController* sc);
    
    Reflow::ClefType Clef() const;
    Reflow::OttaviaType Ottavia() const;

protected slots:
    void on_clefType_currentIndexChanged(int);
    void on_ottaviaType_currentIndexChanged(int);

private:
    Ui::REClefDialog *ui;
};

#endif // RECLEFDIALOG_H
