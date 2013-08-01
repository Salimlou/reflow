//
//  REManipulator.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/04/13.
//
//

#include "REManipulator.h"
#include "REPainter.h"
#include "REBezierPath.h"
#include "REViewport.h"
#include "RESlur.h"
#include "REScoreController.h"
#include "REStaff.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REChord.h"
#include "REPhrase.h"
#include "RESong.h"
#include "REVoice.h"
#include "RETrack.h"
#include "REBarMetrics.h"

REHandle::REHandle(const std::string& identifier, REManipulator* manipulator)
: REScoreNode(), _identifier(identifier), _manipulator(manipulator), _draggable(true)
{
    _parent = _manipulator;
}



REManipulator::~REManipulator()
{
    for(auto h : _handles)
    {
        delete h.second;
    }
    _handles.clear();
}

REScoreController* REManipulator::ScoreController()
{
    return _tool->ScoreController();
}
const REScoreController* REManipulator::ScoreController() const
{
    return _tool->ScoreController();
}


REHandle* REManipulator::HandleAtPoint(const REPoint& pt) const
{
    for(auto it : _handles)
    {
        REHandle* h = it.second;
        RERect rc = RectFromSceneToLocal(h->SceneFrame());
        if(rc.PointInside(pt))
        {
            return h;
        }
    }
    return nullptr;
}

REHandle* REManipulator::DraggableHandleAtPoint(const REPoint& pt) const
{
    for(auto it : _handles)
    {
        REHandle* h = it.second;
        if(!h->IsDraggable()) continue;
        
        RERect rc = RectFromSceneToLocal(h->SceneFrame());
        if(rc.PointInside(pt))
        {
            return h;
        }
    }
    return nullptr;
}

bool REManipulator::IsDragging() const
{
    return _focusedHandle != nullptr;
}

bool REManipulator::MouseDown(const REPoint& pt, unsigned long flags)
{
    _dragged = false;
    REHandle* h = DraggableHandleAtPoint(pt);
    if(h)
    {
        _focusedHandle = h;
        _dragOrigin = pt;
        _focusedHandleOriginPoint = _focusedHandle->Position();
        if(ViewportItem()) {
            ViewportItem()->SetNeedsDisplay();
        }
        return true;
    }
    return false;
}

bool REManipulator::MouseDragged(const REPoint& pt, unsigned long flags)
{
    if(_focusedHandle)
    {
        REPrintf("mouseDragged @(%1.3f %1.3f) dragOrigin(%1.3f %1.3f)\n", pt.x, pt.y, _dragOrigin.x, _dragOrigin.y);
        
        REPoint delta = (pt - _dragOrigin);
        REPoint newPointLocal = _focusedHandleOriginPoint + delta;
        
        REPoint backupDragOriginInScene = PointFromLocalToScene(_dragOrigin);
        
        _focusedHandle->SetPosition(newPointLocal);
        RefreshBounds();
        
        REPoint dpos = PointFromSceneToLocal(backupDragOriginInScene) - _dragOrigin;
        _dragOrigin = PointFromSceneToLocal(backupDragOriginInScene);
        _focusedHandleOriginPoint = _focusedHandleOriginPoint + dpos;
        
        _dragged = true;
        
        return true;
    }
    return false;
}

bool REManipulator::MouseUp(const REPoint& pt, unsigned long flags)
{
    _focusedHandle = nullptr;
    if(ViewportItem()) {
        ViewportItem()->SetNeedsDisplay();
    }
    return true;
}

bool REManipulator::MouseDoubleClicked(const REPoint& pointInScene, unsigned long flags)
{
    if(IsEditable())
    {
        StartEditing();
        return true;
    }
    return false;
}

void REManipulator::RefreshBounds()
{
    RERect rc = ManipulatedItemBounds();
    
    if(_handles.empty() && rc.Width() == 0 && rc.Height() == 0) return;

    bool first = true;
    
    float minLeft = 0;
    float maxRight = 0;
    float minTop = 0;
    float maxBottom = 0;
    
    {
        RERect rc = ManipulatedItemBounds();
        if(rc.Width() != 0 && rc.Height() != 0)
        {
            minLeft = rc.Left();
            minTop = rc.Top();
            maxRight = rc.Right();
            maxBottom = rc.Bottom();
            first = false;
        }
    }
    
    for(auto it : _handles)
    {
        REHandle* h = it.second;
        RERect rc = RectFromSceneToLocal(h->SceneFrame());
        if(!first) {
            minLeft = std::min<float>(rc.Left(), minLeft);
            minTop = std::min<float>(rc.Top(), minTop);
            maxRight = std::max<float>(rc.Right(), maxRight);
            maxBottom = std::max<float>(rc.Bottom(), maxBottom);
        }
        else {
            first = false;
            minLeft = rc.Left();
            minTop = rc.Top();
            maxRight = rc.Right();
            maxBottom = rc.Bottom();
        }
    }
    SetBounds(RERect(minLeft, minTop, maxRight-minLeft, maxBottom-minTop));
    if(ViewportItem()) {
        static_cast<REViewportManipulatorItem*>(ViewportItem())->UpdateFrame();
    }
}



