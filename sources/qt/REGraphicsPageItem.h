#ifndef REGRAPHICSPAGEITEM_H
#define REGRAPHICSPAGEITEM_H

#include <QGraphicsRectItem>

class REGraphicsPageItem : public QGraphicsRectItem
{
public:
    REGraphicsPageItem(int pageIndex, const QRectF& frame);
    virtual ~REGraphicsPageItem() {}

protected:
    int _pageIndex;
};

#endif // REGRAPHICSPAGEITEM_H
