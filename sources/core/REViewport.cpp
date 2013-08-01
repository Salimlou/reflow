//
//  REViewport.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 23/11/12.
//
//

#include "REViewport.h"

#include "REScore.h"
#include "REScoreRoot.h"
#include "REPage.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REFrame.h"
#include "RESong.h"
#include "REScoreController.h"
#include "RESongController.h"
#include "RESequencer.h"
#include "REStaff.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REPhrase.h"
#include "REChord.h"
#include "REBarMetrics.h"
#include "REBar.h"
#include "RENote.h"

#include <cmath>
#include <algorithm>

#ifdef REFLOW_QT
#  include <QDebug>
#endif

#include <boost/foreach.hpp>

REViewport::REViewport(REScoreController* scoreController)
: _scoreController(scoreController)
{
    _tabInputCursorVisible = false;
    _tabInputCursorRect = RERect(0,0,0,0);
    
    _playbackCursorVisible = false;
    _playbackRunning = false;
    _barPlaying = 0;
    _tickInBarPlaying = 0.0;
    _currentUpdate = 0;
    _lastPlaybackUpdate = 0;
    _currentPlaybackCursorRect = RERect(0,0,0,0);
    _lastPlaybackCursorRect = RERect(0,0,0,0);
    _playbackCursorRect = RERect(0,0,0,0);
    _currentSystemRect = RERect(0,0,0,0);
    
    // Realtime audio Thread
    _playbackRunningRT = false;
    _barPlayingRT = 0;
    _tickInBarPlayingRT = 0;
    _currentUpdateRT = 0;
    
    _playbackTrackingEnabled = false; //true;
    _playbackTrackingPause = 0.0;
    _playbackTrackingSmoother.speed = 1.00;
}

void REViewport::Build()
{
    REScore* score = _scoreController->Score();
    REScoreRoot* root = score->Root();
    Reflow::ScoreLayoutType layout = _scoreController->LayoutType();

    switch(layout)
    {
        case Reflow::PageScoreLayout:
        {
            for(REPage* page : root->Pages())
            {
                REViewportPageItem* pageItem = CreatePageItem(page);
                if(!pageItem) continue;
                
                page->SetViewportItem(pageItem);
                pageItem->CreateSystemItems();
                
                for(REFrame* frame : page->TextFrames())
                {
#ifdef REFLOW_QT
                    qDebug("Creating frame for page %d with text %s\n", page->Number(), frame->Text().c_str());
#endif
                    REViewportFrameItem* frameItem = CreateFrameItem(frame);
                    frame->SetViewportItem(frameItem);
                }
            }
            break;
        }

        case Reflow::HorizontalScoreLayout:
        {
            RESystem* hsystem = score->System(0);
            for(RESlice* slice : hsystem->Slices())
            {
                REViewportSliceItem* sliceItem = CreateSliceItem(slice);
                if(!sliceItem) continue;
                slice->SetViewportItem(sliceItem);
            }
            break;
        }
    }
    
    // Create Manipulators
    RETool* tool = _scoreController->CurrentTool();
    if(tool) tool->CreateManipulators();
    
    UpdateContentSize();
    RepositionTabCursor();
    //UpdatePlaybackCursor();
    UpdateVisibleViews();
}

void REViewport::RefreshItemAtBarIndex(int barIndex)
{
	REScore* score = _scoreController->Score();
    REScoreRoot* root = score->Root();
    Reflow::ScoreLayoutType layout = _scoreController->LayoutType();

    switch(layout)
    {
        case Reflow::PageScoreLayout:
		{
			RESystem* system = score->SystemWithBarIndex(barIndex);
			if(system && system->ViewportItem()) system->ViewportItem()->SetNeedsDisplay();
			break;
        }
            
        case Reflow::HorizontalScoreLayout:
        {
            RESystem* hsystem = score->System(0);
			RESlice* slice = hsystem->SliceWithBarIndex(barIndex);
			if(slice && slice->ViewportItem()) slice->ViewportItem()->SetNeedsDisplay();
            break;
        }
    }
    
    UpdateContentSize();
    RepositionTabCursor();
    //UpdatePlaybackCursor();
    UpdateVisibleViews();
}

