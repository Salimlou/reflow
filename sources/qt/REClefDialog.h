#ifndef RECLEFDIALOG_H
#define RECLEFDIALOG_H

#include <QDialog>

namespace Ui {
class REClefDialog;
}

class REClefDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REClefDialog(QWidget *parent = 0);
    ~REClefDialog();
    
private:
    Ui::REClefDialog *ui;
};

#endif // RECLEFDIALOG_H
