//
//  RESystem.cpp
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESystem.h"
#include "REScore.h"
#include "REScoreRoot.h"
#include "RESong.h"
#include "REBar.h"
#include "RESlice.h"
#include "REStaff.h"
#include "REBarMetrics.h"
#include "REPainter.h"
#include "REStandardStaff.h"
#include "REFunctions.h"
#include "RETrack.h"
#include "REPage.h"
#include "REStyle.h"

#include <sstream>
#include <cmath>

#include <boost/format.hpp>

RESystem::RESystem()
: _indexInPage(0), _flags(0), _score(NULL)
{
    _leftMargin = 35.0;
    _rehearsalYOffset = 0.0;
    _chordNameYOffset = 0.0;
    _horizontalSystem = false;
}
RESystem::~RESystem()
{
    Clear();
}

void RESystem::Clear()
{
    for(unsigned int i=0; i<SystemBarCount(); ++i) {
        delete _systemBars[i];
    }
    for(unsigned int i=0; i<StaffCount(); ++i) {
        delete _staves[i];
    }
    _systemBars.clear();
    _staves.clear();
}

void RESystem::_UpdateIndices()
{
    for(unsigned int i=0; i<SystemBarCount(); ++i) {
        _systemBars[i]->_index = i;
    }
    for(unsigned int i=0; i<StaffCount(); ++i) {
        _staves[i]->_index = i;
    }
}

const RESlice* RESystem::SystemBar(int idx) const
{
    if(idx >= 0 && idx < (unsigned int)_systemBars.size()) {
        return _systemBars[idx];
    }
    return 0;
}

RESlice* RESystem::SystemBar(int idx) {
    if(idx >= 0 && idx < (unsigned int)_systemBars.size()) {
        return _systemBars[idx];
    }
    return 0;    
}

const REStaff* RESystem::Staff(int idx) const 
{
    if(idx >= 0 && idx < (unsigned int)_staves.size()) {
        return _staves[idx];
    }
    return 0;
}
REStaff* RESystem::Staff(int idx)
{
    if(idx >= 0 && idx < (unsigned int)_staves.size()) {
        return _staves[idx];
    }
    return 0;
}

void RESystem::InsertSystemBar(RESlice* systemBar, int index)
{
    _systemBars.insert(_systemBars.begin() + index, systemBar);
    systemBar->_parent = this;
    _UpdateIndices();
}

void RESystem::InsertStaff(REStaff *staff, int index)
{
    _staves.insert(_staves.begin() + index, staff);
    staff->_parent = this;
    _UpdateIndices();
}

bool RESystem::HasStandardStaffForTrack(const RETrack* track) const
{
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        if(staff->Track() == track && staff->Type() == Reflow::StandardStaff) {
            return true;
        }
    }
    return false;
}

bool RESystem::HasTablatureStaffForTrack(const RETrack* track) const
{
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        if(staff->Track() == track && staff->Type() == Reflow::TablatureStaff) {
            return true;
        }
    }
    return false;    
}

const REStaff* RESystem::FirstStaffOfTrack(const RETrack* track) const
{
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        if(staff->Track() == track) {
            return staff;
        }
    }
    return NULL;
}

const REStaff* RESystem::LastStaffOfTrack(const RETrack* track) const
{
    for(int i=_staves.size()-1; i >= 0; --i)
    {
        const REStaff* staff = Staff(i);
        if(staff->Track() == track) {
            return staff;
        }
    }
    return NULL;
}

float RESystem::ContentWidth() const
{
    return Width() - _leftMargin;
}

const RESlice* RESystem::SystemBarWithBarIndex(int barIndex) const
{
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {   
        const RESlice* systemBar = SystemBar(i);
        if(systemBar->BarIndex() <= barIndex && barIndex <= systemBar->LastBarIndex()) {
            return systemBar;
        }
    }
    return NULL;
}

RESlice* RESystem::SliceWithBarIndex(int barIndex)
{
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {   
        RESlice* systemBar = SystemBar(i);
        if(systemBar->BarIndex() <= barIndex && barIndex <= systemBar->LastBarIndex()) {
            return systemBar;
        }
    }
    return NULL;
}