void REViewport::RebuildManipulators()
{
    // Destroy manipulators of every tool
    for(RETool* tool : _scoreController->Tools()) {
        if(tool) tool->DestroyManipulators();
    }
    
    // Create Manipulators
    RETool* tool = _scoreController->CurrentTool();
    if(tool) tool->CreateManipulators();
    
    RepositionTabCursor();
    UpdateVisibleViews();
}

void REViewport::Clear()
{
    REScore* score = _scoreController->Score();
    REScoreRoot* root = score->Root();
	if(root == NULL) return;

    // Destroy manipulators of every tool
    for(RETool* tool : _scoreController->Tools()) {
        if(tool) tool->DestroyManipulators();
    }
    
    // Destroy text frames of every pages
    for(REPage* page : root->Pages()) {
        for(REFrame* frame : page->TextFrames())
        {
#ifdef REFLOW_QT
            qDebug("Destroying frame for page %d with text %s\n", page->Number(), frame->Text().c_str());
#endif
            REViewportFrameItem* frameItem = static_cast<REViewportFrameItem*>(frame->ViewportItem());
            if(frameItem) {
                DestroyFrameItem(frameItem);
                frame->SetViewportItem(nullptr);
            }
        }
    }
    
    Reflow::ScoreLayoutType layout = _scoreController->LayoutType();

	switch(layout)
    {
        case Reflow::PageScoreLayout:
        {
            for(RESystem* system : score->Systems())
            {
                REViewportSystemItem* systemItem = static_cast<REViewportSystemItem*>(system->ViewportItem());
                if(systemItem) {
                    DestroySystemItem(systemItem);
                    system->SetViewportItem(NULL);
                }
            }

            for(REPage* page : root->Pages())
            {
                REViewportPageItem* pageItem = static_cast<REViewportPageItem*>(page->ViewportItem());
                if(pageItem) {
                    DestroyPageItem(pageItem);
                    page->SetViewportItem(NULL);
                }
            }
            break;
        }

        case Reflow::HorizontalScoreLayout:
        {
            RESystem* hsystem = score->System(0);
            for(RESlice* slice : hsystem->Slices())
            {
                REViewportSliceItem* sliceItem = static_cast<REViewportSliceItem*>(slice->ViewportItem());
                if(sliceItem) {
                    DestroySliceItem(sliceItem);
                    slice->SetViewportItem(NULL);
                }
            }
            break;
        }
    }
}

const REScore* REViewport::Score() const
{
    return _scoreController ? _scoreController->Score() : NULL;
}

REScore* REViewport::Score()
{
    return _scoreController ? _scoreController->Score() : NULL;
}

REPoint REViewport::PositionOfCursor(const RECursor& cursor) const
{
    const REScore* score = _scoreController->Score();
    
    int barIndex = cursor.BarIndex();
    int voiceIndex = cursor.VoiceIndex();
    int staffIndex = cursor.StaffIndex();
    int chordIndex = cursor.ChordIndex();
    int lineIndex = cursor.LineIndex();
    
    const RESystem* system = score->SystemWithBarIndex(barIndex);
    int staffCount = system->StaffCount();
    if(staffCount == 0) {
        return REPoint(0,0);
    }
    
    if(staffIndex >= staffCount) {
        staffIndex = staffCount-1;
    }
    const REStaff* staff = system->Staff(staffIndex);
    const RETrack* track = staff->Track();
    const REVoice* voice = track->Voice(voiceIndex);
    const REPhrase* phrase = voice->Phrase(barIndex);
    if(chordIndex < 0) chordIndex = 0;
    if(chordIndex > phrase->ChordCount()) chordIndex = phrase->ChordCount();
    
    const REChord* chord = phrase->Chord(chordIndex);
    const RESlice* slice = system->SystemBarWithBarIndex(barIndex);
    unsigned long tick = (chord ? chord->OffsetInTicks() : 0);
    
    RERect sliceFrame = slice->SceneFrame();
    float x = roundf(sliceFrame.origin.x + slice->XOffsetOfTick(tick));
    float y = roundf(sliceFrame.origin.y + staff->YOffset() + staff->YOffsetOfLine(lineIndex));
    
    return REPoint(x, y);
}

