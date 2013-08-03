#include "RETabCursorItem.h"
#include "REQtViewport.h"
#include "REScoreScene.h"

#include <QPainter>

#include <REPainter.h>

#define DIM_FACTOR      0.50

RETabCursorItem::RETabCursorItem(REQtViewport* viewport) :
    QGraphicsItem(NULL), _dimmed(false), _typingSecondDigit(false), _size(10,10), _viewport(viewport)
{
}

QRectF RETabCursorItem::boundingRect() const
{
    return QRectF(QPointF(0,0), _size.ToQSizeF());
}

void RETabCursorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
{
    float alphaFactor = 1.0;
    if(_dimmed) alphaFactor = DIM_FACTOR;

    painter->setPen(QColor::fromRgbF(0.20, 0.25, 0.85, 0.95 * alphaFactor));
    QBrush brush = QBrush(QColor::fromRgbF(0.20, 0.25, 0.85, 0.24 * alphaFactor));

    if(!_rects.empty())
    {
        float dx = scenePos().x();
        float dy = scenePos().y();
        for(const RERect& subrect : _rects)
        {
            RERect rc = subrect;
            rc.origin.x = rc.origin.x - dx + 0.5;
            rc.origin.y = rc.origin.y - dy + 0.5;
            rc.size.w-= 1.0;
            rc.size.h -= 1.0;

            painter->fillRect(rc.ToQRectF(), brush);
            painter->drawRect(rc.ToQRectF());
        }
    }
    else
    {
        RERect rc = RERect(boundingRect());
        rc.origin.x += 0.5;
        rc.origin.y += 0.5;
        rc.size.w -= 1.0;
        rc.size.h -= 1.0;

        painter->fillRect(rc.ToQRectF(),
                          _typingSecondDigit ? QBrush(QColor::fromRgbF(1,1,1,0.20)) : brush);
        painter->drawRect(rc.ToQRectF());
    }
}