void RESystem::_RefreshMetrics()
{
    _flags = 0;
    
    const REScore* score = Score();
    const REScoreSettings& settings = score->Settings();
    const REStyle* style = settings.Style();
    if(style == nullptr) style = REStyle::DefaultReflowStyle();
    
    bool hideTempo = settings.HasFlag(REScoreSettings::HideTempo);
    bool hideRehearsal = settings.HasFlag(REScoreSettings::HideRehearsal);
    
    // System Flags
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {   
        RESlice* systemBar = SystemBar(i);
        const REBar* bar = systemBar->Bar();
        
        if(bar->HasFlag(REBar::RehearsalSign) && !hideRehearsal) {SetFlag(RESystem::RehearsalBand);}
        if(bar->ChordNameCount()) {SetFlag(RESystem::ChordNameBand);}
        if(bar->HasFlag(REBar::RepeatEnd) && bar->RepeatCount() > 2) {SetFlag(RESystem::RepeatCountBand);}
        if(bar->HasAnyAlternateEnding()) {SetFlag(RESystem::AlternateEndingsBand);}
        if(bar->HasDaCapoOrDalSegnoOrDalSegnoSegno()) {SetFlag(RESystem::JumpBand);}
        if(bar->HasDirectionJump(Reflow::ToCoda)) {SetFlag(RESystem::ToCodaBand);}
        if(bar->HasDirectionJump(Reflow::ToDoubleCoda)) {SetFlag(RESystem::ToDoubleCodaBand);}
        if(bar->HasDirectionTarget(Reflow::Fine)) {SetFlag(RESystem::FineBand);}
        if(bar->HasDirectionTarget(Reflow::Coda)) {SetFlag(RESystem::CodaBand);}
        if(bar->HasDirectionTarget(Reflow::DoubleCoda)) {SetFlag(RESystem::DoubleCodaBand);}
        if(bar->HasDirectionTarget(Reflow::Segno)) {SetFlag(RESystem::SegnoBand);}
        if(bar->HasDirectionTarget(Reflow::SegnoSegno)) {SetFlag(RESystem::SegnoSegnoBand);}
    }
    
    // Tempo change
    if(!hideTempo)
    {
        const RESong* song = score->Song();
        bool hasTempoChange = _barRange.FirstIndex() == 0 || song->TempoTimeline().HasItemInBarRange(_barRange.FirstIndex(), _barRange.LastIndex());
        if(hasTempoChange) {SetFlag(RESystem::TempoMarkerBand);}
    }
    
    _rehearsalYOffset = 0;
    _chordNameYOffset = 0;
    _codaYOffset = 0;
    _doubleCodaYOffset = 0;
    _segnoYOffset = 0;
    _segnoSegnoYOffset = 0;
    _jumpYOffset = 0;
    _toCodaYOffset = 0;
    _toDoubleCodaYOffset = 0;
    _fineYOffset = 0;
    _repeatCountYOffset = 0;
    _alternateEndingsYOffset = 0;
    _tempoMarkerYOffset = 0;
    
    float yOffset = 0.0f;
    
    // Calculate Spacing for Bands Above the Staves
    // -------------------------------------------------
    if(HasFlag(RESystem::TempoMarkerBand)) {
        _tempoMarkerYOffset = yOffset;
        yOffset += 16.0;
    }
    
    if(HasFlag(RESystem::CodaBand)) {
        _codaYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::DoubleCodaBand)) {
        _doubleCodaYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::SegnoBand)) {
        _segnoYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::SegnoSegnoBand)) {
        _segnoSegnoYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    
    if(HasFlag(RESystem::RehearsalBand)) {
        _rehearsalYOffset = yOffset;
        yOffset += 20.0;
    }
    
    if(HasFlag(RESystem::AlternateEndingsBand)) {
        _alternateEndingsYOffset = yOffset;
        yOffset += 12.0;
    }
    if(HasFlag(RESystem::RepeatCountBand)) {
        _repeatCountYOffset = yOffset;
        yOffset += 12.0;
    }
    
    // Chord Names ?
    if(HasFlag(RESystem::ChordNameBand)) {
        _chordNameYOffset = yOffset;
        yOffset += (style->ChordNameFontSize() + 1.0);
    }

    // Calculate Spacing of the Staves
    // -------------------------------------------------
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        REStaff* staff = Staff(i);
        staff->_RefreshMetrics();
        
        yOffset += staff->TopSpacing();
        staff->_yOffset = yOffset;
        yOffset += staff->Height() + staff->BottomSpacing();
    }
    
    // Calculate Spacing for Bands Below the Staves
    // -------------------------------------------------
    if(HasFlag(RESystem::JumpBand)) {
        _jumpYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::ToCodaBand)) {
        _toCodaYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::ToDoubleCodaBand)) {
        _toDoubleCodaYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    if(HasFlag(RESystem::FineBand)) {
        _fineYOffset = yOffset;
        yOffset += DirectionSymbolHeight();
    }
    
    SetHeight(yOffset);
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {
        RESlice* slice = SystemBar(i);
        slice->SetHeight(yOffset);
    }
}


