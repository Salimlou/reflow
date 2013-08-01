//
//  REQtViewport.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 23/11/12.
//
//

#ifndef Reflow_REQtViewport_h
#define Reflow_REQtViewport_h

#include "REViewport.h"
#include "RETimer.h"

class QGraphicsItem;

class REGraphicsPageItem;
class REGraphicsSliceItem;
class REGraphicsSystemItem;
class REGraphicsFrameItem;
class RETabCursorItem;
class REPlaybackCursorItem;
class REScoreScene;
class REScoreSceneView;
class REDocumentView;


class REQtViewport : public REViewport
{
    friend class REDocumentView;

public:
    REQtViewport(REDocumentView* documentView);
    virtual ~REQtViewport();
    
public:
    virtual REViewportPageItem* CreatePageItem(const REPage* page);
    virtual REViewportSystemItem* CreateSystemItem(const RESystem* system);
    virtual REViewportSliceItem* CreateSliceItem(const RESlice* slice);
    virtual REViewportFrameItem* CreateFrameItem(const REFrame* frame);
    
    virtual void DestroyPageItem(REViewportPageItem* page);
    virtual void DestroySystemItem(REViewportSystemItem* system);
    virtual void DestroySliceItem(REViewportSliceItem* slice);
    virtual void DestroyFrameItem(REViewportFrameItem* frame);

    virtual RESize ViewportSize() const;    
    virtual RERect ViewportVisibleRect() const;
    
    virtual void SetViewportOffset(const REPoint& offset);

    virtual void UpdateContentSize();
    virtual void RepositionTabCursor();
    virtual void UpdatePlaybackCursor(float dt);
    virtual void UpdateVisibleViews();
    virtual void UpdateSelectionRect();
    
    virtual double TimeInSecondsSinceStart();
    
public:    
    REDocumentView* DocumentView() {return _documentView;}
    const REDocumentView* DocumentView() const {return _documentView;}

    REScoreScene* ScoreScene();
    const REScoreScene* ScoreScene() const;
    REScoreSceneView* ScoreSceneView();
    const REScoreSceneView* ScoreSceneView() const;
    REScoreController* ScoreController();

    void SafeDeleteGraphicsItem(QGraphicsItem*);
    
protected:
    void _UpdateZOrder();
    void _CreateControllerItems();
    
protected:
    REDocumentView* _documentView;
    RETabCursorItem* _tabCursor;
    REPlaybackCursorItem* _playbackCursor;
    RETimer _startDate;
};



class REQtViewportPageItem : public REViewportPageItem
{
    friend class REQtViewport;

public:
    REQtViewportPageItem(REQtViewport* viewport, const REPage* page);
    virtual ~REQtViewportPageItem();
    
    virtual REViewportSystemItem* CreateSystemItemInPage(const RESystem* system);
    
    REGraphicsPageItem* GraphicsItem() const {return _pageItem;}

    virtual void AttachToViewport(bool vis);
    virtual void SetNeedsDisplay();
    
protected:
    REGraphicsPageItem* _pageItem;
};



class REQtViewportSystemItem : public REViewportSystemItem
{
    friend class REQtViewport;

public:
    REQtViewportSystemItem(REQtViewport* viewport, const RESystem* system);
    virtual ~REQtViewportSystemItem();
    
    REGraphicsSystemItem* GraphicsItem() const {return _systemItem;}

    virtual void AttachToViewport(bool vis);    
    virtual void SetNeedsDisplay();
    virtual void UpdateFrame();

protected:
    REGraphicsSystemItem* _systemItem;
};



class REQtViewportSliceItem : public REViewportSliceItem
{
    friend class REQtViewport;

public:
    REQtViewportSliceItem(REQtViewport* viewport, const RESlice* slice);
    virtual ~REQtViewportSliceItem();
    
    REGraphicsSliceItem* GraphicsItem() const {return _sliceItem;}

    virtual void AttachToViewport(bool vis);
    virtual void SetNeedsDisplay();
    virtual void UpdateFrame();
    
protected:
    REGraphicsSliceItem* _sliceItem;
};



class REQtViewportFrameItem : public REViewportFrameItem
{
    friend class REQtViewport;

public:
    REQtViewportFrameItem(REQtViewport* viewport, const REFrame* frame);
    virtual ~REQtViewportFrameItem();

    REGraphicsFrameItem* GraphicsItem() const {return _frameItem;}

    virtual void AttachToViewport(bool vis);
    virtual void SetNeedsDisplay();
    virtual void UpdateFrame();

protected:
    REGraphicsFrameItem* _frameItem;
};

#endif
