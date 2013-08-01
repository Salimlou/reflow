//
//  RETablatureStaff.cpp
//  Reflow
//
//  Created by Sebastien on 04/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REPainter.h"
#include "RETablatureStaff.h"
#include "RETrack.h"
#include "RESystem.h"
#include "RESlice.h"
#include "RELocator.h"
#include "REScore.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REBarMetrics.h"
#include "REVoice.h"
#include "REBar.h"
#include "RESymbol.h"

#include <sstream>
#include <cmath>

//---------------------------------------------------------------------------------------
RETablatureStaff::RETablatureStaff()
{
    _interline = 9.0;
}

//---------------------------------------------------------------------------------------
RETablatureStaff::~RETablatureStaff()
{
}

//---------------------------------------------------------------------------------------
unsigned int RETablatureStaff::LineCount() const
{
    return (_track != NULL ? _track->StringCount() : 6);
}

//---------------------------------------------------------------------------------------
void RETablatureStaff::DrawSlice(REPainter& painter, int sliceIndex) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    unsigned int nbStrings = _track->StringCount();
    const RESlice* sbar = _parent->SystemBar(sliceIndex);        
    unsigned int barIndex = sbar->BarIndex();
    float width = sbar->Width();
    
    // Draw Lines
    //-----------------------------------------------------------------------------------
    float x0 = 0.5;
    float x1 = x0 + width /*- 0.5*/;
    float y = 0.5;
    painter.SetStrokeColor(painter.IsForcedToBlack() ? REColor::Black : REColor(0.75,0.75,0.75));
    for(unsigned int stringIndex=0; stringIndex<nbStrings; ++stringIndex)
    {
        // Initial point
        float startX = x0;
        
        // Make holes in the line for fret numbers
        for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
        {
            RELocator locator(song, barIndex, _track->Index(), voiceIndex);
            const REPhrase* phrase = locator.Phrase();
            if(phrase->IsEmpty()) continue;
            
            const REChord* chord = locator.Chord();
            while(chord) {
                const RENote* note = chord->NoteOnString(stringIndex);
                if(note) 
                {
                    unsigned long tick = chord->OffsetInTicks();
                    float x = sbar->XOffsetOfTick(tick) /*+ sbar->XOffset()*/;
                    if(note->Fret() >= 10) {
                        painter.StrokeHorizontalLine(startX, x-6.5f, y);
                        startX = x + 5.5f;
                    }
                    else {
                        painter.StrokeHorizontalLine(startX, x-4.5f, y);
                        startX = x + 3.5f;
                    }
                }
                chord = locator.NextChord();
            }
        }
        
        // Final point
        painter.StrokeHorizontalLine(startX, x1, y);
        y += Interline();
    }
    
    // Draw Barlines
    //-----------------------------------------------------------------------------------
    painter.SetStrokeColor(REColor::Black);
    DrawBarlinesOfSlice(painter, sliceIndex);
    
    // Draw TAB Header
    //-----------------------------------------------------------------------------------
    if(sliceIndex == 0)
    {
        painter.SetFillColor(painter.IsForcedToBlack() ? REColor::Black : REColor(0.35,0.35,0.35));
        RERect rcHeader = RERect(x0, 0.5, 20.0, Height());
        painter.FillRect(rcHeader);
        
        float letterSize = 11.0;
        float letterSpacing = 3.0;
#ifdef REFLOW_MAC
        float initialOffsetY = 10.0;
#else
        float initialOffsetY = 0.0;
#endif
        if(LineCount() <= 4) {
            letterSize = 8.0;
            letterSpacing = 0.0;
#ifdef REFLOW_MAC
            initialOffsetY -= 3.0;
#else
            initialOffsetY -= 2.0;
#endif
        }
        else if(LineCount() <= 5) {
            letterSize = 9.0;
            letterSpacing = 1.0;
            initialOffsetY -= 1.0;
        }
        float totalLetterSize = (letterSize + letterSpacing) * 3.0;
        float offsetY = (rcHeader.Height() - totalLetterSize) * 0.5;
        float yT = offsetY + initialOffsetY;
        float yA = yT + letterSpacing + letterSize;
        float yB = yA + letterSpacing + letterSize;
        painter.SetFillColor(REColor(1,1,1,1));
        painter.DrawTextInRect("T", RERect(rcHeader.origin.x, rcHeader.Top() + yT, rcHeader.Width(), letterSize+2), "Arial", REPainter::Bold, letterSize, REColor(0.95, 0.95, 0.95));
        painter.DrawTextInRect("A", RERect(rcHeader.origin.x, rcHeader.Top() + yA, rcHeader.Width(), letterSize+2), "Arial", REPainter::Bold, letterSize, REColor(0.95, 0.95, 0.95));
        painter.DrawTextInRect("B", RERect(rcHeader.origin.x, rcHeader.Top() + yB, rcHeader.Width(), letterSize+2), "Arial", REPainter::Bold, letterSize, REColor(0.95, 0.95, 0.95));
    }
    
    // MultiRest
    painter.SetFillColor(REColor(0,0,0));
    if(sbar->IsMultiRest())
    {
        float startX = roundf(sbar->XOffsetAfterLeadingSpace());
        float endX = roundf(sbar->Width() - 3*UnitSpacing());
        float midX = (startX + endX)/2;
        float multiRestY = roundf(YOffsetOfLine(LineCount()/2 - 1)) + 0.5;
        float h = Interline();
        painter.FillRect(RERect(startX, multiRestY, endX - startX, h));
        painter.StrokeVerticalLine(startX+0.5, multiRestY-h/2, multiRestY+h+h/2);
        painter.StrokeVerticalLine(endX+0.5, multiRestY-h/2, multiRestY+h+h/2);
        
        std::ostringstream oss; oss << sbar->BarCount();
        painter.DrawText(oss.str(), REPoint(midX-5.0, -18.0), "Times New Roman", REPainter::Bold, 16, REColor::Black);
    }
    
    // Time Signature
    //-----------------------------------------------------------------------------------
    if(!HasFlag(REStaff::HideRhythm))
    {
        const REBar* bar = sbar->Bar();
        if(bar->HasTimeSignatureChange()) {
            const REBarMetrics& metrics = sbar->Metrics();    
            float x = metrics.TimeSignatureOffset(sbar->IsFirstInSystem());
            float y = 0.5 * Height();
            DrawTimeSignature(painter, bar->TimeSignature(), REPoint(x, y));
        }
    }
    
    // Draw Frets of Chord (pass 0: normal notes)
    //-----------------------------------------------------------------------------------
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty()) continue;
        
        const REChord* chord = locator.Chord();
        while(chord)
        {
            DrawFretsOfChord(painter, sbar, chord);
            chord = locator.NextChord();
        }
    }
    
    // Beaming and rests
    if(!HasFlag(REStaff::HideRhythm)) {
        DrawBeamingAndRestsOfSlice(painter, sliceIndex);
    }
    
    // Draw Symbols
    IterateSymbolsOfSlice(sliceIndex, [&](const REConstSymbolAnchor& anchor)
    {
        anchor.symbol->Draw(painter, anchor.origin + anchor.symbol->Offset(), REColor::Black, UnitSpacing());
    });
}

