#ifndef RECLEFPREVIEW_H
#define RECLEFPREVIEW_H

#include <QWidget>

#include "RETypes.h"

class REClefPreview : public QWidget
{
    Q_OBJECT
public:
    explicit REClefPreview(QWidget *parent = 0);

    void SetClef(Reflow::ClefType clef);
    void SetOttavia(Reflow::OttaviaType ottavia);

    Reflow::ClefType Clef() const {return _clef;}
    Reflow::OttaviaType Ottavia() const {return _ottavia;}

protected:
    virtual void paintEvent(QPaintEvent*);

private:
    Reflow::ClefType _clef;
    Reflow::OttaviaType _ottavia;
};

#endif // RECLEFPREVIEW_H
