#ifndef RETEXTDIALOG_H
#define RETEXTDIALOG_H

#include <QDialog>

class REChord;

namespace Ui {
class RETextDialog;
}

class RETextDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RETextDialog(QWidget *parent = 0);
    ~RETextDialog();

    void InitializeFromChord(const REChord* chord);

    QString Text() const;
    int PositioningIndex() const;
    
private:
    Ui::RETextDialog *ui;
};

#endif // RETEXTDIALOG_H
