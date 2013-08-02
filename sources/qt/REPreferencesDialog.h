#ifndef REPREFERENCESDIALOG_H
#define REPREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class REPreferencesDialog;
}

class REPreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REPreferencesDialog(QWidget *parent = 0);
    ~REPreferencesDialog();
    
private:
    Ui::REPreferencesDialog *ui;
};

#endif // REPREFERENCESDIALOG_H