REPoint REViewport::PositionOfCursorForGraceNote(const RECursor& cursor, int graceNoteIndex) const
{
    const REScore* score = _scoreController->Score();
    
    int barIndex = cursor.BarIndex();
    int voiceIndex = cursor.VoiceIndex();
    int staffIndex = cursor.StaffIndex();
    int chordIndex = cursor.ChordIndex();
    int lineIndex = cursor.LineIndex();
    
    const RESystem* system = score->SystemWithBarIndex(barIndex);
    int staffCount = system->StaffCount();
    if(staffCount == 0) {
        return REPoint(0,0);
    }
    
    if(staffIndex >= staffCount) {
        staffIndex = staffCount-1;
    }
    const REStaff* staff = system->Staff(staffIndex);
    const RETrack* track = staff->Track();
    const REVoice* voice = track->Voice(voiceIndex);
    const REPhrase* phrase = voice->Phrase(barIndex);
    if(chordIndex < 0) chordIndex = 0;
    if(chordIndex > phrase->ChordCount()) chordIndex = phrase->ChordCount();
    
    const REChord* chord = phrase->Chord(chordIndex);
    const RESlice* slice = system->SystemBarWithBarIndex(barIndex);
    unsigned long tick = (chord ? chord->OffsetInTicks() : 0);
    
    RERect sliceFrame = slice->SceneFrame();
    float noteX = roundf(sliceFrame.origin.x + slice->XOffsetOfTick(tick));
    float noteY = roundf(sliceFrame.origin.y + staff->YOffset() + staff->YOffsetOfLine(lineIndex));
    
    const RENote* note = cursor.Note();
    if(note && graceNoteIndex >= 0 && graceNoteIndex < note->GraceNoteCount())
    {
        REGraceNoteMetrics graceNoteMetrics;
        note->CalculateGraceNoteMetrics(score->IsTransposing(), (0.5 + staff->UnitSpacing()) * 0.75, &graceNoteMetrics);
        float stretch = slice->StretchFactor();
        float graceX = noteX - stretch * (graceNoteMetrics.Width() + graceNoteMetrics.XOffsetOfNote(graceNoteIndex));
        return REPoint(graceX, noteY);
    }
    else {
        return REPoint(noteX, noteY);
    }
}