void RESystem::CalculateBarDimensions()
{
    bool roundBarLinePosition = false;
    
    float stretchFactor = 1.0;
    if(!IsHorizontalSystem())
    {
        float totalIdealWidth = 0.0f;
        for(unsigned int i=0; i<_systemBars.size(); ++i)
        {
            unsigned long flags = 0;
            if(i == 0) flags |= REFLOW_BAR_METRICS_FIRST;
            if(i == SystemBarCount()-1) flags |= REFLOW_BAR_METRICS_LAST;
            
            RESlice* systemBar = SystemBar(i);
            totalIdealWidth += systemBar->Metrics().IdealWidth(flags);
        }
        
        float contentWidth = ContentWidth();
        stretchFactor = contentWidth / totalIdealWidth;
        if(stretchFactor > 1.30f && (IsLastSystemInScore()/* || HasSystemBreakOnLastBar()*/)) {
            stretchFactor = 1.0f;
        }
    }
        
    float xOffset = _leftMargin;
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {
        unsigned long flags = 0;
        if(i == 0) flags |= REFLOW_BAR_METRICS_FIRST;
        if(i == SystemBarCount()-1) flags |= REFLOW_BAR_METRICS_LAST;
        
        RESlice* slice = SystemBar(i);
        slice->SetPosition(REPoint(xOffset, 0));
        slice->SetWidth(stretchFactor * slice->Metrics().IdealWidth(flags));
        if(roundBarLinePosition) {
            slice->SetWidth(roundf(slice->Width()));
        }
        xOffset += slice->Width();
    }

    SetWidth(xOffset);
}

const RESystem* RESystem::PreviousSibling() const
{
    const REScore* score = Score();
    return score ? score->System(Index()-1) : NULL;
}

bool RESystem::HasSystemBreakOnLastBar() const 
{
    const REScore* score = Score();
    return score ? score->Settings().HasSystemBreakAtBarIndex(BarRange().LastIndex()) : false;
}

bool RESystem::IsLastSystemInScore() const
{
    const REScore* score = Score();
    if(score == NULL) return false;
    
    return BarRange().LastIndex() == (score->Song()->BarCount() - 1);
}

bool RESystem::IsFirstSystemInPage() const
{
    const RESystem* prev = PreviousSibling();
    if(prev == NULL) return true;
    return prev->PageNumber() != PageNumber();
}

unsigned int RESystem::PageNumber() const
{
    if(_parent == NULL || _parent->NodeType() != Reflow::PageNode) return 0;
    return static_cast<REPage*>(_parent)->Number();
}

const REStaff* RESystem::StaffAtY(float y, int* lineIndex) const
{
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        float dy;
        if(staff->IsYOffsetInside(y, &dy)) {
            int line = staff->LineAtYOffset(dy);
            if(lineIndex != NULL) *lineIndex = line;
            return staff;
        }
    }
    return NULL;
}

const RESlice* RESystem::SystemBarAtX(float x, float* relativeX) const
{
    for(unsigned int i=0; i<_systemBars.size(); ++i)
    {
        const RESlice* systemBar = SystemBar(i);
        float dx = x - systemBar->XOffset();
        if(dx >= 0.0 && dx < systemBar->Width()) {
            *relativeX = dx;
            return systemBar;
        }
    }
    return NULL;
}

unsigned int RESystem::ListGizmos(Reflow::GizmoType type, REGizmoVector& gizmos) const
{
    
    switch(type)
    {
        case Reflow::FirstBarlineGizmo: 
        {
            const REStaff* firstStaff = Staff(0);
            const REStaff* lastStaff = Staff(StaffCount()-1);
            float y0 = firstStaff->YOffset();
            float y1 = lastStaff->YOffset() + lastStaff->Height();
            float x0 = _leftMargin;
            {
                REGizmo gizmo;
                gizmo.type = Reflow::FirstBarlineGizmo;
                gizmo.barIndex = _barRange.index;
                gizmo.box = RERect(x0-2.0, y0-2.0, 4.0, (y1-y0)+4.0);
                gizmos.push_back(gizmo);
            }
            return gizmos.size();
        }
            
        case Reflow::MiddleBarlineGizmo: 
        {
            
            const REStaff* firstStaff = Staff(0);
            const REStaff* lastStaff = Staff(StaffCount()-1);
            float y0 = firstStaff->YOffset();
            float y1 = lastStaff->YOffset() + lastStaff->Height();
            
            float x0 = _leftMargin;
            for(unsigned int i=1; i<_systemBars.size(); ++i)
            {
                const RESlice* sbar = _systemBars[i];
                float x = sbar->XOffset();
                
                REGizmo gizmo;
                gizmo.type = Reflow::MiddleBarlineGizmo;
                gizmo.barIndex = _barRange.index + i;
                gizmo.box = RERect(x-2.0, y0-2.0, 4.0, (y1-y0)+4.0);
                gizmos.push_back(gizmo);
            }
            return gizmos.size();
        }
            
        default: {
            break;
        }
    }
    return 0;
}

