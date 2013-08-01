//
//  REStandardStaff.cpp
//  ReflowIphone
//
//  Created by Sebastien on 11/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REStandardStaff.h"
#include "REPainter.h"
#include "RETrack.h"
#include "RESystem.h"
#include "RESlice.h"
#include "RELocator.h"
#include "REScore.h"
#include "RESong.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REBarMetrics.h"
#include "REVoice.h"
#include "REBar.h"

#include <cmath>
#include <sstream>

REStandardStaff::REStandardStaff() : _hand(REStandardStaff::RightHand)
{
    _interline = 3.5;
}

REStandardStaff::~REStandardStaff()
{
    
}

unsigned int REStandardStaff::LineCount() const 
{
    return 9;
}

int REStandardStaff::FirstVoiceIndex() const
{
    if(_hand == REStandardStaff::RightHand) {
        return 0;
    }
    return 2;
}
int REStandardStaff::LastVoiceIndex() const
{
    return FirstVoiceIndex() + 1;
}

void REStandardStaff::DrawSlice(REPainter& painter, int sliceIndex) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    const RESlice* sbar = _parent->SystemBar(sliceIndex);        
    const REBarMetrics& metrics = sbar->Metrics();
    const REBar* bar = sbar->Bar();
    const RETrack* track = Track();
    unsigned int barIndex = bar->Index();
    bool leftHand = (FirstVoiceIndex() != 0);
    
    float width = sbar->Width();
    
	float x0 = 0;//0.5;
    float x1 = x0 + width/* - 1.0*/;
    float y = 0.5;

    if(!painter.IsForcedToBlack()) {
        painter.SetStrokeColor(REColor(0.75,0.75,0.75));
    }
    
    for(unsigned int lineIndex=0; lineIndex<5; ++lineIndex)
    {
        // Initial point
        painter.StrokeHorizontalLine(x0, x1, y);
        y += 2*Interline();
	}
    
    // Draw Leger Lines
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty()) continue;
        
        const REChord* chord = locator.Chord();
        while(chord)
        {
            int minLine = 0;
            int maxLine = LineCount()-1;
            chord->CalculateLineRange(score->Settings().HasFlag(REScoreSettings::TransposingScore), &minLine, &maxLine);
            
            if(minLine < -1) 
            {    
                float x = sbar->XOffsetOfTick(chord->OffsetInTicks()) - Interline();
                int nbLedgerLinesAbove = (-minLine/2);
                for(int ledgerLineIndex = 0; ledgerLineIndex < nbLedgerLinesAbove; ++ledgerLineIndex)
                {
                    float ledgerY = 0.5 + YOffsetOfLine(-(ledgerLineIndex+1)*2);
                    float ledgerX0 = x - Interline();
                    float ledgerX1 = x + UnitSpacing() + Interline();
                    painter.StrokeHorizontalLine(ledgerX0, ledgerX1, ledgerY);
                }
            }
            
            if(maxLine > 9) 
            {    
                float x = sbar->XOffsetOfTick(chord->OffsetInTicks()) - Interline();
                int nbLedgerLinesBelow = (maxLine-8)/2;
                for(int ledgerLineIndex = 0; ledgerLineIndex < nbLedgerLinesBelow; ++ledgerLineIndex)
                {
                    float ledgerY = 0.5 + YOffsetOfLine(10 + ledgerLineIndex * 2);
                    float ledgerX0 = x - Interline();
                    float ledgerX1 = x + UnitSpacing() + Interline();
                    painter.StrokeHorizontalLine(ledgerX0, ledgerX1, ledgerY);
                }
            }
            chord = locator.NextChord();
        }
    }
	
	// Draw Barlines
    //painter.SetStrokeColor(REColor(0,0,0));
    //DrawBarlinesOfSlice(painter, sliceIndex);
    
    // MultiRest
    painter.SetFillColor(REColor(0,0,0));
    if(sbar->IsMultiRest())
    {
        float startX = roundf(sbar->XOffsetAfterLeadingSpace());
        float endX = roundf(sbar->Width() - 3*UnitSpacing());
        float midX = (startX + endX)/2;
        float multiRestY = roundf(YOffsetOfLine(3)) + 0.5;
        float h = 2 * Interline();
        painter.FillRect(RERect(startX, multiRestY, endX - startX, h));
        painter.StrokeVerticalLine(startX+0.5, multiRestY-h/2, multiRestY+h+h/2);
        painter.StrokeVerticalLine(endX+0.5, multiRestY-h/2, multiRestY+h+h/2);
        
        std::ostringstream oss; oss << sbar->BarCount();
        painter.DrawText(oss.str(), REPoint(midX-5.0, -18.0), "Times New Roman", REPainter::Bold, 16, REColor::Black);
    }
    
    // Guides ?
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor(0.20, 0.90, 0.30));
    painter.StrokeLine(REPoint(x0, _yMinBeaming), REPoint(x1, _yMinBeaming));
    painter.StrokeLine(REPoint(x0, _yMaxBeaming), REPoint(x1, _yMaxBeaming));
    
    painter.SetStrokeColor(REColor(0.20, 0.20, 0.90));
    painter.StrokeLine(REPoint(x0, YOffsetOfTupletLine(0)), REPoint(x1, YOffsetOfTupletLine(0)));
    
    painter.SetStrokeColor(REColor(0.90, 0.20, 0.30));
    painter.StrokeLine(REPoint(x0, YOffsetOfSticking()), REPoint(x1, YOffsetOfSticking()));
