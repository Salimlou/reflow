#ifndef RETABCURSORITEM_H
#define RETABCURSORITEM_H

#include <QGraphicsItem>

#include <RETypes.h>

class REQtViewport;

class RETabCursorItem : public QGraphicsItem
{
    friend class REQtViewport;

public:
    explicit RETabCursorItem(REQtViewport* viewport);
    
public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    bool _dimmed;
    bool _typingSecondDigit;
    RESize _size;
    RERectVector _rects;
    REQtViewport* _viewport;
};

#endif // RETABCURSORITEM_H