RESymbolManipulator::RESymbolManipulator(RETool* tool, const REStaff* staff, const REChord* chord, const RESymbol* psymbol, const REPoint& startPoint)
: REManipulator(tool), _staff(staff), _pchord(chord), _psymbol(psymbol), _startPoint(startPoint)
{
    CreateHandles(startPoint);
}

RERect RESymbolManipulator::ManipulatedItemBounds()
{
    REHandle* start = _handles.find("start")->second;
    REPoint p_start = PointFromSceneToLocal(start->ScenePosition());
    
    RERect bounds = _psymbol->Bounds(_staff->UnitSpacing());
    bounds.origin.x -= 2.0;
    bounds.origin.y -= 2.0;
    bounds.size.w += 4.0;
    bounds.size.h += 4.0;
    
    return bounds.Translated(p_start);
}

void RESymbolManipulator::Draw(REPainter& painter) const
{
    REHandle* start = _handles.find("start")->second;
    REHandle* start_anchor = _handles.find("start_anchor")->second;
    
    REPoint p_start = PointFromSceneToLocal(start->ScenePosition());
    REPoint p_start_anchor = PointFromSceneToLocal(start_anchor->ScenePosition());
    
    // Anchor Points
    {
        painter.SetStrokeColor(REColor::Red);
        REReal lengths[] = {1.0f, 1.0f};
        painter.SetLineDash(lengths, 2, 0.0);
        
        painter.StrokeLine(p_start_anchor, p_start);
    }
    
    // Gray borders
    /*{
        painter.SetStrokeColor(REColor::LightGray);
        REReal lengths[] = {2.0f, 1.0f};
        painter.SetLineDash(lengths, 2, 0.0);
        
        painter.StrokeRect(Bounds());
                
        painter.SetLineDash(NULL, 0, 0.0f);
        painter.SetStrokeColor(REColor::DarkGray);
    }*/
    
    // Draw Handles
    {
        REHandle* handle = start;
        RERect rc = handle->SceneFrame();
        
        rc = RectFromSceneToLocal(rc);
        rc.origin.x += 0.5;
        rc.origin.y += 0.5;
        rc.size.w -= 1.0;
        rc.size.h -= 1.0;
        painter.StrokeRect(rc);
    }
    
    _psymbol->Draw(painter, p_start, REColor::Black, _staff->UnitSpacing());
}

void RESymbolManipulator::CreateHandles(const REPoint& startPoint)
{
    float hsize = 6.0f;
    RERect handleBounds = RERect(-hsize/2, -hsize/2, hsize, hsize);
    
    RERect bounds = _psymbol->Bounds(_staff->UnitSpacing());
    bounds.origin.x -= 2.0;
    bounds.origin.y -= 2.0;
    bounds.size.w += 4.0;
    bounds.size.h += 4.0;
    
    SetPosition(startPoint);
    
    // Start anchor point
    REHandle* startAnchorHandle = new REHandle("start_anchor", this);
    _handles[startAnchorHandle->Identifier()] = startAnchorHandle;
    startAnchorHandle->SetPosition(REPoint(0,0));
    startAnchorHandle->SetBounds(handleBounds);
    
    // start point
    REHandle* startHandle = new REHandle("start", this);
    _handles[startHandle->Identifier()] = startHandle;
    startHandle->SetPosition(_psymbol->Offset());
    startHandle->SetBounds(bounds);
    
    RefreshBounds();
}