void RETablatureStaff::IterateSymbolsOfSlice(int sliceIndex, const REConstSymbolAnchorOperation& op) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    unsigned int nbStrings = _track->StringCount();
    const RESlice* slice = _parent->SystemBar(sliceIndex);
    unsigned int barIndex = slice->BarIndex();
    
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty()) continue;
        
        const REChord* chord = locator.Chord();
        while(chord)
        {
            unsigned long tick = chord->OffsetInTicks();
            float stretchFactor = slice->StretchFactor();
            float x = slice->XOffsetOfTick(tick) - 1.5;
            int voice = chord->Phrase()->Voice()->Index();
            
            // Symbols of the chord
            for(const RESymbol* symbol : chord->Symbols())
            {
                REConstSymbolAnchor anchor = {symbol, this, REPoint(x, 0.0f), chord->Locator()};
                op(anchor);
            }
            
            chord = locator.NextChord();
        }
    }
}

//---------------------------------------------------------------------------------------
// NOTE: This function is called between BeginTextBatched/EndTextBatched pair
void RETablatureStaff::DrawFretsOfChord(REPainter& painter, const RESlice* sbar, const REChord* chord) const
{
    const REPhrase* phrase = chord->Phrase();
    unsigned long tick = chord->OffsetInTicks();
    float stretchFactor = sbar->StretchFactor();
    float x = sbar->XOffsetOfTick(tick) /*+ sbar->XOffset()*/ - 1.5;
    int voice = chord->Phrase()->Voice()->Index();
    
    bool hasBrush = chord->HasFlag(REChord::Brush);
    bool hasArpeggio = chord->HasFlag(REChord::Arpeggio);
    float minX = x;
    
    bool gray = painter.ShouldGrayOutInactiveVoice() && voice != painter.ActiveVoiceIndex();
    REColor baseFretColor = gray ? REColor::Gray : REColor::Black;
    
    for(unsigned int noteIndex=0; noteIndex < chord->NoteCount(); ++noteIndex)
    {
        const RENote* note = chord->Note(noteIndex);
        
        float y = YOffsetOfLine(note->String());
        /*if((note->HasFlag(RENote::TieDestination) && pass == 1)  ||
           (!note->HasFlag(RENote::TieDestination) && pass == 0))*/
        {
            std::ostringstream oss; 
            if(note->HasFlag(RENote::DeadNote)) {
                painter.DrawText("x", REPoint(x-2.0,y-7.0), "Arial", REPainter::Bold, 11.0, baseFretColor);
                
                // Ghost
                if(note->HasFlag(RENote::GhostNote) || note->HasFlag(RENote::TieDestination)) {
                    float yGhost = y - 7.0;
                    float nx = x - UnitSpacing();
                    if(nx < minX) minX = nx;
                    painter.DrawText("(", REPoint(nx,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                    painter.DrawText(")", REPoint(x+UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                }
            }
            else
            {
                REColor fretColor = (note->IsSelected() && painter.IsDrawingToScreen()) ? REColor::Blue : REColor::Black;
                oss << note->Fret();
                if(note->Fret() >= 10) {
                    painter.DrawText(oss.str(), REPoint(x-5.0,y-7.0), "Arial", REPainter::Bold, 11.0, fretColor);

                    // Ghost
                    if(note->HasFlag(RENote::GhostNote) || note->HasFlag(RENote::TieDestination)) {
                        float yGhost = y - 7.0;
                        float nx = x - 1.25 * UnitSpacing();
                        if(nx < minX) minX = nx;
                        painter.DrawText("(", REPoint(minX,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                        painter.DrawText(")", REPoint(x+1.25*UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                    }
                }
                else {
                    if(note->IsSelected() && painter.IsDrawingToScreen()) {
                        painter.DrawText(oss.str(), REPoint(x-2.0,y-7.0), "Arial", REPainter::Bold, 11.0, REColor::Blue);
                    }
                    else {
                        //painter.DrawTextBatched(oss.str(), REPoint(x-2.0,y-7.0));
                        std::ostringstream fretstr;
                        fretstr << "fret" << note->Fret();
                        painter.DrawMusicSymbol(fretstr.str().c_str(), x, y, 1.15 * UnitSpacing());
                    }
                    
                    // Ghost
                    if(note->HasFlag(RENote::GhostNote) || note->HasFlag(RENote::TieDestination)) {
                        float yGhost = y - 7.0;
                        float nx = x - UnitSpacing();
                        if(nx < minX) minX = nx;
                        painter.DrawText("(", REPoint(nx,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                        painter.DrawText(")", REPoint(x+UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                    }
                }
            }
        }

        // Tied Note
        if(note->HasFlag(RENote::TieOrigin)) 
        {
            const RENote* tied = note->FindDestinationOfTiedNote();
            if(tied != NULL)
            {
                int barOfTiedNote = tied->Chord()->Phrase()->Index();
                if(_parent->BarRange().IsInRange(barOfTiedNote))
                {
                    REPoint center(0,0);
                    FindCenterOfNote(tied, &center);
                    
                    painter.PathBegin();
                    {
                        float x0 = x + 6.0;
                        float x1 = center.x - sbar->XOffset() - 6.0;
                        float y0 = y + 3.0;
                        painter.PathMoveToPoint(x0, y0);
                        //painter.PathQuadCurveToPoint(REPoint(x1, y0), REPoint((x0+x1)/2, y0+3.0));
                        REPoint cp1 = REPoint((x0+x1)*0.50, y0+3.0);
                        REPoint cp2 = REPoint((x0+x1)*0.50, y0+3.0);
                        painter.PathCurveToPoint(REPoint(x1,y0), cp1, cp2);
                        REPoint cp3 = REPoint((x0+x1)*0.50, y0+4.5);
                        REPoint cp4 = REPoint((x0+x1)*0.50, y0+4.5);
                        painter.PathCurveToPoint(REPoint(x0,y0), cp3, cp4);
                    }
                    painter.PathClose();
                    painter.PathFill();
                    painter.PathStroke();
                }
            }
        }
        
        // If Staff is in an horizontal system, we need to draw the tie destination too
        /*if(_parent->IsHorizontalSystem())
        {
            if(note->HasFlag(RENote::TieDestination) && pass == 0)
            {
                const RENote* tiedOrigin = note->FindOriginOfTiedNote();
                if(tiedOrigin != NULL)
                {
                    int barOfTiedNote = tiedOrigin->Chord()->Phrase()->Index();
                    if(barOfTiedNote != sbar->BarIndex())
                    {
                        REPoint center(0,0);
                        FindCenterOfNote(tiedOrigin, &center);
                        
                        painter.PathBegin();
                        {
                            float x0 = center.x - sbar->XOffset() + 6.0;
                            float x1 = x - 6.0;
                            float y0 = y + 3.0;
                            painter.PathMoveToPoint(x0, y0);
                            //painter.PathQuadCurveToPoint(REPoint(x1, y0), REPoint((x0+x1)/2, y0+3.0));
                            REPoint cp1 = REPoint((x0+x1)*0.50, y0+3.0);
                            REPoint cp2 = REPoint((x0+x1)*0.50, y0+3.0);
                            painter.PathCurveToPoint(REPoint(x1,y0), cp1, cp2);
                            REPoint cp3 = REPoint((x0+x1)*0.50, y0+4.5);
                            REPoint cp4 = REPoint((x0+x1)*0.50, y0+4.5);
                            painter.PathCurveToPoint(REPoint(x0,y0), cp3, cp4);
                        }
                        painter.PathClose();
                        painter.PathFill();
                        painter.PathStroke();
                    }
                }
            }
        }*/
        

#if 0
        // Legato
        if(note->HasFlag(RENote::Legato))
        {
            const RENote* prev = note->FindPreviousNoteOnSameString();
            if(prev == NULL || !prev->HasFlag(RENote::Legato)) 
            {
                const RENote* last = note->FindLastSiblingNoteOnSameStringInLegatoSuite();
                if(last)
                {
                    REPoint center(0,0);
                    FindCenterOfNote(last, &center);
                    
                    painter.PathBegin();
                    {
                        float x0 = x + 6.0;
                        float x1 = center.x - sbar->XOffset() - 6.0;
                        float y0 = y - UnitSpacing();
                        painter.PathMoveToPoint(x0, y0);
                        REPoint cp1 = REPoint((x0+x1)*0.50, y0-3.0);
                        REPoint cp2 = REPoint((x0+x1)*0.50, y0-3.0);
                        painter.PathCurveToPoint(REPoint(x1,y0), cp1, cp2);
                        REPoint cp3 = REPoint((x0+x1)*0.50, y0-4.5);
                        REPoint cp4 = REPoint((x0+x1)*0.50, y0-4.5);
                        painter.PathCurveToPoint(REPoint(x0,y0), cp3, cp4);
                    }
                    painter.PathClose();
                    painter.PathFill();
                    painter.PathStroke();
                }
            }
        }
#endif
        
        // Bend
        if(note->HasBend()) {
            const REBend& bend = note->Bend();
            bend.Draw(painter, REPoint(x+UnitSpacing(), y), RESize(3*UnitSpacing(), 5*UnitSpacing()));
        }
        
        // Slide Out
        float dy = 0.25 * UnitSpacing();
        Reflow::SlideOutType slideOut = note->SlideOut();
        if(slideOut == Reflow::ShiftSlide)
        {
            float x0 = x + UnitSpacing();
            float x1 = 0;
            const RENote* destNote = note->FindDestinationOfTiedNote();
            bool goesUp = (destNote ? (note->Fret() <= destNote->Fret()) : true);
            if(destNote) 
            {
                const REChord* destChord = destNote->Chord();
                const REPhrase* destPhrase = destChord->Phrase();
                int destBarIndex = destPhrase->Index();
                if(phrase == destPhrase) {
                    // Shift slide in the same bar
                    x1 = sbar->XOffsetOfTick(destNote->Chord()->OffsetInTicks()) - UnitSpacing();
                }
                else if(_parent->BarRange().IsInRange(destBarIndex)) {
                    // Shift slide in the same system (but different bars)
                    const RESlice* destSlice = _parent->SystemBarWithBarIndex(destBarIndex);
                    float dxSlice = (destSlice->XOffset() - sbar->XOffset());
                    x1 = dxSlice + destSlice->XOffsetOfTick(destChord->OffsetInTicks()) - UnitSpacing();
                }
                else {
                    // Shift slide but not on the same system
                    x1 = x + 2*UnitSpacing();
                }
            }
            else {
                x1 = x + 2 * UnitSpacing();
            }
            
            if(goesUp) {painter.StrokeLine(REPoint(x0, y + dy), REPoint(x1, y - dy));}
            else       {painter.StrokeLine(REPoint(x0, y - dy), REPoint(x1, y + dy));}
        }
        else if(slideOut == Reflow::SlideOutHigh) {
            painter.StrokeLine(REPoint(x + UnitSpacing(), y + dy), REPoint(x + 2*UnitSpacing(), y - dy));
        }
        else if(slideOut == Reflow::SlideOutLow) {
            painter.StrokeLine(REPoint(x + UnitSpacing(), y - dy), REPoint(x + 2*UnitSpacing(), y + dy));
        }
        
        // Slide In
        Reflow::SlideInType slideIn = note->SlideIn();
        if(slideIn == Reflow::SlideInFromAbove) {
            painter.StrokeLine(REPoint(x - 2*UnitSpacing(), y + dy), REPoint(x - UnitSpacing(), y - dy));
        }
        else if(slideIn == Reflow::SlideInFromBelow) {
            painter.StrokeLine(REPoint(x - 2*UnitSpacing(), y - dy), REPoint(x - UnitSpacing(), y + dy));
        }
    }
    
    // Brush and arpeggio
    if(hasBrush)
    {
        int minLine = 1000, maxLine = -1000;
        chord->CalculateStringRange(&minLine, &maxLine);
        float y0 = YOffsetOfLine(minLine) - UnitSpacing();
        float y1 = YOffsetOfLine(maxLine) + UnitSpacing();
        float x = minX - UnitSpacing();
        DrawBrush(painter, x, y0, y1, !chord->HasFlag(REChord::StrumUpwards));
    }
    else if(hasArpeggio)
    {
        int minLine = 1000, maxLine = -1000;
        chord->CalculateStringRange(&minLine, &maxLine);
        float y0 = YOffsetOfLine(minLine) - UnitSpacing();
        float y1 = YOffsetOfLine(maxLine) + UnitSpacing();
        float x = minX - UnitSpacing();
        DrawArpeggio(painter, x, y0, y1, !chord->HasFlag(REChord::StrumUpwards));
    }
    
    // Grace Notes
    for(unsigned int noteIndex=0; noteIndex < chord->NoteCount(); ++noteIndex)
    {
        const RENote* note = chord->Note(noteIndex);
        int graceNoteCount = note->GraceNoteCount();
        if(graceNoteCount> 0)
        {
            float graceUnitSpacing = 0.75 * UnitSpacing();
            REGraceNoteMetrics graceMetrics;
            float graceWidth = note->CalculateGraceNoteMetrics(false, graceUnitSpacing, &graceMetrics);
            graceWidth *= stretchFactor;
            
            float graceX0 = minX - UnitSpacing() - graceWidth;
            for(int i=0; i<graceNoteCount; ++i)
            {
                float x = graceX0;
                const REGraceNote* graceNote = note->GraceNote(i);
                
                float y = YOffsetOfLine(graceNote->String());
                {
                    std::ostringstream oss;
                    if(graceNote->HasFlag(RENote::DeadNote)) {
                        painter.DrawText("x", REPoint(x-2.0,y-7.0), "Arial", 0, 8.0, REColor::Black);
                        
                        // Ghost
                        if(graceNote->HasFlag(RENote::GhostNote)) {
                            float yGhost = y - 7.0;
                            float nx = x - UnitSpacing();
                            if(nx < minX) minX = nx;
                            painter.DrawText("(", REPoint(nx,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                            painter.DrawText(")", REPoint(x+UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                        }
                    }
                    else {
                        oss << graceNote->Fret();
                        if(graceNote->Fret() >= 10) {
                            if(note->IsSelected() && painter.IsDrawingToScreen()) {
                                painter.DrawText(oss.str(), REPoint(x-5.0,y-7.0), "Arial", REPainter::Bold, 8.0, REColor::Blue);
                            }
                            else {
                                painter.DrawText(oss.str(), REPoint(x-5.0,y-7.0), "Arial", REPainter::Bold, 8.0, baseFretColor);
                            }
                            
                            // Ghost
                            if(graceNote->HasFlag(RENote::GhostNote)) {
                                float yGhost = y - 7.0;
                                float nx = x - 1.25 * UnitSpacing();
                                if(nx < minX) minX = nx;
                                painter.DrawText("(", REPoint(minX,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                                painter.DrawText(")", REPoint(x+1.25*UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                            }
                        }
                        else {
                            if(note->IsSelected() && painter.IsDrawingToScreen()) {
                                painter.DrawText(oss.str(), REPoint(x-2.0,y-7.0), "Arial", REPainter::Bold, 8.0, REColor::Blue);
                            }
                            else {
                                painter.DrawText(oss.str(), REPoint(x-2.0,y-7.0), "Arial", REPainter::Bold, 8.0, baseFretColor);
                            }
                            
                            // Ghost
                            if(graceNote->HasFlag(RENote::GhostNote)) {
                                float yGhost = y - 7.0;
                                float nx = x - UnitSpacing();
                                if(nx < minX) minX = nx;
                                painter.DrawText("(", REPoint(nx,yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                                painter.DrawText(")", REPoint(x+UnitSpacing(),yGhost), "Arial", REPainter::Bold, 8.0, REColor::Black);
                            }
                        }
                    }
                }
            }
        }
    }
}

float RETablatureStaff::UnitSpacing() const
{
    return 7.0;
}

void RETablatureStaff::_DrawSingleStem(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const
{
    unsigned int voiceIndex = chord->Phrase()->Voice()->Index();    
    bool stemDown = (voiceIndex == LowVoiceIndex());
    const BeamCache* beam = BeamCacheForChord(chord->Index(), voiceIndex, sbar->Index());
    unsigned long tick0 = chord->OffsetInTicks();
    
    float x0 = 0.5 + sbar->XOffsetOfTick(tick0) /*+ sbar->XOffset()*/;
    y = 0.5 + y;
    float y0 = y;
    float h = 0.5 * UnitSpacing();
    
    painter.SetStrokeColor(REColor(0.0, 0.0, 0.0, 1.0));
    
    // First Stem
    if(chord->NoteValue() == Reflow::HalfNote) {
        if(orientation == 0) {
            painter.StrokeVerticalLine(x0, y0, y0+5.0);
        }
        else {
            painter.StrokeVerticalLine(x0, y0, y0-5.0);
        }
    }
    else if(chord->NoteValue() != Reflow::WholeNote) 
    {
        float dy = (fabsf(h)+1) * (chord->NoteValue() - Reflow::EighthNote);
        
        float x = beam->x;
        float y = (stemDown ? YOffsetOfLine(_track->StringCount()-1) + 20.0 + dy: -20 - dy);
        
        if(stemDown) {
            float yBase = YOffsetOfLine(beam->maxLine) + 5.0;
            painter.StrokeVerticalLine(x, y, yBase);
        }
        else {
            float yBase = YOffsetOfLine(beam->minLine) - 5.0;
            painter.StrokeVerticalLine(x, y, yBase);
        }
        
        // Secondary beams
        float y1 = y;
        float yinc = (stemDown ? -(h+1) : (h+1));
        
        for(int noteValue = Reflow::EighthNote; noteValue <= chord->NoteValue(); ++noteValue)
        {
            painter.FillRect(RERect(x, y1, 3.0, h));
            y1 += yinc;
        }
    }
}

void RETablatureStaff::_DrawStem(REPainter& painter, const RESlice* slice, const REChord* chord, const BeamCache* beam) const
{
    int voiceIndex = chord->Phrase()->Voice()->Index();
    bool stemDown = (voiceIndex == LowVoiceIndex());
    
    float x = beam->x;
    if(stemDown) {
        float yBase = YOffsetOfLine(beam->maxLine) + 5.0;
        painter.StrokeVerticalLine(x, beam->yStem, yBase);
    }
    else {
        float yBase = YOffsetOfLine(beam->minLine) - 5.0;
        painter.StrokeVerticalLine(x, beam->yStem, yBase);
    }
}

void RETablatureStaff::CalculateBeamingCoordinates(const REChord* firstChord, const REChord* lastChord, BeamCache* beamCache)
{
    int firstChordIndex = firstChord->Index();
    int lastChordIndex = lastChord->Index();
    int nbChords = lastChordIndex - firstChordIndex + 1;
    BeamCache* firstBeam = &beamCache[0];
    BeamCache* lastBeam = &beamCache[nbChords-1];
    int voiceIndex = firstChord->Phrase()->Voice()->Index();
    bool beamStemDown = (voiceIndex == LowVoiceIndex());
    bool forceHorizontal = false;
    
    Reflow::NoteValue maxNoteValue = Reflow::EighthNote;
    for(const REChord* chord = firstChord; chord != lastChord; chord = chord->NextSibling()) {
        maxNoteValue = std::max<Reflow::NoteValue>(maxNoteValue, chord->NoteValue());
    }
    
    int dh = (maxNoteValue - Reflow::EighthNote);
    float dy = dh * 0.5 * UnitSpacing();
    
    float y = (beamStemDown ? YOffsetOfLine(_track->StringCount()-1) + 20.0 + dy: -20 - dy);
    for(int i=0; i<nbChords; ++i)
    {
        beamCache[i].yStem = y;
    }
}

void RETablatureStaff::ListNoteFretGizmosOfChord(REGizmoVector& gizmos, const RESlice* sbar, const REChord* chord) const
{
    const REPhrase* phrase = chord->Phrase();
    unsigned long tick = chord->OffsetInTicks();
    float x = sbar->XOffsetOfTick(tick) + sbar->XOffset() - 1.5;
    int voice = chord->Phrase()->Voice()->Index();
    
    float w = 1.5 * UnitSpacing();
    float h = 1.5 * UnitSpacing();
    
    for(unsigned int noteIndex=0; noteIndex < chord->NoteCount(); ++noteIndex)
    {
        const RENote* note = chord->Note(noteIndex);   
        float y = YOffset() + YOffsetOfLine(note->String());
        
        REGizmo gizmo = {Reflow::NoteFretGizmo, RERect(x-w/2, y-h/2, w, h), note, static_cast<uint16_t>(phrase->Index())};
        gizmos.push_back(gizmo);
    }
}

void RETablatureStaff::ListNoteGizmos(REGizmoVector& gizmos) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    
    for(unsigned int i=0; i<_parent->SystemBarCount(); ++i)
    {
        const RESlice* sbar = _parent->SystemBar(i);        
        unsigned int barIndex = sbar->BarIndex();
        
        for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
        {
            RELocator locator(song, barIndex, _track->Index(), voiceIndex);
            const REPhrase* phrase = locator.Phrase();
            if(phrase->IsEmpty()) continue;
            
            const REChord* chord = locator.Chord();
            while(chord) {
                ListNoteFretGizmosOfChord(gizmos, sbar, chord);
                chord = locator.NextChord();
            }
        }
    }
}