#endif*/
    
    painter.SetStrokeColor(REColor::Black);
    painter.SetFillColor(REColor::Black);
    
    if(track->IsDrums())
    {
        // Neutral Clef
        if(sbar->IsFirstInSystem())
        {
            float x = metrics.ClefOffset(sbar->IsFirstInSystem());
            float y = YOffsetOfLine(4) + 0.5;
            painter.DrawMusicSymbol("nclef", x, y, 2*Interline());
        }
        
        // .. And no key signature at all
    }
    else 
    {
        // Clef
        bool keySignatureChange = bar->HasKeySignatureChange();
        bool clefChange = track->HasClefChangeAtBar(barIndex);
        const REClefTimeline& clefTimeLine = _track->ClefTimeline(leftHand);
        const REClefItem* clefItem = clefTimeLine.ItemAt(barIndex, RETimeDiv(0), &clefChange);
        Reflow::ClefType clefType = (clefItem ? clefItem->clef : Reflow::TrebleClef);
        Reflow::OttaviaType ottaviaType = (clefItem ? clefItem->ottavia : Reflow::NoOttavia);
        if(sbar->IsFirstInSystem() || clefChange)
        {
            float x = metrics.ClefOffset(sbar->IsFirstInSystem());
            
            if(clefType == Reflow::TrebleClef) {
                float y = YOffsetOfLine(6) + 0.5;
                float size = 1.25 * UnitSpacing();
                std::string txt = "";
                painter.DrawMusicSymbol("gclef", x, y, 2*Interline());
                
                switch(ottaviaType)
                {
                    case Reflow::Ottavia_8va: {
                        y -= 5.8 * UnitSpacing();
                        txt = "8";
                        break;
                    }
                    case Reflow::Ottavia_8vb: {
                        y += 2.1 * UnitSpacing();
                        txt = "8";
                        break;
                    }
                    case Reflow::Ottavia_15ma: {
                        y -= 5.8 * UnitSpacing();
                        txt = "15";
                        break;
                    }
                    case Reflow::Ottavia_15mb: {
                        y += 2.1 * UnitSpacing();
                        txt = "15";
                        break;
                    }
                    default: break;
                }
                
                if(!txt.empty()) {
                    painter.DrawText(txt, REPoint(x + UnitSpacing(), y), "Arial", REPainter::Bold, size, REColor(0,0,0));
                }
            }
            else if(clefType == Reflow::BassClef) {
                float y = YOffsetOfLine(2) + 0.5;
                float size = 1.25 * UnitSpacing();
                std::string txt = "";
                painter.DrawMusicSymbol("fclef", x, y, 2*Interline());
                
                switch(ottaviaType)
                {
                    case Reflow::Ottavia_8va: {
                        y -= 2.8 * UnitSpacing();
                        txt = "8";
                        break;
                    }
                    case Reflow::Ottavia_8vb: {
                        y += 2.4 * UnitSpacing();
                        txt = "8";
                        break;
                    }
                    case Reflow::Ottavia_15ma: {
                        y -= 2.8 * UnitSpacing();
                        txt = "15";
                        break;
                    }
                    case Reflow::Ottavia_15mb: {
                        y += 2.4 * UnitSpacing();
                        txt = "15";
                        break;
                    }
                    default: break;
                }
                
                if(!txt.empty()) {
                    painter.DrawText(txt, REPoint(x + UnitSpacing(), y), "Arial", REPainter::Bold, size, REColor(0,0,0));
                }

            }
            
        }

        // Key signature
        if(keySignatureChange || clefChange || sbar->IsFirstInSystem())
        {
            const REBar* prevBar = bar->PreviousBar();
            
            float dx = UnitSpacing();
            float x = metrics.KeySignatureOffset(sbar->IsFirstInSystem());
            float y = 0.5 * Height();
            int naturals[7]; int nbNaturals = 0;
            int accidentals[7]; int nbAccidentals = 0;
            if(prevBar) {
                nbNaturals = bar->KeySignature().DetermineLinesOfNaturals(clefType, prevBar->KeySignature(), naturals);
            }
            nbAccidentals = bar->KeySignature().DetermineLinesOfAccidentals(clefType, accidentals);
            
            for(int i=0; i<nbNaturals; ++i) {
                y = YOffsetOfLine(naturals[i]);
                painter.DrawMusicSymbol("natural", x, y, 2*Interline());
                x += dx;
            }
            bool useSharps = (bar->KeySignature().SharpCount() > 0);
            for(int i=0; i<nbAccidentals; ++i) {
                y = YOffsetOfLine(accidentals[i]);
                painter.DrawMusicSymbol((useSharps ? "sharp" : "flat"), x, y, 2*Interline());
                x += dx;
            }
        }
    }
    
    // Time Signature
    if(bar->HasTimeSignatureChange()) {
        const REBarMetrics& metrics = sbar->Metrics();    
        float x = /*sbar->XOffset() +*/ metrics.TimeSignatureOffset(sbar->IsFirstInSystem());
        float y = 0.5 * Height();
        DrawTimeSignature(painter, bar->TimeSignature(), REPoint(x, y));
    }
    
    // Draw Note Heads
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty()) continue;
        
        const REChord* chord = locator.Chord();
        while(chord)
        {
            DrawNoteHeadsOfChord(painter, sbar, chord, !score->Settings().InConcertTone());            
            chord = locator.NextChord();
        }
    }
    
    // Beaming and rests
    if(!HasFlag(REStaff::HideRhythm)) {
        DrawBeamingAndRestsOfSlice(painter, sliceIndex);
    }
    
    // Draw Symbols
    IterateSymbolsOfSlice(sliceIndex, [&](const REConstSymbolAnchor& anchor) {
        anchor.symbol->Draw(painter, anchor.origin + anchor.symbol->Offset(), REColor::Black, UnitSpacing());
    });
}

