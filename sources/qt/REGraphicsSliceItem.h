#ifndef REGRAPHICSSLICEITEM_H
#define REGRAPHICSSLICEITEM_H

#include <QGraphicsItem>

#include <RETypes.h>

class REGraphicsSliceItem : public QGraphicsItem
{
public:
    REGraphicsSliceItem(const RESlice* slice);
    virtual ~REGraphicsSliceItem() {}

public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

protected:
    const RESlice* _slice;
};

#endif // REGRAPHICSSYSTEMITEM_H