void REViewport::RepositionTabCursor()
{
    const REScore* score = _scoreController->Score();
    
    const RESong* song = score->Song();
    _tabInputCursorRect = RERect(0,0,0,0);
    _tabInputCursorSubRects.clear();
    _tabInputCursorVisible = (score != NULL && song->BarCount() != 0);
    
    _tabInputGraceCursorVisible = false;
    _tabInputGraceCursorRect = RERect(0,0,0,0);
    
    if(!_tabInputCursorVisible) {
        return;
    }
    
    const float unitSpacing = 7.0;
    
    const RECursor& cursor = _scoreController->Cursor();
    const RECursor& originCursor = _scoreController->OriginCursor();
    const RESystem* sA = originCursor.System();
    const RESystem* sB = cursor.System();
    
    int barA = originCursor.BarIndex();
    int barB = cursor.BarIndex();
    int ia = sA->Index();
    int ib = (sB ? sB->Index() : ia);
    
    if(_scoreController->InferredSelectionKind() == REScoreController::CursorSelection)
    {
        REPoint pt = PositionOfCursor(_scoreController->Cursor());
        float w = roundf(2.50 * unitSpacing);
        float h = roundf(1.50 * unitSpacing);
        _tabInputCursorRect = RERect(pt.x-w/2, pt.y-h/2, w, h);
        
        const RENote* noteUnder = _scoreController->Cursor().Note();
        if(_scoreController->IsEditingGraceNote() && noteUnder && noteUnder->GraceNoteCount() > 0)
        {
            REPoint pt = PositionOfCursorForGraceNote(_scoreController->Cursor(), _scoreController->SelectedGraceNoteIndex());
            float w = roundf(1.80 * unitSpacing);
            float h = roundf(1.15 * unitSpacing);
            _tabInputGraceCursorRect = RERect(pt.x-w/2, pt.y-h/2, w, h);
            _tabInputGraceCursorVisible = true;
        }
    }
    else if(_scoreController->InferredSelectionKind() == REScoreController::RectangleCursorSelection)
    {
        REPoint pt = PositionOfCursor(cursor);
        REPoint ptOrig = PositionOfCursor(originCursor);
        double x0 = (pt.x > ptOrig.x ? ptOrig.x : pt.x) - 1.25 * unitSpacing;
        double y0 = (pt.y > ptOrig.y ? ptOrig.y : pt.y) - 0.75 * unitSpacing;
        double w = roundf(fabsf(ptOrig.x - pt.x)) + 2.50 * unitSpacing;
        double h = roundf(fabsf(ptOrig.y - pt.y)) + 1.50 * unitSpacing;
        
        _tabInputCursorRect = RERect(x0, y0, w, h);
    }
    else if(_scoreController->InferredSelectionKind() == REScoreController::TickRangeSelection)
    {
        int tickA = originCursor.Tick();
        int tickB = cursor.Tick();
        if(ia > ib) {
            std::swap(ia,ib);
            std::swap(barA,barB);
            std::swap(tickA,tickB);
        }
        else if(ia == ib)
        {
            // Make sure barA <= barB
            if(barA > barB) {
                std::swap(barA,barB);
                std::swap(tickA,tickB);
            }
            else if(barA == barB)
            {
                // Make sure tickA <= tickB
                if(tickA > tickB) {
                    std::swap(tickA,tickB);
                }
            }
        }
        
        RERect reunion = RERect(0,0,0,0);
        _tabInputCursorSubRects.reserve(ib-ia+1);
        
        for(int systemIndex=ia; systemIndex<=ib; ++systemIndex)
        {
            const RESystem* system = score->System(systemIndex);
            RERect rc = system->SceneFrame();
            
            double x0 = rc.origin.x;
            double y0 = rc.origin.y;
            double x1 = x0 + rc.size.w;
            double y1 = y0 + rc.size.h;
            
            const RETrack* track = cursor.Track();
            if(track == NULL) break;
            const REStaff* firstStaff = cursor.Staff();
            const REStaff* lastStaff = cursor.Staff();
            if(firstStaff == NULL || lastStaff == NULL) break;
            y0 = rc.origin.y + firstStaff->YOffset() - firstStaff->TopSpacing();
            y1 = rc.origin.y + lastStaff->YOffset() + lastStaff->Height() + lastStaff->BottomSpacing();
            
            // First system within selection range
            if(systemIndex == ia) {
                const RESlice* sliceA = system->SystemBarWithBarIndex(barA);
                x0 = sliceA->ScenePosition().x + sliceA->XOffsetOfTick(tickA) - firstStaff->UnitSpacing();
            }
            
            // Last system within selection range
            if(systemIndex == ib) {
                const RESlice* sliceB = system->SystemBarWithBarIndex(barB);
                x1 = sliceB->ScenePosition().x + sliceB->XOffsetOfTick(tickB) + firstStaff->UnitSpacing();
            }
            
            rc = RERect(x0, y0, x1-x0, y1-y0);
            reunion = reunion.Union(rc);
            _tabInputCursorSubRects.push_back(rc);
        }
        _tabInputCursorRect = reunion;
    }
    else if(_scoreController->InferredSelectionKind() == REScoreController::BarRangeSelection)
    {
        if(ia > ib) {
            std::swap(ia,ib);
            std::swap(barA,barB);
        }
        else if(ia == ib)
        {
            if(barA > barB) {
                std::swap(barA,barB);
            }
        }
        
        RERect reunion = RERect(0,0,0,0);
        _tabInputCursorSubRects.reserve(ib-ia+1);
        
        for(int systemIndex=ia; systemIndex<=ib; ++systemIndex)
        {
            const RESystem* system = score->System(systemIndex);
            RERect rc = system->SceneFrame();
            
            double x0 = rc.origin.x;
            double y0 = rc.origin.y;
            double x1 = x0 + rc.size.w;
            double y1 = y0 + rc.size.h;
            
            const RETrack* track = cursor.Track();
            if(track == NULL) break;
            
            // First system within selection range
            if(systemIndex == ia) {
                const RESlice* sliceA = system->SystemBarWithBarIndex(barA);
                x0 = sliceA->ScenePosition().x;
            }
            
            // Last system within selection range
            if(systemIndex == ib) {
                const RESlice* sliceB = system->SystemBarWithBarIndex(barB);
                x1 = sliceB->ScenePosition().x + sliceB->Width();
            }
            
            rc = RERect(x0, y0, x1-x0, y1-y0);
            reunion = reunion.Union(rc);
            _tabInputCursorSubRects.push_back(rc);
        }
        _tabInputCursorRect = reunion;
    }
    else if(_scoreController->InferredSelectionKind() == REScoreController::SingleTrackBarRangeSelection)
    {
        if(ia > ib) {
            std::swap(ia,ib);
            std::swap(barA,barB);
        }
        else if(ia == ib)
        {
            if(barA > barB) {
                std::swap(barA,barB);
            }
        }
        
        RERect reunion = RERect(0,0,0,0);
        _tabInputCursorSubRects.reserve(ib-ia+1);
        
        for(int systemIndex=ia; systemIndex<=ib; ++systemIndex)
        {
            const RESystem* system = score->System(systemIndex);
            RERect rc = system->SceneFrame();
            
            double x0 = rc.origin.x;
            double y0 = rc.origin.y;
            double x1 = x0 + rc.size.w;
            double y1 = y0 + rc.size.h;
            
            const RETrack* track = cursor.Track();
            if(track == NULL) break;
            const REStaff* firstStaff = system->FirstStaffOfTrack(track);
            const REStaff* lastStaff = system->LastStaffOfTrack(track);
            if(firstStaff == NULL || lastStaff == NULL) break;
            y0 = rc.origin.y + firstStaff->YOffset() - firstStaff->TopSpacing();
            y1 = rc.origin.y + lastStaff->YOffset() + lastStaff->Height() + lastStaff->BottomSpacing();
            
            // First system within selection range
            if(systemIndex == ia) {
                const RESlice* sliceA = system->SystemBarWithBarIndex(barA);
                x0 = sliceA->ScenePosition().x;
            }
            
            // Last system within selection range
            if(systemIndex == ib) {
                const RESlice* sliceB = system->SystemBarWithBarIndex(barB);
                x1 = sliceB->ScenePosition().x + sliceB->Width();
            }
            
            rc = RERect(x0, y0, x1-x0, y1-y0);
            reunion = reunion.Union(rc);
            _tabInputCursorSubRects.push_back(rc);
        }
        _tabInputCursorRect = reunion;
    }
    
}

