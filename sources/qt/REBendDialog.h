#ifndef REBENDDIALOG_H
#define REBENDDIALOG_H

#include <QDialog>

#include "REBend.h"

namespace Ui {
class REBendDialog;
}

class REBendDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REBendDialog(QWidget *parent = 0);
    ~REBendDialog();

    void SetBend(const REBend& bend);
    const REBend& Bend() const {return _bend;}
    
protected slots:
    void on_bendType_currentIndexChanged(int);
    void on_bentPitch_valueChanged(int);
    void on_releasePitch_valueChanged(int);

private:
    Ui::REBendDialog *ui;
    REBend _bend;
};

#endif // REBENDDIALOG_H