bool RESymbolManipulator::MouseDragged(const REPoint& pt, unsigned long flags)
{
    if(_focusedHandle)
    {
        //REPrintf("mouseDragged @(%1.3f %1.3f) dragOrigin(%1.3f %1.3f)\n", pt.x, pt.y, _dragOrigin.x, _dragOrigin.y);
        
        if(_focusedHandle->Identifier() == "start")
        {
            REPoint p = _focusedHandle->ScenePosition();
            const RESystem* system = _staff->System();
            REPoint pointInSystem = system->PointFromSceneToLocal(p);
            float xInSlice = 0.0;
            const RESlice* slice = system->SystemBarAtX(pointInSystem.x, &xInSlice);
            if(slice)
            {
                int columnIndex = 0;
                float snapX = xInSlice;
                auto result = slice->QueryColumnAtX(xInSlice, &columnIndex, &snapX);
                if(result == RESlice::OnColumn || result == RESlice::OnLeftEdgeOfColumn || result == RESlice::OnRightEdgeOfColumn)
                {
                    int barIndex = slice->BarIndex();
                    unsigned long tick = slice->Metrics().TickOfColumn(columnIndex);
                    
                    float deltaX = slice->PointFromLocalToScene(REPoint(snapX,0)).x - ScenePosition().x;
                    
                    REHandle* anchor = _handles["start_anchor"];
                    anchor->SetPosition(REPoint(deltaX, 0.0f));
                    
                    _newStartBeat = REGlobalTimeDiv(barIndex, RETimeDiv(tick, REFLOW_PULSES_PER_QUARTER));
                    _newStartBeatDeltaX = deltaX;
                    
                    REPrintf(" # snapX: %1.3f (new anchor: %d:%d) deltaX: %1.3f\n", snapX, barIndex, (int)tick, deltaX);
                }
            }
        }
        
        REPoint delta = (pt - _dragOrigin);
        REPoint newPointLocal = _focusedHandleOriginPoint + delta;
        
        REPoint backupDragOriginInScene = PointFromLocalToScene(_dragOrigin);
        
        _focusedHandle->SetPosition(newPointLocal);
        RefreshBounds();
        
        REPoint dpos = PointFromSceneToLocal(backupDragOriginInScene) - _dragOrigin;
        _dragOrigin = PointFromSceneToLocal(backupDragOriginInScene);
        _focusedHandleOriginPoint = _focusedHandleOriginPoint + dpos;
        
        _dragged = true;
        
        return true;
    }
    return false;
}

bool RESymbolManipulator::MouseUp(const REPoint& pt, unsigned long flags)
{
    if(_focusedHandle && _focusedHandle->Identifier() == "start" && _dragged)
    {
        REPoint offset = _focusedHandle->Position();
        REManipulator::MouseUp(pt, flags);
        
        RELocator startLocator = _pchord->Locator();
        REGlobalTimeDiv startBeat = REGlobalTimeDiv(_pchord->Phrase()->Index(), _pchord->Offset());
        if(startBeat != _newStartBeat)
        {
            RELocator destLocator(startLocator.Song(), _newStartBeat.bar, _staff->Track()->Index(), _pchord->Phrase()->Voice()->Index());
            const REPhrase* destPhrase = destLocator.Phrase();
            if(destPhrase)
            {
                const REChord* destChord = destPhrase->ChordAtTimeDiv(_newStartBeat.timeDiv);
                if(destChord)
                {
                    ScoreController()->PerformTaskOnSong([&](RESong* song)
                    {
                        // Remove symbol from source
                        bool copy = (0 != (flags & REScoreController::CursorAltDown));
                        REChord* chord = song->ChordAtLocator(startLocator);
                        int idx = chord->IndexOfSymbol(_psymbol);
                        RESymbol* symbol = (copy ? chord->Symbol(idx) : chord->TakeSymbolAtIndex(idx));
                        
                        // Add symbol to dest
                        if(symbol) {
                            if(copy) symbol = symbol->Clone();
                            song->ChordAtLocator(destChord->Locator())->AddSymbol(symbol);
                            symbol->SetOffset(offset - REPoint(_newStartBeatDeltaX, 0.0f));
                        }
                    });
                }
            }
        }
        else {
            ScoreController()->PerformTaskOnSong([&](RESong* song)
            {
                // Remove symbol from source
                REChord* chord = song->ChordAtLocator(startLocator);
                RESymbol* symbol = chord->Symbol(chord->IndexOfSymbol(_psymbol));
                symbol->SetOffset(offset - REPoint(_newStartBeatDeltaX, 0.0f));
            });
        }
        
        
        
        
        /*if(_newStartBeat != _psymbol->StartBeat())
        {
            _scoreController->SetSlurStartBeat(_trackIndex, _staffIndex, _slurIndex, _newStartBeat, offset - REPoint(_newStartBeatDeltaX, 0.0f));
        }
        else
        {
            _scoreController->SetSlurStartOffset(_trackIndex, _staffIndex, _slurIndex, offset);
        }*/
    }
    else
    {
        REManipulator::MouseUp(pt, flags);
    }
    
    return true;
}

