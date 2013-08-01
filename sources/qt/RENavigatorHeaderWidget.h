#ifndef RENAVIGATORHEADERWIDGET_H
#define RENAVIGATORHEADERWIDGET_H

#include "RETypes.h"

#include <QWidget>

class REDocumentView;

class RENavigatorHeaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RENavigatorHeaderWidget(QWidget *parent = 0);
    
    void SetDocumentView(REDocumentView*);

    int TickAtX(float x) const;
    float XOfTick(int tick) const;
    int BarIndexAtX(float x) const;
    int TotalWidth() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    REDocumentView* _documentView;
    
};

#endif // RENAVIGATORHEADERWIDGET_H