void REStandardStaff::IterateSymbolsOfSlice(int sliceIndex, const REConstSymbolAnchorOperation& op) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    const RESlice* sbar = _parent->SystemBar(sliceIndex);
    const REBarMetrics& metrics = sbar->Metrics();
    const REBar* bar = sbar->Bar();
    const RETrack* track = Track();
    unsigned int barIndex = bar->Index();
    bool leftHand = (FirstVoiceIndex() != 0);
    
    float width = sbar->Width();
    
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty()) continue;
        
        const REChord* chord = locator.Chord();
        while(chord)
        {
            unsigned long tick = chord->OffsetInTicks();
            float psize = 0.5 + UnitSpacing();
            float stretchFactor = sbar->StretchFactor();
            float x = sbar->XOffsetOfTick(tick) - Interline();
            int voice = chord->Phrase()->Voice()->Index();
            const BeamCache* beam = BeamCacheForChord(chord->Index(), voice, sbar->Index());
            
            for(const RESymbol* symbol : chord->Symbols())
            {
				REConstSymbolAnchor csa = {symbol, this, REPoint(x, 0.0f), locator};
                op(csa);
            }
            chord = locator.NextChord();
        }
    }
}

void REStandardStaff::DrawNoteHead(REPainter& painter, const RENote* note, float x, float y, float headX, float psize, const char* noteHead, bool transposed, bool hasStackedSeconds, float &minX) const
{
    const RENote::REStandardRep& rep = note->Representation(transposed);
    
    // Note Head
    if(note->IsSelected() && painter.IsDrawingToScreen()) {painter.SetFillColor(REColor::Blue);}
    if(rep.noteHeadSymbol == Reflow::DefaultNoteHead)
    {
        painter.DrawMusicSymbol(noteHead, headX, y, psize);
    }
    else {
        switch(rep.noteHeadSymbol) {
            case Reflow::CrossNoteHead:            painter.DrawMusicSymbol("cross", headX, y, psize);  break;
            case Reflow::CircledCrossNoteHead:     painter.DrawMusicSymbol("circlecross", headX, y, psize);  break;
            case Reflow::DiamondNoteHead:          painter.DrawMusicSymbol("diamondwhite", headX, y, psize);  break;
            case Reflow::FilledDiamondNoteHead:    painter.DrawMusicSymbol("diamondblack", headX, y, psize);  break;
            case Reflow::DoubleSharpNoteHead:      painter.DrawMusicSymbol("doublesharp", headX, y, psize);  break;
            default:
                painter.DrawMusicSymbol(noteHead, headX, y, psize); break;
        }
    }
    if(note->IsSelected() && painter.IsDrawingToScreen()) {painter.SetFillColor(REColor::Black);}
    
    // Ghost Note
    float deltaAccidental = 0;
    if(note->HasFlag(RENote::GhostNote))
    {
        float yGhost = y - UnitSpacing();
        painter.DrawText("(", REPoint(x-0.75*UnitSpacing()/2,yGhost), "Arial", REPainter::Bold, psize+1, REColor::Black);
        painter.DrawText(")", REPoint(x+1.25*UnitSpacing(),yGhost), "Arial", REPainter::Bold, psize+1, REColor::Black);
        deltaAccidental = -UnitSpacing();
        if(x - deltaAccidental < minX) minX = x - deltaAccidental;
    }
    
    // Accidental ?
    if(rep.accidental != Reflow::NoAccidental) {
        deltaAccidental += (-1.0 * psize * rep.accidentalOffset);
        if(hasStackedSeconds) deltaAccidental -= 0.5 * psize;
    }
    switch(rep.accidental)
    {
        case Reflow::Sharp: {
            float nx = x - (1.0*psize) + deltaAccidental;
            painter.DrawMusicSymbol("sharp", nx, y, psize);
            if(nx < minX) minX = nx;
            break;
        }
        case Reflow::Flat: {
            float nx = x - (1.0*psize) + deltaAccidental;
            painter.DrawMusicSymbol("flat", nx, y, psize);
            if(nx < minX) minX = nx;
            break;
        }
        case Reflow::DoubleSharp: {
            float nx = x - (1.0*psize) + deltaAccidental;
            painter.DrawMusicSymbol("doublesharp", nx, y, psize);
            if(nx < minX) minX = nx;
            break;
        }
        case Reflow::DoubleFlat: {
            float nx = x - (1.25*psize) + deltaAccidental;
            painter.DrawMusicSymbol("doubleflat", nx, y, psize);
            if(nx < minX) minX = nx;
            break;
        }
        case Reflow::Natural: {
            float nx = x - (1.0*psize) + deltaAccidental;
            painter.DrawMusicSymbol("natural", nx, y, psize);
            if(nx < minX) minX = nx;
            break;
        }
    }
}