void RESystem::PickNotesInRect(const RERect& rect, RENoteSet* noteSet, REIntSet* affectedBarsSet) const
{
    REPrintf("PickNotesInRect %1.2f %1.2f %1.2f %1.2f\n", rect.origin.x, rect.origin.y, rect.size.w, rect.size.h);
    
    REGizmoVector gizmos;
    for(unsigned int i=0; i<_staves.size(); ++i) {
        const REStaff* staff = Staff(i);
        staff->ListNoteGizmos(gizmos);
    }
    
    REGizmoVector::const_iterator it = gizmos.begin();
    for(; it != gizmos.end(); ++it)
    {
        const REGizmo& gizmo = *it;
        if(rect.PointInside(gizmo.box.Center()))
        {
            if(affectedBarsSet) affectedBarsSet->insert(gizmo.barIndex);
            RENote* note = (RENote*)gizmo.object;
            if(noteSet) noteSet->insert(note);
        }
    }
}

void RESystem::IterateSymbols(const REConstSymbolAnchorOperation& op) const
{
    for(const REStaff* staff : _staves)
    {
        for(const RESlice* slice : _systemBars)
        {
            int sliceIndex = slice->Index();
            staff->IterateSymbolsOfSlice(sliceIndex, [&](const REConstSymbolAnchor& anchor)
            {
                REPoint origin = anchor.origin + REPoint(slice->XOffset(), staff->YOffset());
				REConstSymbolAnchor csa = {anchor.symbol, staff, origin, anchor.locator};
                op(csa);
            });
        }
    }
}


void RESystem::IterateSymbolsAtPoint(const REPoint& pointInSystem, const REConstSymbolAnchorOperation& op) const
{
    for(const REStaff* staff : _staves)
    {
        for(const RESlice* slice : _systemBars)
        {
            int sliceIndex = slice->Index();
            staff->IterateSymbolsOfSlice(sliceIndex, [&](const REConstSymbolAnchor& anchor)
            {
                RERect symbolFrameInSlice = anchor.symbol->Frame(staff->UnitSpacing()).Translated(anchor.origin);
                RERect symbolFrameInStaff = symbolFrameInSlice.Translated(slice->XOffset(), 0.0f);
                RERect symbolFrameInSystem = symbolFrameInStaff.Translated(0.0f, staff->YOffset());
                if(symbolFrameInSystem.PointInside(pointInSystem))
                {
                    REPoint origin = anchor.origin + REPoint(slice->XOffset(), staff->YOffset());
					REConstSymbolAnchor csa = {anchor.symbol, staff, origin, anchor.locator};
					op(csa);
                }
            });
        }
    }
}

REPoint RESystem::OffsetInPage() const
{
    return REPoint(Score()->Settings().VirtualMargin(Reflow::LeftMargin) + Origin().x, 
                   Score()->Settings().VirtualMargin(Reflow::TopMargin) + Origin().y);
}

void RESystem::DrawTempoMarker(REPainter& painter, int tempo, Reflow::TempoUnitType tempoUnitType, const REPoint& pt) const
{
    float fontSize = 11.0;
    const char* fontName = "Arial";
    RERect bbox = painter.BoundingBoxOfMusicSymbol("quarternote", 12.0);
    
    // Quarter note symbol
    painter.DrawMusicSymbol("quarternote", pt, 12.0);
    float dx = 0.0;
    if(tempoUnitType == Reflow::QuarterDottedTempoUnit) {
        painter.DrawMusicSymbol("dot", pt.x + bbox.Width() + 3.0, pt.y + 11.5, 5.0);
        dx += 3.5;
    }
    
    // Text metrics
    std::ostringstream oss;
    oss << " = " << tempo;
    painter.DrawText(oss.str(), REPoint(dx + pt.x + bbox.Width(), pt.y + 0.5), fontName, REPainter::Bold, fontSize, REColor(0,0,0));
}

