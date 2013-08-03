#ifndef REGRAPHICSSYSTEMITEM_H
#define REGRAPHICSSYSTEMITEM_H

#include <QGraphicsItem>

#include <RETypes.h>

class REGraphicsSystemItem : public QGraphicsItem
{
public:
    REGraphicsSystemItem(const REScoreController* scoreController, const RESystem* system, QGraphicsItem* parentItem=0);
    virtual ~REGraphicsSystemItem(){}

public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

protected:
    const RESystem* _system;
    const REScoreController* _scoreController;
};

#endif // REGRAPHICSSYSTEMITEM_H