void REViewport::UpdateVisibleViews()
{
    REScore* score = Score();
    if(score == NULL) return;
    
    RERect visRect = ViewportVisibleRect();
    
    switch(score->LayoutType())
    {
        case Reflow::PageScoreLayout:
        {
            for(REPage* page : score->Root()->Pages()) {
                REViewportItem* item = page->ViewportItem();
                if(item) item->AttachToViewport(RERect::Intersects(page->SceneFrame(), visRect));
            }
            break;
        }
        case Reflow::HorizontalScoreLayout:
        {
            for(RESlice* slice : score->System(0)->Slices()) {
                REViewportItem* item = slice->ViewportItem();
                if(item) item->AttachToViewport(RERect::Intersects(slice->SceneFrame(), visRect));
            }
            break;
        }
    }
    
    // Update Manipulator visibility
    for(RETool* tool : _scoreController->Tools())
    {
        if(tool) tool->UpdateVisibleManipulators();
    }
}

void REViewport::Update()
{
    // Update playback cursor
    //UpdatePlaybackCursor();
    
    // Update tracker
    
}

// WARNING: THIS METHOD IS CALLED FROM RT THREAD
void REViewport::_UpdatePlaybackRT(const RESequencer* sequencer)
{
    _playbackRunningRT = sequencer->IsRunning();
    _barPlayingRT = sequencer->BarIndexThatsCurrentlyPlaying();
    _tickInBarPlayingRT = sequencer->TickInBarPlaying();
#ifdef REFLOW_OPENGL_VIEWPORT
    ++_currentUpdateRT;
#else
    _currentUpdateRT = TimeInSecondsSinceStart();
#endif
    
    if(sequencer->IsPlaybackFinished()) {
        OnPlaybackFinishedRT();
        _playbackRunning = false;
    }
}



