#include "REGraphicsPageItem.h"

REGraphicsPageItem::REGraphicsPageItem(int pageIndex, const QRectF& frame)
    : QGraphicsRectItem(frame, 0), _pageIndex(pageIndex)
{
}