void REStandardStaff::DrawNoteHeadsOfChord(REPainter& painter, const RESlice* sbar, const REChord* chord, bool transposed) const
{
    unsigned long tick = chord->OffsetInTicks();
    float psize = 0.5 + UnitSpacing();
    float stretchFactor = sbar->StretchFactor();
    float x = sbar->XOffsetOfTick(tick) /*+ sbar->XOffset()*/ - Interline();
    int voice = chord->Phrase()->Voice()->Index();
    const BeamCache* beam = BeamCacheForChord(chord->Index(), voice, sbar->Index());
    
    const char* noteHead = "quarter";
    if(chord->NoteValue() == Reflow::HalfNote) {
        noteHead = "half";
    }
    else if(chord->NoteValue() == Reflow::WholeNote) {
        noteHead = "whole";
    }
    
    
    // Accent and staccato
    if(ShouldDrawAccentsWithNoteHead() && (chord->HasFlag(REChord::Accent) || chord->HasFlag(REChord::StrongAccent) || chord->HasFlag(REChord::Staccato)))
    {
        bool hasAccent = false;
        float yAccent = 0;
        if(beam->flags & REStaff::BeamIsStemDown)
        {
            yAccent = YOffsetOfLine(beam->minLine) - UnitSpacing();
        
            if(chord->HasFlag(REChord::Staccato)) {
                painter.DrawMusicSymbol("dot", x+UnitSpacing()/2, yAccent, psize/2);
                yAccent -= UnitSpacing();
            }
            if(chord->HasFlag(REChord::Accent)) {
                painter.DrawMusicSymbol("accent", x, yAccent, psize);
                yAccent -= UnitSpacing();
            }
            else if(chord->HasFlag(REChord::StrongAccent)) {
                painter.DrawMusicSymbol("strongaccent", x, yAccent, psize);
                yAccent -= UnitSpacing();
            }
        }
        else
        {
            yAccent = YOffsetOfLine(beam->maxLine) + UnitSpacing();
            
            if(chord->HasFlag(REChord::Staccato)) {
                painter.DrawMusicSymbol("dot", x+UnitSpacing()/2, yAccent, psize/2);
                yAccent += UnitSpacing();
            }
            if(chord->HasFlag(REChord::Accent)) {
                painter.DrawMusicSymbol("accent", x, yAccent, psize);
                yAccent += UnitSpacing();
            }
            else if(chord->HasFlag(REChord::StrongAccent)) {
                yAccent += UnitSpacing()/2;
                painter.DrawMusicSymbol("strongaccent", x, yAccent, psize);
                yAccent += UnitSpacing();
            }
        }
    }
    else {
        if(chord->HasFlag(REChord::Accent))
        {
            float y = YOffsetOfAccent();
            painter.DrawMusicSymbol("accent", x, y, psize);
        }
        else if(chord->HasFlag(REChord::StrongAccent))
        {
            float y = YOffsetOfAccent();
            painter.DrawMusicSymbol("strongaccent", x, y, psize);
        }
    }
    
    bool hasStackedSeconds = chord->HasVerticallyStackedSeconds(0 == (beam->flags & REStaff::BeamIsStemDown), transposed);
    bool hasLeftStick = false;
    bool hasRightStick = false;
    int lineOfLeftStick = 100, lineOfRightStick = 100;
    float minX = x;
    for(unsigned int noteIndex=0; noteIndex < chord->NoteCount(); ++noteIndex)
    {
        const RENote* note = chord->Note(noteIndex);
        const RENote::REStandardRep& rep = note->Representation(transposed);
        float y = 0.25 + roundf(YOffsetOfLine(rep.line));
        float headX = x;
        
        // Offset the note head if we have stacked seconds
        if(hasStackedSeconds)
        {
            if(0 == (beam->flags & REStaff::BeamIsStemDown)) {
                if(rep.flags & RENote::StackedSecondOnOppositeSideWithStemUp) {
                    headX += 0.5 * psize;
                }
                else {
                    headX -= 0.5 * psize;
                }
            }
            else {
                if(rep.flags & RENote::StackedSecondOnOppositeSideWithStemDown) {
                    headX -= 0.5 * psize;
                }
                else {
                    headX += 0.5 * psize;
                }
            }
        }
        
        // Sticking
        if(voice == 0)
        {
            if(note->HasFlag(RENote::LeftStick)) {
                if(rep.line < lineOfLeftStick) lineOfLeftStick = rep.line;
                hasLeftStick = true;
            }
            if(note->HasFlag(RENote::RightStick)) {
                if(rep.line < lineOfRightStick) lineOfRightStick = rep.line;
                hasRightStick = true;
            }
        }
        
        // Note Head
        DrawNoteHead(painter, note, x, y, headX, psize, noteHead, transposed, hasStackedSeconds, minX);
        
        // Tie origin
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
                        float x0 = x + 8.0;
                        float x1 = center.x - sbar->XOffset() - 5.0;
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
        if(_parent->IsHorizontalSystem())
        {
            if(note->HasFlag(RENote::TieDestination))
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
                            float x0 = center.x - sbar->XOffset() + 8.0;
                            float x1 = x - 5.0;
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
        }
        
        // Dotted Note
        if(chord->Dots() == 1)
        {
            painter.DrawMusicSymbol("dot", x + (1.4 * psize), y-UnitSpacing()/2, 0.75 * psize);
        }
        else if(chord->Dots() == 2)
        {
            painter.DrawMusicSymbol("dot", x + (1.4 * psize), y-UnitSpacing()/2, 0.75 * psize);
            painter.DrawMusicSymbol("dot", x + (2.0 * psize), y-UnitSpacing()/2, 0.75 * psize);
        }
    }
    
    // Sticking
    if(voice == 0)
    {
        if(hasLeftStick && hasRightStick) 
        {
            float y = YOffsetOfSticking();
#ifdef REFLOW_MAC
            y -= 2.0;
#endif
            if(lineOfLeftStick <= lineOfRightStick) {
                painter.DrawText("L", REPoint(x, y), "Arial", 0, psize, REColor(0,0,0));
                painter.DrawText("R", REPoint(x, y+7.0), "Arial", 0, psize, REColor(0,0,0));
            }
            else {
                painter.DrawText("R", REPoint(x, y), "Arial", 0, psize, REColor(0,0,0));
                painter.DrawText("L", REPoint(x, y+7.0), "Arial", 0, psize, REColor(0,0,0));                
            }
        }
        else if(hasLeftStick) {
            float y = YOffsetOfSticking();
#ifdef REFLOW_MAC
            y -= 2.0;
#endif
            painter.DrawText("L", REPoint(x, y), "Arial", 0, psize, REColor(0,0,0));
        }
        else if(hasRightStick) {
            float y = YOffsetOfSticking();
#ifdef REFLOW_MAC
            y -= 2.0;
#endif
            painter.DrawText("R", REPoint(x, y), "Arial", 0, psize, REColor(0,0,0));            
        }
    }
    
    // Arpeggio
    if(chord->HasFlag(REChord::Arpeggio))
    {
        const BeamCache* beam = BeamCacheForChord(chord->Index(), voice, sbar->Index());
        float y0 = YOffsetOfLine(beam->minLine) - UnitSpacing();
        float y1 = YOffsetOfLine(beam->maxLine) + UnitSpacing();
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
            float graceUnitSpacing = 0.75 * psize;
            REGraceNoteMetrics graceMetrics;
            float graceWidth = note->CalculateGraceNoteMetrics(transposed, graceUnitSpacing, &graceMetrics);
            graceWidth *= stretchFactor;
            
            float graceX0 = minX - UnitSpacing() - graceWidth;
            for(int i=0; i<graceNoteCount; ++i)
            {
                const REGraceNote* graceNote = note->GraceNote(i);
                const RENote::REStandardRep& rep = graceNote->Representation(transposed);
                float y = 0.25 + roundf(YOffsetOfLine(rep.line));
                float graceX = graceX0 + stretchFactor * graceMetrics.XOffsetOfNote(i);
                float headX = graceX;
                
                float minGraceX;
                DrawNoteHead(painter, graceNote, graceX, y, headX, graceUnitSpacing, "quarter", transposed, false, minGraceX);
                
                // Stem
                bool graceStemUp = true;
                float x = headX + graceUnitSpacing;
                float yStem = y - psize * 2.0;
                painter.SetStrokeColor(REColor::Black);
                painter.DrawMusicSymbol("flag8", x, yStem, 0.75 * graceUnitSpacing);
                
                painter.StrokeVerticalLine(x, yStem, y);
                REPoint stroke0 = REPoint(x - psize * 0.6, y - psize * 0.7);
                REPoint stroke1 = REPoint(x + psize * 0.6, y - psize * 1.3);
                painter.StrokeLine(stroke0, stroke1);
            }
        }

    }
}