void REViewport::UpdatePlaybackCursor(float dt)
{
    bool playbackStatusChanged = false;
    
    // Check if there was a change
    if(_currentUpdateRT != _currentUpdate)
    {
        // Update !
        _playbackRunning = _playbackRunningRT;
        _barPlaying = _barPlayingRT;
        _tickInBarPlaying = _tickInBarPlayingRT;
        _currentUpdate = _currentUpdateRT;
    }
    
    if(_playbackRunning)
    {
        _playbackCursorVisible = true;
        
        const REScore* score = _scoreController->Score();
        const RESystem* system = score->SystemWithBarIndex(_barPlaying);
        const RESlice* slice = (system ? system->SystemBarWithBarIndex(_barPlaying) : NULL);
        if(system != NULL && slice != NULL)
        {
            RERect systemRect = system->SceneFrame();
            systemRect.origin.x -= 40.0;
            systemRect.size.w += 80.0;
            systemRect.origin.y -= 40.0;
            systemRect.size.h += 80.0;
            RERect sliceRect = slice->SceneFrame();
            const REBarMetrics& metrics = slice->Metrics();
            
            const RESystem* nextSystem = score->System(1 + system->Index());
            RERect nextSystemRect = (nextSystem ? nextSystem->SceneFrame() : systemRect);
            
            float playbackIndicatorX = 0.0;
            unsigned long prevTick, nextTick;
            int columnIndex = metrics.ColumnIndexAtOrBeforeTick(_tickInBarPlaying, &prevTick, &nextTick);
            if(columnIndex == -1) {
            }
            else if(columnIndex == metrics.ColumnCount()-1)
            {
                nextTick = slice->Bar()->TheoricDurationInTicks();
                float t = (float)((long)_tickInBarPlaying - (long)prevTick) / (float)(nextTick-prevTick);
                float x0, x1;
                x0 = slice->XOffsetOfTick(prevTick);
                if(slice->IsLastInSystem()) {
                    x1 = slice->Width();
                }
                else {
                    const RESlice* sbarNext = slice->NextSibling();
                    x1 = sbarNext->XOffset() + sbarNext->XOffsetOfTick(0) - slice->XOffset();
                }
                playbackIndicatorX = x0 + t * (x1-x0);
            }
            else {
                float t = (float)((long)_tickInBarPlaying - (long)prevTick) / (float)(nextTick-prevTick);
                float x0 = slice->XOffsetOfTick(prevTick);
                float x1 = slice->XOffsetOfTick(nextTick);
                playbackIndicatorX = x0 + t * (x1-x0);
            }
            
            float x = sliceRect.origin.x + playbackIndicatorX;
            float y = sliceRect.origin.y;
            float h = sliceRect.size.h;
            
            
            _lastPlaybackCursorRect = _currentPlaybackCursorRect;
            
            _currentPlaybackCursorRect = RERect(x-6.0, y, 12, h);
            
            _currentSystemRect = systemRect;
            _nextSystemRect = nextSystemRect;
            
            
            
            // Now smoothen update playback
            SmoothUpdatePlayback(dt);
                        
            /*REPrintf("current system rect: {L:%1.3f T:%1.3f R:%1.3f B:%1.3f }\n", _currentSystemRect.Left(), _currentSystemRect.Top(), _currentSystemRect.Right(), _currentSystemRect.Bottom());
            REPrintf("next system rect   : {L:%1.3f T:%1.3f R:%1.3f B:%1.3f }\n", _nextSystemRect.Left(), _nextSystemRect.Top(), _nextSystemRect.Right(), _nextSystemRect.Bottom());
            REPrintf("union system rect  : {L:%1.3f T:%1.3f R:%1.3f B:%1.3f }\n", unionRect.Left(), unionRect.Top(), unionRect.Right(), unionRect.Bottom());
            REPrintf("viewport vis rect  : {L:%1.3f T:%1.3f R:%1.3f B:%1.3f }\n", viewportRect.Left(), viewportRect.Top(), viewportRect.Right(), viewportRect.Bottom());*/
            
            
            
            /*if(_playbackTrackingEnabled)
            {
                REPrintf("(%1.4f) Playback Cursor Rect: %1.2f %1.2f %1.2f %1.2f\n",
                         _currentUpdate, _playbackCursorRect.origin.x, _playbackCursorRect.origin.y, _playbackCursorRect.size.w, _playbackCursorRect.size.h);
            }
            else {
                REPrintf("(%1.4f) Tracking disabled\n", _currentUpdate);
            }*/
            _playbackCursorVisible = true;
        }
        
    }
    else {
        _playbackCursorVisible = false;
    }
    
    /*if(playbackStatusChanged)
    {
        if(_playbackRunning) {
            PlaybackStarted();
        }
        else {
            PlaybackStopped();
        }
    }*/
}