void RESystem::DrawSlice(REPainter& painter, int sliceIndex) const
{
    const RESlice* systemBar = SystemBar(sliceIndex);
    const REBar* bar = systemBar->Bar();
    if(bar == NULL) return;
    
    const REScore* score = Score();
    const RESong* song = score->Song();
    const REStyle* style = score->Settings().Style();
    if(style == nullptr) style = REStyle::DefaultReflowStyle();
    
    // Draw Staves
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        
        // Save & Translate
        painter.Save();
        painter.Translate(0.0, staff->YOffset());
        
        // Draw Staff
        staff->DrawSlice(painter, sliceIndex);
        
        // Restore
        painter.Restore();
    }
    
    // Draw Barlines of Slices
    painter.SetStrokeColor(REColor(0,0,0));    
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        
        // Save & Translate
        painter.Save();
        painter.Translate(0.0, staff->YOffset());
        
        // Draw Staff
        staff->DrawBarlinesOfSlice(painter, sliceIndex);
        
        // Restore
        painter.Restore();
    }
    
    painter.SetFillColor(REColor::Black);
    
    // Tempo Marker
    if(HasFlag(RESystem::TempoMarkerBand))
    {
        int firstIdx = 0;
        int lastIdx = 0;
        const RETempoTimeline& tempoMarkers = song->TempoTimeline();
        
        // Default Tempo
        if(bar->Index() == 0 && tempoMarkers.ItemAt(0,0) == NULL)
        {
            int defaultTempo = song->DefaultTempo();
            REPoint pt = REPoint(0.5, _tempoMarkerYOffset);
            
            DrawTempoMarker(painter, defaultTempo, Reflow::QuarterTempoUnit, pt);
        }
        
        // Other Tempo Markers
        if(tempoMarkers.FindItemsInBarRange(bar->Index(), bar->Index(), &firstIdx, &lastIdx))
        {
            for(int idx = firstIdx; idx <= lastIdx; ++idx)
            {
                const RETempoItem* item = tempoMarkers.Item(idx);
                int tick = Reflow::TimeDivToTicks(item->beat);
                float x = (tick == 0 ? 0 : systemBar->XOffsetOfTick(tick));
                REPoint pt = REPoint(x+0.5, _tempoMarkerYOffset);
                
                DrawTempoMarker(painter, item->tempo, item->unitType, pt);
            }
        }
    }
    
    // Draw Rehearsal Signs
    if(bar->HasFlag(REBar::RehearsalSign) && !score->Settings().HasFlag(REScoreSettings::HideRehearsal))
    {
        const std::string& txt = bar->RehearsalSignText();
        float boxMargin = 3.0;
        float fontSize = 16.0;
        const char* fontName = "Arial";
        REPoint pt = REPoint(0.5, roundf(_rehearsalYOffset) + 0.5);
        
        // Text metrics
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Bold, fontSize);
        
        // Frame
        painter.StrokeRect(RERect(pt.x, pt.y, sz.w + 2.0 * boxMargin, sz.h + boxMargin), 1.5);
        
        // Rehearsal Text
        painter.DrawText(txt, REPoint(pt.x + boxMargin, pt.y + 0.5*boxMargin), fontName, REPainter::Bold, fontSize, REColor(0,0,0));
    }
    
    // Draw Chord Names
    style->DrawChordNamesOfSlice(painter, systemBar, _chordNameYOffset);
    
    // D.C., D.S. and D.S.S
    if(bar->HasDaCapoOrDalSegnoOrDalSegnoSegno())
    {
        Reflow::DirectionJump jump = bar->FindDaCapoOrDalSegnoOrDalSegnoSegno();
        std::string txt = Reflow::NameOfDirectionJump(jump);
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        REPoint pt = REPoint(x + 0.5, _jumpYOffset + 0.5);
        
        // Text metrics
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Italic, fontSize);
        pt.x -= sz.w;
        painter.DrawText(txt, REPoint(pt.x, pt.y + 0.5), fontName, REPainter::Italic, fontSize, REColor(0,0,0));
    }
    
    // Fine
    if(bar->HasDirectionTarget(Reflow::Fine))
    {
        std::string txt = "Fine";
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        REPoint pt = REPoint(x + 0.5, _fineYOffset + 0.5);
        
        // Text metrics
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Italic, fontSize);
        pt.x -= sz.w;
        painter.DrawText(txt, REPoint(pt.x, pt.y + 0.5), fontName, REPainter::Italic, fontSize, REColor(0,0,0));
    }
    
    // Coda
    if(bar->HasDirectionTarget(Reflow::Coda)) 
    {
        RERect bbox = painter.BoundingBoxOfMusicSymbol("coda", 7.0);
        painter.DrawMusicSymbol("coda", 0, bbox.HalfHeight() + _codaYOffset, 7.0);
    }
    
    // Segno
    if(bar->HasDirectionTarget(Reflow::Segno)) 
    {
        RERect bbox = painter.BoundingBoxOfMusicSymbol("segno", 7.0);
        painter.DrawMusicSymbol("segno", 0, bbox.HalfHeight() + _segnoYOffset, 7.0);
    }
    
    // Double Coda
    if(bar->HasDirectionTarget(Reflow::DoubleCoda)) 
    {
        RERect bbox = painter.BoundingBoxOfMusicSymbol("coda", 7.0);
        painter.DrawMusicSymbol("coda", 0, bbox.HalfHeight() + _doubleCodaYOffset, 7.0);
        painter.DrawMusicSymbol("coda", bbox.Width() + 1.0, bbox.HalfHeight() + _doubleCodaYOffset, 7.0);
    }
    
    // Double Segno
    if(bar->HasDirectionTarget(Reflow::SegnoSegno)) 
    {
        RERect bbox = painter.BoundingBoxOfMusicSymbol("segno", 7.0);
        painter.DrawMusicSymbol("segno", 0, bbox.HalfHeight() + _segnoSegnoYOffset, 7.0);
        painter.DrawMusicSymbol("segno", bbox.Width() + 1.0, bbox.HalfHeight() + _segnoSegnoYOffset, 7.0);
    }
    
    // To Coda
    if(bar->HasDirectionJump(Reflow::ToCoda)) 
    {
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        
        // Coda
        RERect bbox = painter.BoundingBoxOfMusicSymbol("coda", 7.0);
        x -= bbox.Width()/2;
        painter.DrawMusicSymbol("coda", x, bbox.HalfHeight() + _toCodaYOffset, 7.0);
        x -= bbox.Width()/2;
        
        // Text metrics
        std::string txt = "To ";
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Italic, fontSize);
        x -= sz.w;
        painter.DrawText(txt, REPoint(x, _toCodaYOffset + bbox.HalfHeight() - sz.h/2), fontName, REPainter::Italic, fontSize, REColor(0,0,0));
    }
    
    // To Double Coda
    if(bar->HasDirectionJump(Reflow::ToDoubleCoda)) 
    {
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        
        // Coda
        RERect bbox = painter.BoundingBoxOfMusicSymbol("coda", 7.0);
        x -= bbox.Width()/2;
        painter.DrawMusicSymbol("coda", x, bbox.HalfHeight() + _toDoubleCodaYOffset, 7.0);
        x -= (bbox.Width() + 1);
        painter.DrawMusicSymbol("coda", x, bbox.HalfHeight() + _toDoubleCodaYOffset, 7.0);
        x -= bbox.Width()/2;
        
        // Text metrics
        std::string txt = "To ";
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Italic, fontSize);
        x -= sz.w;
        painter.DrawText(txt, REPoint(x, _toDoubleCodaYOffset + bbox.HalfHeight() - sz.h/2), fontName, REPainter::Italic, fontSize, REColor(0,0,0));
    }
    
    // Alternate Endings
    if(bar->HasAnyAlternateEnding())
    {
        double x0 = 0.5;
        double x1 = roundf(systemBar->Width())-0.5;
        double y0 = roundf(_alternateEndingsYOffset) + 0.5;
        double y1 = y0 + 6.0;
        
        painter.StrokeHorizontalLine(x0, x1, y0);
        painter.StrokeVerticalLine(x0, y0, y1);
        std::ostringstream oss;
        bool first = true;
        for(int i=0; i<8; ++i) {
            if(bar->HasAlternateEnding(i)) {
                if(!first) {oss << ", ";}
                oss << (i+1) << ".";
                first = false;
            }
        }
        painter.DrawText(oss.str(), REPoint(x0 + 2.0, y0+2.0), "Times New Roman", 0, 9.0, REColor(0,0,0));
    }
    
    // Repeat Count
    if(bar->HasFlag(REBar::RepeatEnd) && bar->RepeatCount() > 2) 
    {
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        
        // Text metrics
        std::string txt = boost::str(boost::format("%1% x") % (bar->RepeatCount()));
        RESize sz = painter.SizeOfText(txt, fontName, REPainter::Bold, fontSize);
        x -= (sz.w + 1);
        painter.DrawText(txt, REPoint(x, _repeatCountYOffset), fontName, REPainter::Bold, fontSize, REColor(0,0,0));
    }
    
    // System Break
    if(painter.IsDrawingToScreen() && score->Settings().HasSystemBreakAtBarIndex(bar->Index()))
    {
        const char pilcrowUTF8[3] = {static_cast<char>(0xC2), static_cast<char>(0xB6), 0x00};
        std::string txt = pilcrowUTF8;
        float x = systemBar->Width();
        
        float fontSize = 12.0;
        const char* fontName = "Times New Roman";
        
        // Text metrics
        RESize sz = painter.SizeOfText(txt, fontName, 0, fontSize);
        x -= (sz.w + 1);
        painter.DrawText(txt, REPoint(x, 0.0), fontName, 0, fontSize, REColor(0,0,0));
    }
    
    if(IsHorizontalSystem() && StaffCount() != 0)
    {
        const REStaff* firstStaff = Staff(0);
        const REStaff* lastStaff = Staff(StaffCount()-1);
        float y0 = firstStaff->YOffset();
        float y1 = lastStaff->YOffset() + lastStaff->Height();

        painter.SetFillColor(REColor::Gray);
        
        // System bar range
        std::ostringstream oss;
        oss << systemBar->BarIndex() + 1 ;
        painter.DrawText(oss.str(), REPoint(0.0, y0 - 20.0), "Arial", REPainter::Bold, 10.0, REColor::Gray);
        
        painter.SetFillColor(REColor::Gray);
    }
}

