#ifndef RENAVIGATORROWITEM_H
#define RENAVIGATORROWITEM_H

#include <QGraphicsItem>

#include "RETypes.h"

class RENavigatorScene;

class RENavigatorRowItem : public QGraphicsItem
{
public:
    RENavigatorRowItem(const RERange& range, int trackIndex);

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    void CalculateBounds(const RENavigatorScene* navigator);
    void SetMinPitch(int mp) {_minPitch=mp;}
    void SetMaxPitch(int mp) {_maxPitch=mp;}

protected:
    RERange _barRange;
    int _trackIndex;
    QSizeF _bounds;
    int _minPitch;
    int _maxPitch;
};

#endif // RENAVIGATORROWITEM_H