void REViewport::UpdatePlaybackCursorStatus(const RESequencer* sequencer)
{
    if(sequencer == nullptr) return;
    
    if(!sequencer->IsRunning()) {
        _playbackRunning = false;
        _playbackCursorVisible = false;
    }
}

void REViewport::SmoothUpdatePlayback(float dt)
{
    double x0 = _lastPlaybackCursorRect.origin.x;
    double x1 = _currentPlaybackCursorRect.origin.x;
    
    double x = x1; //(dt != 0 ? x0 + (x1-x0) * (((now - _lastPlaybackUpdate) - dt) / dt) : x1);
    
    double y = _currentPlaybackCursorRect.origin.y;
    double w = _currentPlaybackCursorRect.size.w;
    double h = _currentPlaybackCursorRect.size.h;
    
    _playbackCursorRect = RERect(x,y,w,h);
    
    RERect unionRect = _currentSystemRect.Union(_nextSystemRect);
    RERect viewportRect = ViewportVisibleRect();
    
    if(_playbackTrackingEnabled && _playbackTrackingPause <= 0.0)
    {
        //REPrintf("REViewport::UpdatePlaybackCursor dt = %1.4g (now: %1.4g)\n", dt, now);
        REPoint offset = unionRect.origin;
        _playbackTrackingSmoother.Update(dt);
        
        if(_playbackTrackingSmoother.dest != offset)
        {
            _playbackTrackingSmoother.Set(offset);
            //REPrintf("REViewport::UpdatePlaybackCursor Set(%1.4g %1.4g)\n", offset.x, offset.y);
        }
        
        REPoint contentOffset = _playbackTrackingSmoother.cur;
        
        float pcx = _playbackCursorRect.origin.x;
        float dx = pcx - offset.x;
        float scrollableDx = (_currentSystemRect.Width() - viewportRect.Width());
        if(scrollableDx > 0.0)
        {
            float tresholdDx = (0.30 * viewportRect.Width());
            if(dx > tresholdDx)
            {
                float addx = std::min<float>(dx - tresholdDx, scrollableDx);
                contentOffset.x += addx;
            }
//            REPrintf("dx: %g - scrollable dx: %g\n", dx, scrollableDx);
        }
        
        if(_scoreController->LayoutType() == Reflow::HorizontalScoreLayout) {
            contentOffset.y = viewportRect.origin.y;
        }
        SetViewportOffset(contentOffset);
        UpdateVisibleViews();
    }
    
    if(_playbackTrackingPause > 0.0)
    {
        _playbackTrackingSmoother.Reset(viewportRect.origin);
        _playbackTrackingPause = std::max<double>(0.0, _playbackTrackingPause - dt);
    }
}

void REViewportPageItem::CreateSystemItems()
{
    REScore* score = _viewport->Score();
    BOOST_FOREACH(RESystem* system, score->Systems())
    {
        if(system->Parent() != _page) continue;
        
        REViewportSystemItem* systemItem = CreateSystemItemInPage(system);
        system->SetViewportItem(systemItem);
    }
}