bool RESymbolManipulator::IsEditable() const
{
    return false;
}
void RESymbolManipulator::StartEditing()
{
    if(ViewportItem()) {
        _editing = true;
        static_cast<REViewportManipulatorItem*>(ViewportItem())->StartEditing();
    }
}
void RESymbolManipulator::FinishedEditing()
{
    _editing = false;
    if(ViewportItem()) {
        static_cast<REViewportManipulatorItem*>(ViewportItem())->FinishedEditing();
    }
}

#pragma mark -
#pragma mark RETextSymbolManipulator

RETextSymbolManipulator::RETextSymbolManipulator(RETool* tool, const REStaff* staff, const REChord* chord, const RESymbol* psymbol, const REPoint& startPoint)
: RESymbolManipulator(tool, staff, chord, psymbol, startPoint)
{
}

bool RETextSymbolManipulator::IsEditable() const
{
    return true;
}



#pragma mark -
#pragma mark RESlurManipulator

RESlurManipulator::RESlurManipulator(RETool* tool, const REStaff* staff, int trackIndex, int slurIndex, const RESlur* pslur, const REPoint& startPoint, const REPoint& endPoint)
: REManipulator(tool), _staff(staff), _trackIndex(trackIndex), _slurIndex(slurIndex), _pslur(pslur), _startPoint(startPoint), _endPoint(endPoint)
{
    _newStartBeat = pslur->StartBeat();
    _newEndBeat = pslur->EndBeat();
    CreateHandles(startPoint, endPoint);
}

void RESlurManipulator::Draw(REPainter& painter) const
{
    REHandle* start = _handles.find("start")->second;
    REHandle* start_cp = _handles.find("start_cp")->second;
    REHandle* end_cp = _handles.find("end_cp")->second;
    REHandle* end = _handles.find("end")->second;
    REHandle* start_anchor = _handles.find("start_anchor")->second;
    REHandle* end_anchor = _handles.find("end_anchor")->second;
    
    REPoint p_start = PointFromSceneToLocal(start->ScenePosition());
    REPoint p_start_cp = PointFromSceneToLocal(start_cp->ScenePosition());
    REPoint p_end_cp = PointFromSceneToLocal(end_cp->ScenePosition());
    REPoint p_end = PointFromSceneToLocal(end->ScenePosition());
    REPoint p_start_anchor = PointFromSceneToLocal(start_anchor->ScenePosition());
    REPoint p_end_anchor = PointFromSceneToLocal(end_anchor->ScenePosition());
    
    // Anchor Points
    if(_focusedHandle)
    {
        painter.SetStrokeColor(REColor::Red);
        REReal lengths[] = {1.0f, 1.0f};
        painter.SetLineDash(lengths, 2, 0.0);

        painter.StrokeLine(p_start_anchor, p_start);
        painter.StrokeLine(p_end_anchor, p_end);
    }
    
    // Gray borders
    {
        painter.SetStrokeColor(REColor::LightGray);
        REReal lengths[] = {2.0f, 1.0f};
        painter.SetLineDash(lengths, 2, 0.0);
        
        REBezierPath p;
        p.MoveToPoint(p_start);
        p.LineToPoint(p_start_cp);
        p.LineToPoint(p_end_cp);
        p.LineToPoint(p_end);
        p.LineToPoint(p_start);
        painter.StrokePath(p);
        
        painter.SetLineDash(NULL, 0, 0.0f);
        painter.SetStrokeColor(REColor::DarkGray);
    }
    
    // Draw Handles
    for(auto h : _handles)
    {
        REHandle* handle = h.second;
        if(!handle->IsDraggable()) continue;
        
        RERect rc = handle->SceneFrame();
        
        rc = RectFromSceneToLocal(rc);
        rc.origin.x += 0.5;
        rc.origin.y += 0.5;
        rc.size.w -= 1.0;
        rc.size.h -= 1.0;
        painter.StrokeRect(rc);
    }
    
    // Draw Slur
    {
        REPoint deltaThickness = REPoint(0, 5.0f);
        
        REBezierPath slurPath;
        slurPath.MoveToPoint(p_start);
        slurPath.CurveToPoint(p_end, p_start_cp, p_end_cp);
        slurPath.CurveToPoint(p_start, p_end_cp + deltaThickness, p_start_cp + deltaThickness);
        slurPath.Close();
        
        if(_focusedHandle) {
            painter.SetFillColor(REColor(0.20, 0.25, 0.85));
        }
        else {
            painter.SetFillColor(REColor::Black);
        }
        painter.FillPath(slurPath);
    }
}

