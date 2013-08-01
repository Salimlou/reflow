#ifndef REGRAPHICSFRAMEITEM_H
#define REGRAPHICSFRAMEITEM_H

#include <QGraphicsItem>

#include <RETypes.h>

class REGraphicsFrameItem : public QGraphicsItem
{
public:
    REGraphicsFrameItem(const REFrame* frame);
    virtual ~REGraphicsFrameItem() {}

public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    const REFrame* _frame;
};

#endif // REGRAPHICSFRAMEITEM_H
