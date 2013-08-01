#ifndef REREHEARSALDIALOG_H
#define REREHEARSALDIALOG_H

#include <QDialog>

namespace Ui {
class RERehearsalDialog;
}

class RERehearsalDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RERehearsalDialog(QWidget *parent = 0);
    ~RERehearsalDialog();
    
    void setRehearsalText(QString text);
    QString rehearsalText() const;

protected slots:
    void cleared();

private:
    Ui::RERehearsalDialog *ui;
};

#endif // REREHEARSALDIALOG_H
