//
//  REViewport.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 23/11/12.
//
//

#ifndef __Reflow__REViewport__
#define __Reflow__REViewport__

#include "RETypes.h"
#include "REManipulator.h"

/** REViewport
 */
class REViewport
{
    friend class RESongController;
    
public:
    REViewport(REScoreController* scoreController);
    virtual ~REViewport() {}
    
public:
    void Build();
	void RefreshItemAtBarIndex(int barIndex);
    void Clear();
    void RebuildManipulators();
    
public:
    const REScoreController* ScoreController() const {return _scoreController;}
    const REScore* Score() const;
    REScore* Score();
    
    REPoint PositionOfCursor(const RECursor& cursor) const;
    REPoint PositionOfCursorForGraceNote(const RECursor& cursor, int graceNoteIndex) const;
    
    void Update();
    
    void UpdatePlaybackCursorStatus(const RESequencer* sequencer);
    
public:
    virtual REViewportPageItem* CreatePageItem(const REPage* page) {return nullptr;}
    virtual REViewportSystemItem* CreateSystemItem(const RESystem* system) {return nullptr;}
    virtual REViewportSliceItem* CreateSliceItem(const RESlice* slice) {return nullptr;}
    virtual REViewportFrameItem* CreateFrameItem(const REFrame* frame) {return nullptr;}
    virtual REViewportManipulatorItem* CreateManipulatorItem(REManipulator* manipulator) {return nullptr;}

    virtual void DestroyPageItem(REViewportPageItem* page) {}
    virtual void DestroySystemItem(REViewportSystemItem* system) {}
    virtual void DestroySliceItem(REViewportSliceItem* slice) {}
    virtual void DestroyFrameItem(REViewportFrameItem* frame) {}
    virtual void DestroyManipulatorItem(REViewportManipulatorItem* manipulator) {}

    virtual void SetViewportSize(const RESize& sz) {_size = sz;}
    virtual RESize ViewportSize() const {return _size;}
    virtual RERect ViewportVisibleRect() const {return RERect(_offset, _size);}
    
    virtual void SetViewportOffset(const REPoint& offset) {_offset = offset;}
    REPoint ViewportOffset() const {return _offset;}
    
    virtual void UpdateContentSize() {}
    virtual void RepositionTabCursor();
    virtual void UpdatePlaybackCursor(float dt);
    virtual void UpdateVisibleViews();
    virtual void UpdateSelectionRect() {}
    
    virtual void PlaybackStarted() {}
    virtual void PlaybackStopped() {}
    virtual void OnPlaybackFinishedRT() {}
    
    virtual void OnMouseDoubleClicked(const REPoint& point, unsigned long flags) {}
    
    virtual double TimeInSecondsSinceStart() {return 0.0;}
    
    int IndexOfCurrentBarPlaying() const {return _barPlaying;}
    double TickInBarCurrentlyPlaying() const {return _tickInBarPlaying;}
    
protected:
    void SmoothUpdatePlayback(float dt);
    void _UpdatePlaybackRT(const RESequencer* sequencer);
    
protected:
    REScoreController* _scoreController;
    RESize _size;
    REPoint _offset;
    
    // Tab Input Tool
    bool _tabInputCursorVisible;
    bool _tabInputGraceCursorVisible;
    RERectVector _tabInputCursorSubRects;
    RERect _tabInputCursorRect;
    RERect _tabInputGraceCursorRect;
    
    // Select (Arrow) Tool
    REPoint _selectionStartPoint;
    REPoint _selectionEndPoint;
    bool _selectionRectVisible;
    
    // Non realtime
    bool _playbackCursorVisible;
    bool _playbackRunning;
    int _barPlaying;
    double _tickInBarPlaying;
    double _currentUpdate;
    double _lastPlaybackUpdate;
    RERect _currentPlaybackCursorRect;
    RERect _lastPlaybackCursorRect;
    RERect _playbackCursorRect;
    RERect _currentSystemRect;
    RERect _nextSystemRect;
    
    // Realtime audio Thread
    bool _playbackRunningRT;
    int _barPlayingRT;
    double _tickInBarPlayingRT;
    double _currentUpdateRT;
    
    // Playback Tracking
    bool _playbackTrackingEnabled;
    double _playbackTrackingPause;
    REPointSmoother _playbackTrackingSmoother;
};


/** REViewportItem
 */
class REViewportItem
{
public:
    enum ViewportItemType
    {
        PageItem,
        SystemItem,
        SliceItem,
        FrameItem,
        ManipulatorItem
    };
    
protected:
    REViewportItem(REViewport* vp) : _viewport(vp) {}
    
public:
    virtual ~REViewportItem() {}
    
    virtual void AttachToViewport(bool vis) = 0;
    
public:
    virtual ViewportItemType ItemType() const = 0;
    virtual void SetNeedsDisplay() = 0;
    
protected:
    REViewport* _viewport;
};

/** REViewportPageItem 
 */
class REViewportPageItem : public REViewportItem
{
public:
    REViewportPageItem(REViewport* viewport, const REPage* page) : REViewportItem(viewport), _page(page) {}
    virtual ~REViewportPageItem() {}
    
    virtual ViewportItemType ItemType() const {return REViewportItem::PageItem;}
    
    void CreateSystemItems();

    virtual REViewportSystemItem* CreateSystemItemInPage(const RESystem* system) =0;
    
protected:
    const REPage* _page;
};


/** REViewportSystemItem
 */
class REViewportSystemItem : public REViewportItem
{
public:
    REViewportSystemItem(REViewport* vp, const RESystem* system) : REViewportItem(vp), _system(system) {}
    virtual ~REViewportSystemItem() {}
    
    virtual ViewportItemType ItemType() const {return REViewportItem::SystemItem;}

    virtual void UpdateFrame() = 0;
    
protected:
    const RESystem* _system;
};


/** REViewportSliceItem
 */
class REViewportSliceItem : public REViewportItem
{
public:
    REViewportSliceItem(REViewport* vp, const RESlice* slice) : REViewportItem(vp), _slice(slice) {}
    virtual ~REViewportSliceItem() {}
    
    virtual ViewportItemType ItemType() const {return REViewportItem::SliceItem;}

    virtual void UpdateFrame() = 0;    
    
protected:
    const RESlice* _slice;
};

/** REViewportFrameItem
 */
class REViewportFrameItem : public REViewportItem
{
public:
    REViewportFrameItem(REViewport* vp, const REFrame* frame) : REViewportItem(vp), _frame(frame) {}
    virtual ~REViewportFrameItem() {}
    
    virtual ViewportItemType ItemType() const {return REViewportItem::FrameItem;}
    
protected:
    const REFrame* _frame;
};

/** REViewportManipulatorItem
 */
class REViewportManipulatorItem : public REViewportItem
{
public:
    REViewportManipulatorItem(REViewport* vp, REManipulator* manipulator) : REViewportItem(vp), _manipulator(manipulator) {}
    
    virtual ~REViewportManipulatorItem() {}
    
    virtual ViewportItemType ItemType() const {return REViewportItem::ManipulatorItem;}
    
    virtual void UpdateFrame() = 0;
    virtual void StartEditing() = 0;
    virtual void FinishedEditing() = 0;
    
protected:
    REManipulator* _manipulator;
};


#endif /* defined(__Reflow__REViewport__) */
