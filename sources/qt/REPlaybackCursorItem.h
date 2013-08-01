#ifndef REPLAYBACKCURSORITEM_H
#define REPLAYBACKCURSORITEM_H

#include <QGraphicsRectItem>

#include <RETypes.h>

class REQtViewport;

class REPlaybackCursorItem : public QGraphicsRectItem
{
    friend class REQtViewport;

public:
    explicit REPlaybackCursorItem(REQtViewport* viewport, const QRectF& rect);
    
protected:
    REQtViewport* _viewport;
};

#endif // REPLAYBACKCURSORITEM_H
