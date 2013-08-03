#include "REQtViewport.h"
#include "REScoreController.h"

#include "REDocumentView.h"
#include "REScoreScene.h"
#include "REScoreSceneView.h"
#include "REGraphicsPageItem.h"
#include "REGraphicsSystemItem.h"
#include "REGraphicsSliceItem.h"
#include "REGraphicsFrameItem.h"
#include "RETabCursorItem.h"
#include "REPlaybackCursorItem.h"

#include "REPage.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REFrame.h"

#include <boost/foreach.hpp>

#include <QScrollBar>


// ------------------------------------------------------------------------------------------------------------------
REQtViewport::REQtViewport(REDocumentView* documentView)
    : REViewport(documentView->ScoreController()), _documentView(documentView), _tabCursor(NULL), _playbackCursor(NULL)
{
    _startDate.Start();
}

// ------------------------------------------------------------------------------------------------------------------
REQtViewport::~REQtViewport()
{
}

// ------------------------------------------------------------------------------------------------------------------
REScoreScene* REQtViewport::ScoreScene()
{
    return _documentView->_scene;
}

// ------------------------------------------------------------------------------------------------------------------
const REScoreScene* REQtViewport::ScoreScene() const
{
    return _documentView->_scene;
}

// ------------------------------------------------------------------------------------------------------------------
REScoreSceneView* REQtViewport::ScoreSceneView()
{
    return _documentView->_scoreView;
}

// ------------------------------------------------------------------------------------------------------------------
const REScoreSceneView* REQtViewport::ScoreSceneView() const
{
    return _documentView->_scoreView;
}

// ------------------------------------------------------------------------------------------------------------------
REScoreController* REQtViewport::ScoreController()
{
    return _documentView->ScoreController();
}

// ------------------------------------------------------------------------------------------------------------------
REViewportPageItem* REQtViewport::CreatePageItem(const REPage* page)
{
    REQtViewportPageItem* pageItem = new REQtViewportPageItem(this, page);
    ScoreScene()->addItem(pageItem->GraphicsItem());
    return pageItem;
}

// ------------------------------------------------------------------------------------------------------------------
REViewportSystemItem* REQtViewport::CreateSystemItem(const RESystem* system)
{
    REQtViewportSystemItem* systemItem = new REQtViewportSystemItem(this, system);
    ScoreScene()->addItem(systemItem->GraphicsItem());
    return systemItem;
}

// ------------------------------------------------------------------------------------------------------------------
REViewportSliceItem* REQtViewport::CreateSliceItem(const RESlice* slice)
{
    REQtViewportSliceItem* sliceItem = new REQtViewportSliceItem(this, slice);
    ScoreScene()->addItem(sliceItem->GraphicsItem());
    return sliceItem;
}