void RESlurManipulator::CreateHandles(const REPoint& startPoint, const REPoint& endPoint)
{
    float hsize = 6.0f;
    RERect handleBounds = RERect(-hsize/2, -hsize/2, hsize, hsize);
    
    SetPosition(startPoint);
    REPoint deltaEnd = (endPoint - startPoint);
    
    // Start anchor point
    REHandle* startAnchorHandle = new REHandle("start_anchor", this);
    _handles[startAnchorHandle->Identifier()] = startAnchorHandle;
    startAnchorHandle->SetPosition(REPoint(0,0));
    startAnchorHandle->SetBounds(handleBounds);
    startAnchorHandle->SetDraggable(false);
    
    // End anchor point
    REHandle* endAnchorHandle = new REHandle("end_anchor", this);
    _handles[endAnchorHandle->Identifier()] = endAnchorHandle;
    endAnchorHandle->SetPosition(deltaEnd);
    endAnchorHandle->SetBounds(handleBounds);
    endAnchorHandle->SetDraggable(false);
    
    // start point
    REHandle* startHandle = new REHandle("start", this);
    _handles[startHandle->Identifier()] = startHandle;
    startHandle->SetPosition(_pslur->StartOffset());
    startHandle->SetBounds(handleBounds);
    
    // end point
    REHandle* endHandle = new REHandle("end", this);
    _handles[endHandle->Identifier()] = endHandle;
    endHandle->SetPosition(_pslur->EndOffset() + deltaEnd);
    endHandle->SetBounds(handleBounds);
    
    // start control point
    REHandle* scp = new REHandle("start_cp", this);
    scp->SetParent(startHandle);
    _handles[scp->Identifier()] = scp;
    scp->SetPosition(_pslur->StartControlPointOffset());
    scp->SetBounds(handleBounds);
    
    // end control point
    REHandle* ecp = new REHandle("end_cp", this);
    ecp->SetParent(endHandle);
    _handles[ecp->Identifier()] = ecp;
    ecp->SetPosition(_pslur->EndControlPointOffset());
    ecp->SetBounds(handleBounds);
    
    RefreshBounds();
}

bool RESlurManipulator::MouseDragged(const REPoint& pt, unsigned long flags)
{
    if(_focusedHandle)
    {
        //REPrintf("mouseDragged @(%1.3f %1.3f) dragOrigin(%1.3f %1.3f)\n", pt.x, pt.y, _dragOrigin.x, _dragOrigin.y);
        
        if(_focusedHandle->Identifier() == "start" || _focusedHandle->Identifier() == "end")
        {
            bool start = (_focusedHandle->Identifier() == "start");
            
            REPoint p = _focusedHandle->ScenePosition();
            const RESystem* system = _staff->System();
            REPoint pointInSystem = system->PointFromSceneToLocal(p);
            float xInSlice = 0.0;
            const RESlice* slice = system->SystemBarAtX(pointInSystem.x, &xInSlice);
            if(slice)
            {
                int columnIndex = 0;
                float snapX = xInSlice;
                auto result = slice->QueryColumnAtX(xInSlice, &columnIndex, &snapX);
                if(result == RESlice::OnColumn || result == RESlice::OnLeftEdgeOfColumn || result == RESlice::OnRightEdgeOfColumn)
                {
                    int barIndex = slice->BarIndex();
                    unsigned long tick = slice->Metrics().TickOfColumn(columnIndex);
                    
                    float deltaX = 0;
                    if(start)
                    {
                        deltaX = slice->PointFromLocalToScene(REPoint(snapX,0)).x - ScenePosition().x;
                        
                        REHandle* anchor = _handles["start_anchor"];
                        anchor->SetPosition(REPoint(deltaX, 0.0f));
                        
                        _newStartBeat = REGlobalTimeDiv(barIndex, RETimeDiv(tick, REFLOW_PULSES_PER_QUARTER));
                        _newStartBeatDeltaX = deltaX;
                    }
                    else {
                        REPoint deltaEnd = _endPoint - _startPoint;
                        deltaX = (slice->PointFromLocalToScene(REPoint(snapX,0)) - ScenePosition() - deltaEnd).x;
                        
                        REHandle* anchor = _handles["end_anchor"];
                        anchor->SetPosition(REPoint(deltaX + deltaEnd.x, 0.0f));
                        
                        _newEndBeat = REGlobalTimeDiv(barIndex, RETimeDiv(tick, REFLOW_PULSES_PER_QUARTER));
                        _newEndBeatDeltaX = deltaX;
                    }
                    
                    REPrintf(" # snapX: %1.3f (new anchor: %d:%d) deltaX: %1.3f\n", snapX, barIndex, (int)tick, deltaX);
                }
            }
        }
        
        REPoint delta = (pt - _dragOrigin);
        REPoint newPointLocal = _focusedHandleOriginPoint + delta;
        
        REPoint backupDragOriginInScene = PointFromLocalToScene(_dragOrigin);
        
        _focusedHandle->SetPosition(newPointLocal);
        RefreshBounds();
        
        REPoint dpos = PointFromSceneToLocal(backupDragOriginInScene) - _dragOrigin;
        _dragOrigin = PointFromSceneToLocal(backupDragOriginInScene);
        _focusedHandleOriginPoint = _focusedHandleOriginPoint + dpos;
        
        _dragged = true;
        
        return true;
    }
    return false;
}

