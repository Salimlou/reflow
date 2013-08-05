#ifndef RETEMPOMARKERDIALOG_H
#define RETEMPOMARKERDIALOG_H

#include <QDialog>

#include "RETypes.h"
#include "RETimeline.h"

namespace Ui {
class RETempoMarkerDialog;
}

class RETempoMarkerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RETempoMarkerDialog(QWidget *parent = 0);
    ~RETempoMarkerDialog();

    void Initialize(int tempo, Reflow::TempoUnitType tempoUnitType);

    int Tempo() const;
    Reflow::TempoUnitType TempoUnitType() const;
    
protected slots:
    void cleared();

private:
    Ui::RETempoMarkerDialog *ui;
};

#endif // RETEMPOMARKERDIALOG_H
