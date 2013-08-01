#ifndef RETIMESIGNATUREDIALOG_H
#define RETIMESIGNATUREDIALOG_H

#include <QDialog>
#include "RETypes.h"

class REBar;

namespace Ui {
class RETimeSignatureDialog;
}

class RETimeSignatureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RETimeSignatureDialog(QWidget *parent = 0);
    ~RETimeSignatureDialog();
    
    void Initialize(const REBar* bar);

    int Numerator() const;
    int Denominator() const;
    REBeamingPattern BeamingPattern() const;

private:
    Ui::RETimeSignatureDialog *ui;
};

#endif // RETIMESIGNATUREDIALOG_H