// ------------------------------------------------------------------------------------------------------------------
REViewportFrameItem* REQtViewport::CreateFrameItem(const REFrame* frame)
{
    REQtViewportFrameItem* frameItem = new REQtViewportFrameItem(this, frame);
    const REQtViewportPageItem* pageItem = static_cast<const REQtViewportPageItem*>(frame->Parent()->ViewportItem());
    if(pageItem) {
        frameItem->GraphicsItem()->setParentItem(pageItem->GraphicsItem());
    }
    else {
        ScoreScene()->addItem(frameItem->GraphicsItem());
    }
    return frameItem;
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::DestroyPageItem(REViewportPageItem* page)
{
    auto qpage = static_cast<REQtViewportPageItem*>(page);
    SafeDeleteGraphicsItem(qpage->GraphicsItem());
    qpage->_pageItem = nullptr;
    delete page;
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::DestroySystemItem(REViewportSystemItem* system)
{
    auto qsystem = static_cast<REQtViewportSystemItem*>(system);
    SafeDeleteGraphicsItem(qsystem->GraphicsItem());
    qsystem->_systemItem = nullptr;
    delete system;
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::DestroySliceItem(REViewportSliceItem* slice)
{
    auto qslice = static_cast<REQtViewportSliceItem*>(slice);
    SafeDeleteGraphicsItem(qslice->GraphicsItem());
    qslice->_sliceItem = nullptr;
    delete slice;
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::DestroyFrameItem(REViewportFrameItem* frame)
{
    auto qframe = static_cast<REQtViewportFrameItem*>(frame);
    SafeDeleteGraphicsItem(qframe->GraphicsItem());
    qframe->_frameItem = nullptr;
    delete frame;
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::SafeDeleteGraphicsItem(QGraphicsItem* item)
{
    if(item == nullptr) return;

    if(item->parentItem() == nullptr)
    {
        QGraphicsScene* scene = item->scene();
        if(scene) scene->removeItem(item);
        delete item;
    }
}

// ------------------------------------------------------------------------------------------------------------------
RESize REQtViewport::ViewportSize() const
{
    QSizeF size = ScoreSceneView()->size();
    return RESize(size.width(), size.height());
}

// ------------------------------------------------------------------------------------------------------------------
RERect REQtViewport::ViewportVisibleRect() const
{
    // TODO: Change this to return the actual visible rect instead of the whole scene
    return RERect(ScoreScene()->sceneRect());
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::SetViewportOffset(const REPoint& offset)
{
    REViewport::SetViewportOffset(offset);

    QRectF sceneRect = ScoreSceneView()->sceneRect();
    ScoreSceneView()->horizontalScrollBar()->setValue(offset.x);
    ScoreSceneView()->verticalScrollBar()->setValue(offset.y);
    //ScoreSceneView()->setSceneRect(offset.x, offset.y, sceneRect.width(), sceneRect.height());
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::UpdateContentSize()
{
    RESize newSize = Score()->ContentSize();
    ScoreScene()->setSceneRect(0, 0, newSize.w, newSize.h);
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::RepositionTabCursor()
{
    REViewport::RepositionTabCursor();

    _tabCursor->setVisible(_tabInputCursorVisible);
    _tabCursor->_rects = _tabInputCursorSubRects;
    _tabCursor->prepareGeometryChange();
    _tabCursor->_size = _tabInputCursorRect.size;
    _tabCursor->setPos(_tabInputCursorRect.origin.ToQPointF());

    _UpdateZOrder();

    // TODO: Update UI From Cursor
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::UpdatePlaybackCursor(float dt)
{
    _playbackTrackingEnabled = _documentView->IsTrackingEnabled();
    _playbackTrackingPause = 0.0;

    REViewport::UpdatePlaybackCursor(dt);
    
    _playbackCursor->setRect(_playbackCursorRect.ToQRectF());
    _playbackCursor->setVisible(_playbackCursorVisible);

    _UpdateZOrder();
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::UpdateVisibleViews()
{
    REViewport::UpdateVisibleViews();
    _UpdateZOrder();
}

void REQtViewport::UpdateSelectionRect()
{

}


// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::_UpdateZOrder()
{
    _playbackCursor->setZValue(1.0);
    _playbackCursor->update();

    _tabCursor->setZValue(2.0);
    _tabCursor->update();
    //_selectionRectItem->setZValue(3.0)
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewport::_CreateControllerItems()
{
    _tabCursor = new RETabCursorItem(this);
    ScoreScene()->addItem(_tabCursor);

    _playbackCursor = new REPlaybackCursorItem(this, QRectF(0, 0, 0, 0));
    _playbackCursor->setVisible(false);
    _playbackCursor->setPen(QPen());
    _playbackCursor->setBrush(QBrush(QColor::fromRgbF(1.0f, 0.22f, 0.19f, 0.35f)));
    ScoreScene()->addItem(_playbackCursor);
}

// ------------------------------------------------------------------------------------------------------------------
double REQtViewport::TimeInSecondsSinceStart()
{
    return _startDate.DeltaTimeInSeconds();
}








// ------------------------------------------------------------------------------------------------------------------
//  REQtViewportPageItem
// ------------------------------------------------------------------------------------------------------------------
REQtViewportPageItem::REQtViewportPageItem(REQtViewport* viewport, const REPage* page)
: REViewportPageItem(viewport, page), _pageItem(NULL)
{
    RERect frame = page->Frame();
    QPen pen = QPen(QColor(0, 0, 0));
    QBrush brush = QBrush(QColor(255, 255, 255));
    _pageItem = new REGraphicsPageItem(page->Number(), QRectF(0, 0, frame.Width(), frame.Height()));
    _pageItem->setPos(frame.origin.ToQPointF());
    _pageItem->setPen(pen);
    _pageItem->setBrush(brush);
}

// ------------------------------------------------------------------------------------------------------------------
REQtViewportPageItem::~REQtViewportPageItem()
{
    assert(_pageItem == NULL);
}

// ------------------------------------------------------------------------------------------------------------------
void REQtViewportPageItem::AttachToViewport(bool vis)
{
    if(_pageItem)
    {
        _pageItem->setVisible(vis);
    }
}

void REQtViewportPageItem::SetNeedsDisplay()
{
    if(_pageItem) _pageItem->update();
}

// ------------------------------------------------------------------------------------------------------------------
REViewportSystemItem* REQtViewportPageItem::CreateSystemItemInPage(const RESystem* system)
{
    REQtViewportSystemItem* systemItem = new REQtViewportSystemItem((REQtViewport*)_viewport, system);
    
    systemItem->GraphicsItem()->setParentItem(GraphicsItem());
    
    return systemItem;
}




// ------------------------------------------------------------------------------------------------------------------
//  REQtViewportSystemItem
// ------------------------------------------------------------------------------------------------------------------
REQtViewportSystemItem::REQtViewportSystemItem(REQtViewport* viewport, const RESystem* system)
: REViewportSystemItem(viewport, system), _systemItem(NULL)
{
    const REScoreController* scoreController = viewport->DocumentView()->ScoreController();
    RERect frame = system->Frame();

    _systemItem = new REGraphicsSystemItem(scoreController, system);
    _systemItem->setPos(frame.origin.ToQPointF());
}

REQtViewportSystemItem::~REQtViewportSystemItem()
{
    assert(_systemItem == NULL);
}

void REQtViewportSystemItem::AttachToViewport(bool vis)
{
    if(_systemItem) _systemItem->setVisible(vis);
}

void REQtViewportSystemItem::SetNeedsDisplay()
{
    if(_systemItem) _systemItem->update();
}

void REQtViewportSystemItem::UpdateFrame()
{
    if(_systemItem) _systemItem->update();
}


// ------------------------------------------------------------------------------------------------------------------
//  REQtViewportSliceItem
// ------------------------------------------------------------------------------------------------------------------
REQtViewportSliceItem::REQtViewportSliceItem(REQtViewport* viewport, const RESlice* slice)
: REViewportSliceItem(viewport, slice), _sliceItem(NULL)
{
    RERect frame = slice->Frame();

    _sliceItem = new REGraphicsSliceItem(slice);
    _sliceItem->setPos(frame.origin.ToQPointF());
}
REQtViewportSliceItem::~REQtViewportSliceItem()
{
    assert(_sliceItem == NULL);
}

void REQtViewportSliceItem::AttachToViewport(bool vis)
{
    if(_sliceItem) _sliceItem->setVisible(vis);
}

void REQtViewportSliceItem::SetNeedsDisplay()
{
    if(_sliceItem) _sliceItem->update();
}

void REQtViewportSliceItem::UpdateFrame()
{
    if(_sliceItem) _sliceItem->update();
}

// ------------------------------------------------------------------------------------------------------------------
//  REQtViewportFrameItem
// ------------------------------------------------------------------------------------------------------------------
REQtViewportFrameItem::REQtViewportFrameItem(REQtViewport *viewport, const REFrame *frame)
    : REViewportFrameItem(viewport, frame), _frameItem(nullptr)
{
    RERect rc = frame->Frame();
    _frameItem = new REGraphicsFrameItem(frame);
    _frameItem->setPos(rc.origin.ToQPointF());
}

REQtViewportFrameItem::~REQtViewportFrameItem()
{
    assert(_frameItem == nullptr);
}

void REQtViewportFrameItem::AttachToViewport(bool vis)
{
    if(_frameItem) _frameItem->setVisible(vis);
}

void REQtViewportFrameItem::SetNeedsDisplay()
{
    if(_frameItem) _frameItem->update();
}

void REQtViewportFrameItem::UpdateFrame()
{
    if(_frameItem) _frameItem->update();
}