float REStandardStaff::UnitSpacing() const
{
    return 2.0 * Interline();
}


void REStandardStaff::_DrawSingleStem(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const
{
    unsigned int voiceIndex = chord->Phrase()->Voice()->Index();
    const BeamCache* beam = BeamCacheForChord(chord->Index(), voiceIndex, sbar->Index());
    
    painter.SetStrokeColor(REColor(0.0, 0.0, 0.0, 1.0));
    
    // First Stem
    if(chord->NoteValue() != Reflow::WholeNote)
    {        
        float x = beam->x;
        if(beam->flags & REStaff::BeamIsStemDown) {
            float yBase = YOffsetOfLine(beam->minLine);
            painter.StrokeVerticalLine(x, beam->yStem, yBase);
            
            // Flags when stem goes down
            switch(chord->NoteValue()) {
                case Reflow::EighthNote:        painter.DrawMusicSymbol("flag8inv",  x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::SixteenthNote:     painter.DrawMusicSymbol("flag16inv", x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::ThirtySecondNote:  painter.DrawMusicSymbol("flag32inv", x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::SixtyFourthNote:   painter.DrawMusicSymbol("flag64inv", x+0.5, beam->yStem, UnitSpacing()); break;
                default:break;
            }
        }
        else {
            float yBase = YOffsetOfLine(beam->maxLine);
            painter.StrokeVerticalLine(x, beam->yStem, yBase);
            
            // Flags when stem goes up
            switch(chord->NoteValue()) {
                case Reflow::EighthNote:        painter.DrawMusicSymbol("flag8",  x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::SixteenthNote:     painter.DrawMusicSymbol("flag16", x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::ThirtySecondNote:  painter.DrawMusicSymbol("flag32", x+0.5, beam->yStem, UnitSpacing()); break;
                case Reflow::SixtyFourthNote:   painter.DrawMusicSymbol("flag64", x+0.5, beam->yStem, UnitSpacing()); break;
                default:break;
            }

        }
    }
}

void REStandardStaff::DrawBarlinesOfSlice(REPainter& painter, int sliceIndex) const
{
    float us = UnitSpacing();
    float y0 = 0.5;
    float y1 = Height()+0.5;
    float y2 = y1;
    const RESlice* sbar = _parent->SystemBar(sliceIndex);
    const REBarMetrics& metrics = sbar->Metrics();
    const REBar* bar = sbar->Bar();
    bool lastBarInSong = (sbar->LastBarIndex() == bar->Song()->BarCount()-1);
    
    // Grand staff
    bool isGrandStaff = Track()->IsGrandStaff();
    if(isGrandStaff && Hand() == REStandardStaff::RightHand)
    {
        const REStaff* nextStaff = _parent->Staff(Index()+1);
        if(nextStaff && nextStaff->Type() == Reflow::StandardStaff) {
            y2 = nextStaff->Height() + (nextStaff->YOffset() - YOffset());
        }
        else {
            isGrandStaff = false;
        }
    }
    
    // Start barline
    float x = 0.5;
    painter.StrokeVerticalLine(x, y0, y2);
    
    if(sbar->IsLastInSystem())
    {
        // End barline
        float w = sbar->Width();
        x = w - 0.5;
        painter.StrokeVerticalLine(x, y0, y2);
    }
    
    painter.SetFillColor(REColor(0,0,0,1));
    
    // Repeat start ?
    if(bar->HasFlag(REBar::RepeatStart))
    {
        float w = 5.0;
        float x = metrics.RepeatStartOffset(sbar->IsFirstInSystem());
        float x2 = x + w + 3.5;
        float y = (y0+y1)/2;
        
        if(!isGrandStaff || Hand() == REStandardStaff::RightHand) {
            painter.FillRect(RERect(x, y0, w, y2-y0));
            painter.StrokeVerticalLine(x2, y0, y2);
        }
        painter.DrawMusicSymbol("dot", x2 + 3.5, y - Interline(), 0.75 * UnitSpacing());
        painter.DrawMusicSymbol("dot", x2 + 3.5, y + Interline(), 0.75 * UnitSpacing());
    }
    
    // Repeat end
    if(bar->HasFlag(REBar::RepeatEnd) || lastBarInSong)
    {
        float w = 5.0;
        float x = sbar->Width() - w + 1;
        float x2 = x - 3.5;
        float y = (y0+y1)/2;

        if(!isGrandStaff || Hand() == REStandardStaff::RightHand) {
            painter.FillRect(RERect(x, y0, w, y2-y0));
            painter.StrokeVerticalLine(x2, y0, y2);
        }
        
        if(bar->HasFlag(REBar::RepeatEnd)) {
            painter.DrawMusicSymbol("dot", x2 - 5.5, y - Interline(), 0.75 * UnitSpacing());
            painter.DrawMusicSymbol("dot", x2 - 5.5, y + Interline(), 0.75 * UnitSpacing());
        }
    }
}

void REStandardStaff::ListNoteHeadGizmosOfChord(REGizmoVector& gizmos, const RESlice* sbar, const REChord* chord, bool transposed) const
{
    unsigned long tick = chord->OffsetInTicks();
    float psize = 0.5 + UnitSpacing();
    float x = sbar->XOffsetOfTick(tick) + sbar->XOffset();
    int barIndex = sbar->BarIndex();
    
    for(unsigned int noteIndex=0; noteIndex < chord->NoteCount(); ++noteIndex)
    {
        const RENote* note = chord->Note(noteIndex);
        const RENote::REStandardRep& rep = note->Representation(transposed);
        float y = 0.25 + roundf(YOffsetOfLine(rep.line)) + YOffset();
        
        REGizmo gizmo = {Reflow::NoteHeadGizmo, RERect(x-psize/2, y-psize/2, psize, psize), note, static_cast<uint16_t>(barIndex)};
        gizmos.push_back(gizmo);
    }
}

void REStandardStaff::ListNoteGizmos(REGizmoVector& gizmos) const
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
                ListNoteHeadGizmosOfChord(gizmos, sbar, chord, score->Settings().HasFlag(REScoreSettings::TransposingScore));
                chord = locator.NextChord();
            }
        }
    }
}
