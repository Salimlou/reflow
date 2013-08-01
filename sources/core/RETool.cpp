//
//  RETool.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 24/04/13.
//
//

#include "RETool.h"
#include "REFunctions.h"
#include "REScoreController.h"
#include "REViewport.h"
#include "REScore.h"
#include "RESystem.h"
#include "REStaff.h"
#include "RESlice.h"
#include "RETrack.h"
#include "REBarMetrics.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "RENote.h"

#include <sstream>

const char* RETool::Name() const
{
    return Reflow::NameOfTool(Type());
}

void RETool::DestroyManipulators()
{
    _destroyingManipulators = true;
    REViewport* viewport = _scoreController->Viewport();
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        REViewportManipulatorItem* manipulatorItem = static_cast<REViewportManipulatorItem*>(manipulator->ViewportItem());
        if(manipulatorItem)
        {
            viewport->DestroyManipulatorItem(manipulatorItem);
            manipulator->SetViewportItem(nullptr);
        }
        
        delete manipulator;
    }
    _manipulators.clear();
    _destroyingManipulators = false;
}

void RETool::UpdateVisibleManipulators()
{
    REViewport* viewport = _scoreController->Viewport();
    RERect visRect = viewport->ViewportVisibleRect();
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        REViewportItem* item = manipulator->ViewportItem();
        if(item && !manipulator->IsEditing()) item->AttachToViewport(RERect::Intersects(manipulator->SceneFrame(), visRect));
    }
}

