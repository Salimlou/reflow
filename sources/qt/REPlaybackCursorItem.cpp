#include "REPlaybackCursorItem.h"
#include "REQtViewport.h"
#include "REScoreScene.h"

REPlaybackCursorItem::REPlaybackCursorItem(REQtViewport* viewport, const QRectF& rect) :
    QGraphicsRectItem(rect, 0), _viewport(viewport)
{
}
