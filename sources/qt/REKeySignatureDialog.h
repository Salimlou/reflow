#ifndef REKEYSIGNATUREDIALOG_H
#define REKEYSIGNATUREDIALOG_H

#include <QDialog>

#include "RETypes.h"

namespace Ui {
class REKeySignatureDialog;
}

class REKeySignatureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REKeySignatureDialog(QWidget *parent = 0);
    ~REKeySignatureDialog();

    void SetKeySignature(const REKeySignature& sig);
    const REKeySignature& KeySignature() const {return _keySignature;}
    
protected slots:
    void RefreshKeySignatureNames(bool minor);
    void on_minorCheckbox_toggled(bool);
    void on_keySignatureCombo_currentIndexChanged(int);

private:
    Ui::REKeySignatureDialog *ui;
    REKeySignature _keySignature;
};

#endif // REKEYSIGNATUREDIALOG_H