bool RETool::MouseDown(const REPoint& pointInScene, unsigned long flags)
{
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->SceneFrame().PointInside(pointInScene))
        {
            if(manipulator->MouseDown(manipulator->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }
    return false;
}
bool RETool::MouseUp(const REPoint& pointInScene, unsigned long flags)
{
    // Is any manipulator dragging ?
    for(auto m : _manipulators) {
        if(m.second->IsDragging()) {
            if(m.second->MouseUp(m.second->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }

    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->SceneFrame().PointInside(pointInScene))
        {
            if(manipulator->MouseUp(manipulator->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }
    return false;
}
bool RETool::MouseDragged(const REPoint& pointInScene, unsigned long flags)
{
    // Is any manipulator dragging ?
    for(auto m : _manipulators) {
        if(m.second->IsDragging()) {
            if(m.second->MouseDragged(m.second->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }
    
    // Drag other manipulators
    /*for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->SceneFrame().PointInside(pointInScene))
        {
            if(manipulator->MouseDragged(manipulator->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }*/
    return false;
}
bool RETool::MouseMoved(const REPoint& pointInScene, unsigned long flags)
{
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->SceneFrame().PointInside(pointInScene))
        {
            /*if(manipulator->MouseMoved(manipulator->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }*/
        }
    }
    return false;
}

bool RETool::MouseDoubleClicked(const REPoint& pointInScene, unsigned long flags)
{
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->SceneFrame().PointInside(pointInScene))
        {
            if(manipulator->MouseDoubleClicked(manipulator->PointFromSceneToLocal(pointInScene), flags)) {
                return true;
            }
        }
    }
    return false;
}


#pragma mark -
#pragma mark RENoteTool
bool RENoteTool::MouseMoved(const REPoint& pointInScene, unsigned long flags)
{
    const REScore* score = _scoreController->Score();
    REViewport* viewport = _scoreController->Viewport();
    if(!score || !viewport) return false;
    
    REPoint startPoint = pointInScene;
    const RESystem* system = score->PickSystem(pointInScene);
    if(system)
    {
        REPoint pointInSystem = system->PointFromSceneToLocal(pointInScene);
        int lineIndex = 0;
        const REStaff* staff = system->StaffAtY(pointInSystem.y, &lineIndex);
        if(staff)
        {
            float y = staff->YOffset() + staff->YOffsetOfLine(lineIndex);
            float xInSlice = 0.0;
            const RESlice* slice = system->SystemBarAtX(pointInSystem.x, &xInSlice);
            if(slice)
            {
                int columnIndex = 0;
                float snapX = xInSlice;
                auto result = slice->QueryColumnAtX(xInSlice, &columnIndex, &snapX);
                if(result & RESlice::OnColumn)
                {
                    if(result & RESlice::OnLeftEdgeOfColumn) {
                        startPoint = slice->PointFromLocalToScene(REPoint(xInSlice, y));
                    }
                    else if(result & RESlice::OnRightEdgeOfColumn) {
                        startPoint = slice->PointFromLocalToScene(REPoint(xInSlice, y));
                    }
                    else {
                        startPoint = slice->PointFromLocalToScene(REPoint(snapX, y));
                    }
                }
                else if(result == RESlice::InLeadingSpace) {
                    
                }
                else if(result == RESlice::InTrailingSpace) {
                    
                }
            }
        }
    }
    
    auto it = _manipulators.find("RENoteToolHoverManipulator");
    RENoteToolHoverManipulator* hoverManipulator = nullptr;
    if(it != _manipulators.end())
    {
        hoverManipulator = static_cast<RENoteToolHoverManipulator*>(it->second);
    }
    else {
        hoverManipulator = new RENoteToolHoverManipulator(this);
        hoverManipulator->SetPosition(startPoint);
        _manipulators["RENoteToolHoverManipulator"] = hoverManipulator;
        REViewportManipulatorItem* item = viewport->CreateManipulatorItem(hoverManipulator);
        hoverManipulator->SetViewportItem(item);
    }
    
    hoverManipulator->SetPosition(startPoint);
    hoverManipulator->RefreshBounds();
    UpdateVisibleManipulators();
    return true;
}

bool RENoteTool::MouseUp(const REPoint& pointInScene, unsigned long flags)
{
    const REScore* score = _scoreController->Score();
    REViewport* viewport = _scoreController->Viewport();
    if(!score || !viewport) return false;
    
    REPoint startPoint = pointInScene;
    const RESystem* system = score->PickSystem(pointInScene);
    if(system)
    {
        REPoint pointInSystem = system->PointFromSceneToLocal(pointInScene);
        int lineIndex = 0;
        const REStaff* staff = system->StaffAtY(pointInSystem.y, &lineIndex);
        if(staff)
        {
            float y = staff->YOffset() + staff->YOffsetOfLine(lineIndex);
            float xInSlice = 0.0;
            const RESlice* slice = system->SystemBarAtX(pointInSystem.x, &xInSlice);
            if(slice)
            {
                int columnIndex = 0;
                float snapX = xInSlice;
                auto result = slice->QueryColumnAtX(xInSlice, &columnIndex, &snapX);
                if(result & RESlice::OnColumn)
                {
                    if(result & RESlice::OnLeftEdgeOfColumn) {
                        
                    }
                    else if(result & RESlice::OnRightEdgeOfColumn) {
                        
                    }
                    else {
                        // Add Note to Chord
                        int barIndex = slice->BarIndex();
                        int voiceIndex = _scoreController->Cursor().VoiceIndex();
                        unsigned long tick = slice->Metrics().TickOfColumn(columnIndex);
                        
                        const RETrack* track = staff->Track();
                        const REVoice* voice = track ? track->Voice(voiceIndex) : nullptr;
                        const REPhrase* phrase = voice ? voice->Phrase(barIndex) : nullptr;
                        
                        _scoreController->MoveCursorTo(staff->Index(), barIndex, tick, lineIndex, 0);
                        _scoreController->TypeKeypadNumber(0);
                    }
                }
                else if(result == RESlice::InLeadingSpace) {
                    
                }
                else if(result == RESlice::InTrailingSpace) {
                    
                }
            }
        }
    }
    return true;
}

void RENoteTool::CreateManipulators()
{
    
}

#pragma mark -
#pragma mark RESymbolTool

bool RESymbolTool::MouseDown(const REPoint& pointInScene, unsigned long flags)
{
    const REScore* score = _scoreController->Score();
    REViewport* viewport = _scoreController->Viewport();
    if(!score || !viewport) return false;
    
    if(RETool::MouseDown(pointInScene, flags))
    {
        return true;
    }
    
    // Is any manipulator being edited ?
    for(auto m : _manipulators)
    {
        REManipulator* manipulator = m.second;
        if(manipulator->IsEditing()) {
            manipulator->FinishedEditing();
            return true;
        }
    }
    
    bool selectionChanged = false;
    const RESymbol* symbolUnderMouse = nullptr;
    const REStaff* staffUnderMouse = nullptr;
    
    for(const RESystem* system : score->Systems())
    {
        system->IterateSymbols([&](const REConstSymbolAnchor& anchor) {
            if(anchor.symbol->IsSelected()) {
                selectionChanged = true;
                anchor.symbol->SetSelected(false);
            }
        });
        
        system->IterateSymbolsAtPoint(system->PointFromSceneToLocal(pointInScene), [&](const REConstSymbolAnchor& anchor) {
            selectionChanged = true;
            symbolUnderMouse = anchor.symbol;
            staffUnderMouse = anchor.staff;
            anchor.symbol->SetSelected(true);
        });
    }
    
    if(selectionChanged)
    {
        // Create manipulators for new selection
        DestroyManipulators();
        CreateManipulators();
        viewport->UpdateVisibleViews();
        
        if(symbolUnderMouse)
        {
            for(auto m : _manipulators)
            {
                REManipulator* manipulator = m.second;
                if(manipulator->ManipulatorType() == Reflow::SymbolManipulator &&
                   static_cast<RESymbolManipulator*>(manipulator)->Symbol() == symbolUnderMouse &&
                   static_cast<RESymbolManipulator*>(manipulator)->Staff() == staffUnderMouse)
                {
                    manipulator->MouseDown(manipulator->PointFromSceneToLocal(pointInScene), flags);
                    break;
                }
            }
        }
    }
    else
    {
        // Create a new symbol
    }
    
    return true;
}
bool RESymbolTool::MouseUp(const REPoint& pointInScene, unsigned long flags)
{
    return RETool::MouseUp(pointInScene, flags);
}
bool RESymbolTool::MouseDragged(const REPoint& pointInScene, unsigned long flags)
{
    return RETool::MouseDragged(pointInScene, flags);
}
bool RESymbolTool::MouseMoved(const REPoint& pointInScene, unsigned long flags)
{
    return RETool::MouseMoved(pointInScene, flags);
}

void RESymbolTool::CreateManipulators()
{
    REScore* score = _scoreController->Score();
    REViewport* viewport = _scoreController->Viewport();
    if(!score || !viewport) return;
    
    int slurId = 1;
    for(RESystem* system : score->Systems())
    {
        system->IterateSymbols([&](const REConstSymbolAnchor& anchor)
        {
            if(anchor.symbol->IsSelected())
            {
                REPoint startPoint = system->ScenePosition() + anchor.origin;
                RESymbolManipulator* sm = CreateManipulatorForSymbol(anchor, startPoint);
                
                std::ostringstream oss;
                oss << "symbol-" << (slurId++);
                _manipulators[oss.str()] = sm;
                REViewportManipulatorItem* item = viewport->CreateManipulatorItem(sm);
                sm->SetViewportItem(item);
                
                sm->RefreshBounds();
            }
        });
    }
}

RESymbolManipulator* RESymbolTool::CreateManipulatorForSymbol(const REConstSymbolAnchor& anchor, const REPoint& startPoint)
{
    RESymbolManipulator* sm = new RESymbolManipulator(this, anchor.staff, anchor.locator.Chord(), anchor.symbol, startPoint);
    return sm;
}

#pragma mark -
#pragma mark RETextTool

RESymbolManipulator* RETextTool::CreateManipulatorForSymbol(const REConstSymbolAnchor& anchor, const REPoint& startPoint)
{
    RETextSymbolManipulator* sm = new RETextSymbolManipulator(this, anchor.staff, anchor.locator.Chord(), anchor.symbol, startPoint);
    return sm;
}

#pragma mark -
#pragma mark RESlurTool

void RESlurTool::CreateManipulators()
{
    REScore* score = _scoreController->Score();
    REViewport* viewport = _scoreController->Viewport();
    if(!score || !viewport) return;
    
    int slurId = 1;
    for(RESystem* system : score->Systems())
    {
        for(int i=0; i<system->StaffCount(); ++i)
        {
            const REStaff* staff = system->Staff(i);
            const REConstSlurPtrVector& slurs = staff->Slurs();
            
            for(const RESlur* pslur : slurs)
            {
                int firstBarIndex = pslur->StartBeat().bar;
                int lastBarIndex = pslur->EndBeat().bar;
                int startTick = Reflow::TimeDivToTicks(pslur->StartBeat().timeDiv);
                int endTick = Reflow::TimeDivToTicks(pslur->EndBeat().timeDiv);
                
                if(_scoreController->InferredSelectionKind() == REScoreController::CursorSelection &&
                   staff == _scoreController->Cursor().Staff() &&
                   pslur->StartBeat() <= _scoreController->Cursor().Beat() &&
                   _scoreController->Cursor().Beat() <= pslur->EndBeat())
                {
                
                if(system->BarRange().IsInRange(firstBarIndex) && system->BarRange().IsInRange(lastBarIndex))
                {
                    const RESlice* startSlice = system->SystemBarWithBarIndex(firstBarIndex);
                    const RESlice* endSlice = system->SystemBarWithBarIndex(lastBarIndex);
                    
                    float x0 = startSlice->XOffset() + startSlice->XOffsetOfTick(startTick);
                    float x1 = endSlice->XOffset() + endSlice->XOffsetOfTick(endTick);
                    
                    REPoint startPoint = system->ScenePosition() + REPoint(x0, staff->YOffset());
                    REPoint endPoint = system->ScenePosition() + REPoint(x1, staff->YOffset());
                    int slurIndex = staff->Track()->IndexOfSlur(pslur, staff->Identifier());
                    RESlurManipulator* sm = new RESlurManipulator(this, staff, staff->Track()->Index(), slurIndex, pslur, startPoint, endPoint);
                    
                    std::ostringstream oss;
                    oss << "slur-" << (slurId++);
                    _manipulators[oss.str()] = sm;
                    REViewportManipulatorItem* item = viewport->CreateManipulatorItem(sm);
                    sm->SetViewportItem(item);
                    
                    sm->RefreshBounds();
                }
                }
            }
            
        }
    }
}

#pragma mark -
void RETablatureTool::CreateManipulators()
{
    _scoreController->SlurTool().CreateManipulators();
}