void RESystem::Draw(REPainter &painter) const
{
    const REScore* score = Score();
    const RESong* song = score->Song();
    const REScoreSettings& settings = score->Settings();
    const REStyle* style = settings.Style();
        
    // Draw Slices
    for(unsigned int systemBarIndex=0; systemBarIndex < SystemBarCount(); ++systemBarIndex)
    {
        const RESlice* systemBar = SystemBar(systemBarIndex);
        const REBar* bar = systemBar->Bar();
        
        painter.Save();
        painter.Translate(systemBar->XOffset(), 0.0);
        
        DrawSlice(painter, systemBarIndex);
        
        painter.Restore();
    }
    
    // Draw Bands
    if(!IsHorizontalSystem())
    {
        RERange sliceRange(0, SystemBarCount());
        DrawBandsOfSliceRange(painter, sliceRange);
    }
    
    if(StaffCount() != 0)
    {
        const REStaff* firstStaff = Staff(0);
        const REStaff* lastStaff = Staff(StaffCount()-1);
        float y0 = firstStaff->YOffset();
        float y1 = lastStaff->YOffset() + lastStaff->Height();
        
        // Big bar line
        painter.SetStrokeColor(REColor(0,0,0));
        painter.StrokeVerticalLine(_leftMargin+0.5, y0, y1);
        
        // System bar range
        std::ostringstream oss;
        oss << _barRange.index + 1 ;
        painter.DrawText(oss.str(), REPoint(_leftMargin-3.0,y0 - 16.0), "Arial", REPainter::Bold, 10.0, REColor::DarkGray);
        
        // Draw Track Name in header
        if(IsFirstSystemInPage())
        {
            for(const RETrack* track : score->Tracks())
            {
                const REStaff* staff = FirstStaffOfTrack(track);
                if(staff == NULL) continue;
                
                float y = roundf(staff->YOffset() + staff->Height()/2 - 6.0);
                REPoint pt(0.5, y+0.5);
                
                std::string name = track->ShortName();//(Index() == 0 ? track->Name() : track->ShortName());
                painter.DrawText(name, pt, "Times New Roman", REPainter::Bold, 11.0, REColor::Black);
            }
        }
        
        // Brackets
        painter.SetFillColor(REColor::Black);
        for(const RETrack* track : score->Tracks())
        {
            const REStaff* firstStaff = FirstStaffOfTrack(track);
            const REStaff* lastStaff = LastStaffOfTrack(track);
            if(firstStaff && lastStaff && firstStaff != lastStaff) 
            {
                float x0 = _leftMargin - 6.0 + 0.5;
                float x1 = x0 + 3.0;
                float y0 = roundf(firstStaff->YOffset()) + 0.5;
                float y1 = roundf(lastStaff->YOffset() + lastStaff->Height()) + 0.5;
                
                REPoint  o(x0, (y0+y1)/2);
                REPoint p0(x0, y0);
                REPoint p1(x1 + 5.0, y0 - 5.0);
                REPoint p2(x1, y0);
                REPoint p3(p2.x, o.y + (o.y-p2.y));
                REPoint p4(p1.x, o.y + (o.y-p1.y));
                REPoint p5(p0.x, o.y + (o.y-p0.y));
                
                painter.PathBegin();
                {
                    painter.PathMoveToPoint(o);
                    
                    painter.PathLineToPoint(p0);
                    painter.PathLineToPoint(p1);
                    painter.PathLineToPoint(p2);
                    
                    painter.PathLineToPoint(p3);
                    painter.PathLineToPoint(p4);
                    painter.PathLineToPoint(p5);
                    
                    painter.PathLineToPoint(o);
                }
                painter.PathClose();
                painter.PathFill();
            }
        }
        
        // Slurs
        for(const REStaff* staff : _staves)
        {
            const REConstSlurPtrVector& slurs = staff->Slurs();
            if(slurs.empty()) continue;
            
            painter.Save();
            painter.Translate(0.0f, staff->YOffset());
            
            for(const RESlur* slur : staff->Slurs())
            {
                staff->DrawSlur(painter, *slur);
            }
            
            painter.Restore();
        }
    }
    
    // Style post draw
    if(style) style->PostDrawSystem(painter, *this);
    
//#define REFLOW_DRAW_GIZMOS
#ifdef REFLOW_DRAW_GIZMOS
    // Draw Gizmos
    REGizmoVector noteGizmos;
    for(unsigned int i=0; i<_staves.size(); ++i) {
        const REStaff* staff = Staff(i);
        staff->ListNoteGizmos(noteGizmos);
    }
    painter.SetFillColor(REColor(0, 0, 1, 0.25));
    for(const REGizmo& gizmo : noteGizmos) {
        painter.FillRect(gizmo.box);
    }
#endif
}