bool RESlurManipulator::MouseUp(const REPoint& pt, unsigned long flags)
{
    if(_focusedHandle && _focusedHandle->Identifier() == "start_cp" && _dragged)
    {
        REPoint offset = _focusedHandle->Position();
        REManipulator::MouseUp(pt, flags);
        
        ScoreController()->SetSlurStartControlPointOffset(_trackIndex, _staff->Identifier(), _slurIndex, offset);
    }
    else if(_focusedHandle && _focusedHandle->Identifier() == "end_cp" && _dragged)
    {
        REPoint offset = _focusedHandle->Position();
        REManipulator::MouseUp(pt, flags);
        
        ScoreController()->SetSlurEndControlPointOffset(_trackIndex, _staff->Identifier(), _slurIndex, offset);
    }
    else if(_focusedHandle && _focusedHandle->Identifier() == "start" && _dragged)
    {
        REPoint offset = _focusedHandle->Position();
        REManipulator::MouseUp(pt, flags);
        
        if(_newStartBeat != _pslur->StartBeat())
        {
            ScoreController()->SetSlurStartBeat(_trackIndex, _staff->Identifier(), _slurIndex, _newStartBeat, offset - REPoint(_newStartBeatDeltaX, 0.0f));
        }
        else
        {
            ScoreController()->SetSlurStartOffset(_trackIndex, _staff->Identifier(), _slurIndex, offset);
        }
    }
    else if(_focusedHandle && _focusedHandle->Identifier() == "end" && _dragged)
    {
        REPoint delta = (pt - _dragOrigin);
        
        REPoint offset = _pslur->EndOffset() + delta;
        REManipulator::MouseUp(pt, flags);
        
        if(_newEndBeat != _pslur->EndBeat())
        {
            ScoreController()->SetSlurEndBeat(_trackIndex, _staff->Identifier(), _slurIndex, _newEndBeat, offset - REPoint(_newEndBeatDeltaX, 0.0f));
        }
        else
        {
            ScoreController()->SetSlurEndOffset(_trackIndex, _staff->Identifier(), _slurIndex, offset);
        }
    }
    else
    {
        REManipulator::MouseUp(pt, flags);
    }
    
    return true;
}

#pragma mark -
void RENoteToolHoverManipulator::Draw(REPainter& painter) const
{
    painter.SetStrokeColor(REColor::Green);
    painter.StrokeRect(RERect(-3.5, -3.5, 8.0, 8.0));
    
    painter.DrawMusicSymbol("quarter", 0, 0, 7.0);
    
}

RERect RENoteToolHoverManipulator::ManipulatedItemBounds()
{
    RERect bounds = RERect(-5, -5, 10, 10);
    bounds.origin.x -= 2.0;
    bounds.origin.y -= 2.0;
    bounds.size.w += 4.0;
    bounds.size.h += 4.0;
    
    return bounds;
}

