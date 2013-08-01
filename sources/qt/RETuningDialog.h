#ifndef RETUNINGDIALOG_H
#define RETUNINGDIALOG_H

#include <QDialog>

class RETrack;

namespace Ui {
class RETuningDialog;
}

class RETuningDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RETuningDialog(QWidget *parent = 0);
    ~RETuningDialog();
    
    void UpdateUIFromTrack(const RETrack* track);

    const QVector<int>& TuningArray() const {return _tuningArray;}

private:
    void CreateScrollAreaContents();

protected slots:
    void on_addOneLowStringButton_clicked();
    void on_removeOneLowStringButton_clicked();
    void on_allStringsDownButton_clicked();
    void on_allStringsUpButton_clicked();

protected slots:
    void changeTuningOfString(int stringIndex, int pitch);

private:
    Ui::RETuningDialog *ui;
    QVector<int> _tuningArray;
};

#endif // RETUNINGDIALOG_H