void RESystem::DrawSlursOfSlice(REPainter& painter, int sliceIndex) const
{
    const RESlice* slice = SystemBar(sliceIndex);
    if(!slice) return;
    
    int barIndex = slice->BarIndex();
    
    
    // Slurs
    for(const REStaff* staff : _staves)
    {
        const REConstSlurPtrVector& slurs = staff->Slurs();
        if(slurs.empty()) continue;
        
        painter.Save();
        painter.Translate(0, staff->YOffset());
        
        for(const RESlur* slur : staff->Slurs())
        {
            if(slur->BarRange().IsInRange(barIndex))
            {
                staff->DrawSlur(painter, *slur);
            }
        }
        
        painter.Restore();
    }
}

void RESystem::DrawBandsOfSliceRange(REPainter& painter, const RERange& sliceRange) const
{
    for(unsigned int i=0; i<_staves.size(); ++i)
    {
        const REStaff* staff = Staff(i);
        
        // Vibrato
        if(staff->HasFlag(REStaff::VibratoBand)) {
            staff->DrawVibratoBand(painter, staff->YOffset() + staff->YOffsetOfVibrato(), sliceRange);
        }
        
        // Chord Diagrams
        if(staff->HasFlag(REStaff::ChordDiagramBand)){
            staff->DrawChordDiagramBand(painter, staff->YOffset() + staff->YOffsetOfChordDiagram(), sliceRange);
        }
        
        // Dynamics
        if(staff->HasFlag(REStaff::DynamicsBand)) {
            staff->DrawDynamicsBand(painter, staff->YOffset() + staff->YOffsetOfDynamics(), sliceRange);
        }
        
        if(staff->Type() == Reflow::StandardStaff) 
        {
            //const REStandardStaff* stdStaff = static_cast<const REStandardStaff*>(staff);
            
            // Ottavia
            if(staff->HasFlag(REStaff::OttaviaBand)) {
                staff->DrawOttaviaBand(painter, Reflow::Ottavia_8va,  staff->YOffset() + staff->YOffsetOfOttavia(), sliceRange);
                staff->DrawOttaviaBand(painter, Reflow::Ottavia_8vb,  staff->YOffset() + staff->YOffsetOfOttavia(), sliceRange);
                staff->DrawOttaviaBand(painter, Reflow::Ottavia_15ma, staff->YOffset() + staff->YOffsetOfOttavia(), sliceRange);
                staff->DrawOttaviaBand(painter, Reflow::Ottavia_15mb, staff->YOffset() + staff->YOffsetOfOttavia(), sliceRange);
            }
            
            // Text Above
            if(staff->HasFlag(REStaff::TextAboveStaffBand)) {
                staff->DrawTextBand(painter, Reflow::TextAboveStandardStaff, staff->YOffset() + staff->YOffsetOfTextPositionedAbove(), sliceRange);
            }
            
            // Text Below
            if(staff->HasFlag(REStaff::TextBelowStaffBand)) {
                staff->DrawTextBand(painter, Reflow::TextBelowStandardStaff, staff->YOffset() + staff->YOffsetOfTextPositionedBelow(), sliceRange);
            }
        }
        else if(staff->Type() == Reflow::TablatureStaff)
        {
            // Palm Mute
            if(staff->HasFlag(REStaff::PalmMuteBand)) {
                staff->DrawPalmMuteBand (painter, staff->YOffset() + staff->YOffsetOfPalmMute(), sliceRange);
            }
            
            // Let Ring
            if(staff->HasFlag(REStaff::LetRingBand)) {
                staff->DrawLetRingBand (painter, staff->YOffset() + staff->YOffsetOfLetRing(), sliceRange);                    
            }
            
            // Pickstroke
            if(staff->HasFlag(REStaff::StrummingBand)) {
                staff->DrawPickstrokeBand(painter, staff->YOffset() + staff->YOffsetOfStrumming(), sliceRange);
            }
            
            // TSP
            if(staff->HasFlag(REStaff::TappingBand)) {
                staff->DrawTSPBand(painter, staff->YOffset() + staff->YOffsetOfTSP(), sliceRange);
            }
            
            // Text Above
            if(staff->HasFlag(REStaff::TextAboveStaffBand)) {
                staff->DrawTextBand(painter, Reflow::TextAboveTablatureStaff, staff->YOffset() + staff->YOffsetOfTextPositionedAbove(), sliceRange);                    
            }
            
            // Text Below
            if(staff->HasFlag(REStaff::TextBelowStaffBand)) {
                staff->DrawTextBand(painter, Reflow::TextBelowTablatureStaff, staff->YOffset() + staff->YOffsetOfTextPositionedBelow(), sliceRange);                    
            }
        }
    }
}
