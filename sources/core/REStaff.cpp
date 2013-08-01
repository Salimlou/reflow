//
//  REStaff.cpp
//  Reflow
//
//  Created by Sebastien on 01/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REStaff.h"
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
#include "REVoice.h"
#include "REBarMetrics.h"
#include "REBar.h"
#include "REFunctions.h"

#include <iostream>
#include <cmath>


REStaff::REStaff()
: _parent(0), _index(-1), _type(Reflow::StandardStaff),
    _yOffset(0), _height(0), _topSpacing(0), _bottomSpacing(0), _interline(3.5), _vibratoYOffset(0), _flags(0)
{
}

REStaff::~REStaff()
{
    _ClearBeamCache();
}

float REStaff::YOffsetOfLine(int lineIndex) const
{
    return lineIndex * _interline;
}

float REStaff::YOffsetOfAccent() const
{
    return _accentYOffset;
}

float REStaff::YOffsetOfSticking() const
{
    return _stickingYOffset;
}

float REStaff::YOffsetOfTupletLine(int voice) const 
{
    return _tupletLineYOffset[voice];
}

float REStaff::YOffsetOfStrumming() const
{
    return _strummingYOffset;
}

float REStaff::YOffsetOfVibrato() const
{
    return _vibratoYOffset;
}

float REStaff::YOffsetOfOttavia() const
{
    return _ottaviaYOffset;
}

int REStaff::LineAtYOffset(float y) const
{
#ifdef REFLOW_IOS
    return y/_interline;
#else
    return lround(y / _interline);
#endif
}

float REStaff::ChordDiagramHeight() const
{
    return 12.0 + REChordDiagram::HeightForSize(ChordDiagramSize());
}

bool REStaff::IsYOffsetInside(float y, float* relativeY) const
{
    float dy = y - _yOffset;
    if(relativeY != NULL) {
        *relativeY = dy;
    }
    if(dy < -_topSpacing) {
        return false;
    }
    else if(dy > (_height + _bottomSpacing)) {
        return false;
    }
    return true;
}

unsigned int REStaff::LineCount() const
{
    return 9;
}

void REStaff::DrawBarlinesOfSlice(REPainter& painter, int sliceIndex) const
{
    float us = UnitSpacing();
    float y0 = 0.5;
    float y1 = Height()+0.5;
    const RESlice* sbar = _parent->SystemBar(sliceIndex);
    const REBarMetrics& metrics = sbar->Metrics();
    const REBar* bar = sbar->Bar();
    bool lastBarInSong = (sbar->LastBarIndex() == bar->Song()->BarCount()-1);
        
    // Start barline
    float x = 0.5;
    painter.StrokeVerticalLine(x, y0, y1);
    
    if(sbar->IsLastInSystem())
    {
        // End barline
        float w = sbar->Width();
        x = w - 0.5;
        painter.StrokeVerticalLine(x, y0, y1);
    }
    
    painter.SetFillColor(REColor(0,0,0,1));
    
    // Repeat start ?
    if(bar->HasFlag(REBar::RepeatStart))
    {
        float w = 5.0;
        float x = metrics.RepeatStartOffset(sbar->IsFirstInSystem());
        float x2 = x + w + 3.5;
        float y = (y0+y1)/2;
        
        painter.FillRect(RERect(x, y0, w, Height()));
        painter.StrokeVerticalLine(x2, y0, y1);
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
        
        painter.FillRect(RERect(x, y0, w, Height()));
        painter.StrokeVerticalLine(x2, y0, y1);
        if(bar->HasFlag(REBar::RepeatEnd)) {
            painter.DrawMusicSymbol("dot", x2 - 5.5, y - Interline(), 0.75 * UnitSpacing());
            painter.DrawMusicSymbol("dot", x2 - 5.5, y + Interline(), 0.75 * UnitSpacing());
        }
    }
}

void REStaff::DrawBarlines(REPainter& painter) const
{
    float y0 = 0.5;
    float y1 = Height()+0.5;
    for(unsigned int i=0; i<_parent->SystemBarCount(); ++i)
    {
        const RESlice* sbar = _parent->SystemBar(i);
        float x = roundf(sbar->XOffset()) + 0.5;
        painter.StrokeVerticalLine(x, y0, y1);
    }
    
    // Last Barline
    {
        const RESlice* sbar = _parent->SystemBar(_parent->SystemBarCount()-1);
        float w = sbar->Width();
        float x = roundf(sbar->XOffset()) + 0.5 + w - 1.0;
        painter.StrokeVerticalLine(x, y0, y1);
    }
}

void REStaff::IterateSymbols(const REConstSymbolAnchorOperation& op) const
{
    int sliceCount = _parent->SystemBarCount();
    for(int i=0; i<sliceCount; ++i)
    {
        const RESlice* slice = _parent->SystemBar(i);
        IterateSymbolsOfSlice(i, [&](const REConstSymbolAnchor& anchor)
        {
            REConstSymbolAnchor an = anchor;
            an.origin.x += slice->XOffset();
            op(an);
        });
    }
}


void REStaff::_RefreshMetrics()
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const REScoreSettings& settings = score->Settings();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    bool isStandard = (Type() == Reflow::StandardStaff);
    bool isTablature = (Type() == Reflow::TablatureStaff);
    bool systemHasStandard = (system->HasStandardStaffForTrack(_track));
    bool systemHasTablature = (system->HasTablatureStaffForTrack(_track));
    
    bool hideDynamics = settings.HasFlag(REScoreSettings::HideDynamics);
    
    if(HasFlag(REStaff::HideRhythm)) {
        _ClearBeamCache();
    }
    else {
        _CalculateBeamCache();
    }
    _height = YOffsetOfLine(LineCount()-1);
    _topSpacing = 19.0;
    _bottomSpacing = 19.0;
    
    _topSpacing = ceilf(_topSpacing);
    _bottomSpacing = ceilf(_bottomSpacing);
    _height = ceilf(_height);
    
    float yMin = 0.0;
    float yMax = YOffsetOfLine(LineCount()-1);
    if(!HasFlag(REStaff::HideRhythm))
    {
        for(unsigned int voiceIndex=FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
        {
            const BeamCacheVector& beamVector = _beams[voiceIndex];
            for(unsigned int i=0; i<beamVector.size(); ++i)
            {
                const BeamCache& beam = beamVector[i];
                if(beam.flags & REStaff::BeamIsRest) continue;
                float yMinLine = YOffsetOfLine(beam.minLine);
                float yMaxLine = YOffsetOfLine(beam.maxLine);
                
                if(beam.yStem < yMin) {yMin = beam.yStem;}
                if(yMinLine < yMin) {yMin = yMinLine;}
                if(yMaxLine < yMin) {yMin = yMaxLine;}
                if(beam.yStem > yMax) {yMax = beam.yStem;}
                if(yMinLine > yMax) {yMax = yMinLine;}
                if(yMaxLine > yMax) {yMax = yMaxLine;}
            }
        }
    }
    else {
        if(isTablature) {
            yMin -= 7.0;
            yMax += 7.0;
        }
    }
    
    _yMinBeaming = yMin;
    _yMaxBeaming = yMax;
    _tupletLineYOffset[0] = 0.0;
    _tupletLineYOffset[1] = 0.0;
    _accentYOffset = 0.0;
    _vibratoYOffset = 0.0;
    _tappingYOffset = 0.0;
    _strummingYOffset = 0.0;
    _stickingYOffset = 0.0;
    _dynamicsYOffset = 0.0;
    _ottaviaYOffset = 0.0;
    _textAboveYOffset = 0.0;
    _textBelowYOffset = 0.0;
    _palmMuteAndLetRingYOffset = 0.0;
    _chordDiagramYOffset = 0.0;
    
    float deltaChordDiagramYOffset = 0;
    float deltaTupletLineYOffset[2] = {0,0};
    float deltaAccentYOffset = 0;
    float deltaVibratoYOffset = 0;
    float deltaStickingYOffset = 0;    
    float deltaStrummingYOffset = 0;
    float deltaTappingYOffset = 0;
    float deltaDynamicsYOffset = 0;
    float deltaOttaviaYOffset = 0;
    float deltaTextAboveYOffset = 0;
    float deltaTextBelowYOffset = 0;
    float deltaPalmMuteYOffset = 0;
    
    Reflow::DynamicsType lastDynamics[REFLOW_MAX_VOICES];
    for(int i =0; i<REFLOW_MAX_VOICES; ++i) lastDynamics[i] = Reflow::DynamicsUndefined;
    
    // Determine Spacing
    for(unsigned int i=0; i<_parent->SystemBarCount(); ++i)
    {
        const RESlice* sbar = _parent->SystemBar(i);
        unsigned int barIndex = sbar->BarIndex();
        
        for(unsigned int voiceIndex=FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
        {
            RELocator locator(song, barIndex, _track->Index(), voiceIndex);
            const REPhrase* phrase = locator.Phrase();
            if(phrase->IsEmpty()) continue;
            
            // Ottavia Modifier
            if(voiceIndex == FirstVoiceIndex() && phrase->OttaviaModifier().ItemCount() > 0) {
                SetFlag(REStaff::OttaviaBand);
                deltaOttaviaYOffset = -6.0;
            }
            
            // Chord Diagram on Voice 0
            if(voiceIndex == FirstVoiceIndex() && phrase->ChordDiagramCount() > 0)
            {
                if(isStandard || (isTablature && !systemHasStandard))
                {
                    SetFlag(REStaff::ChordDiagramBand);
                    deltaChordDiagramYOffset = -(ChordDiagramHeight() + 6.0);
                }
            }
            
            // Chord Flags
            const REChord* chord = locator.Chord();
            while(chord) 
            {
                // Tuplet
                if(chord->HasFlag(REChord::TupletGrouping))
                {
                    if(voiceIndex == HighVoiceIndex()) {
                        SetFlag(REStaff::HighVoiceTupletBand);
                        deltaTupletLineYOffset[0] = -12.0;
                    }
                    else if (voiceIndex == LowVoiceIndex()) {
                        SetFlag(REStaff::LowVoiceTupletBand);
                        deltaTupletLineYOffset[1] = 12.0;
                    }
                }
                
                // Accent on voice 0
                if(!ShouldDrawAccentsWithNoteHead())
                {
                    if(voiceIndex == 0 && (chord->HasFlag(REChord::Accent) || chord->HasFlag(REChord::StrongAccent)))
                    {
                        deltaAccentYOffset = -5.0;
                        SetFlag(REStaff::AccentBand);
                    }
                }
                
                // Sticking ?
                bool hasLeftStick = false;
                bool hasRightStick = false;
                unsigned int nbNotes = chord->NoteCount();
                for(unsigned int noteIndex=0; noteIndex < nbNotes; ++noteIndex)
                {
                    const RENote* note = chord->Note(noteIndex);
                    if(note->HasFlag(RENote::LeftStick)) {hasLeftStick = true;}
                    if(note->HasFlag(RENote::RightStick)) {hasRightStick = true;}
                }
                
                if(hasLeftStick && hasRightStick) {
                    deltaStickingYOffset = -16.0;
                }
                else if(hasLeftStick || hasRightStick) {
                    if(deltaStickingYOffset > -8.0) {
                        deltaStickingYOffset = -8.0;
                    }
                }
                if(hasLeftStick || hasRightStick) {
                    SetFlag(REStaff::StickingBand);
                }
                
                // Pickstroke
                if(chord->HasFlag(REChord::PickStroke)) {
                    SetFlag(REStaff::StrummingBand);
                    deltaStrummingYOffset = -10.0;
                }
                
                // Vibrato
                if(chord->HasFlag(REChord::Vibrato)) {
                    SetFlag(REStaff::VibratoBand);
                    deltaVibratoYOffset = -6.0;
                }
                
                // PM
                if(chord->HasFlag(REChord::PalmMute)) {
                    SetFlag(REStaff::PalmMuteBand);
                    deltaPalmMuteYOffset = -10.0;
                }

                // Let ring
                if(chord->HasFlag(REChord::LetRing)) {
                    SetFlag(REStaff::LetRingBand);
                    deltaPalmMuteYOffset = -10.0;
                }
                
                // TSP
                if(chord->HasFlag(REChord::Tap) || chord->HasFlag(REChord::Slap) || chord->HasFlag(REChord::Pop)) {
                    SetFlag(REStaff::TappingBand);
                    deltaTappingYOffset = -13.0;
                }
                
                // Text above or below standard staff
                if(chord->HasTextAttached()) {
                    if(isStandard && chord->TextPositioning() == Reflow::TextAboveStandardStaff) {
                        SetFlag(REStaff::TextAboveStaffBand);
                        deltaTextAboveYOffset = -12.0;
                    }
                    else if(isStandard && chord->TextPositioning() == Reflow::TextBelowStandardStaff) {
                        SetFlag(REStaff::TextBelowStaffBand);
                        deltaTextBelowYOffset = 12.0;
                    }
                    else if(isTablature && chord->TextPositioning() == Reflow::TextAboveTablatureStaff) {
                        SetFlag(REStaff::TextAboveStaffBand);
                        deltaTextAboveYOffset = -12.0;
                    }
                    else if(isTablature && chord->TextPositioning() == Reflow::TextBelowTablatureStaff) {
                        SetFlag(REStaff::TextBelowStaffBand);
                        deltaTextBelowYOffset = 12.0;
                    }
                }
                
                // Dynamics Change
                if(chord->Dynamics() != Reflow::DynamicsUndefined &&
                   !hideDynamics &&
                   (isStandard || (isTablature && !systemHasStandard)))
                {
                    SetFlag(REStaff::DynamicsBand);
                    deltaDynamicsYOffset = 12.0;
                }
                    
                
                chord = locator.NextChord();
            }
        }
    }
    
    // Tuplet Hi Line
    yMin += deltaTupletLineYOffset[0];
    _tupletLineYOffset[0] = yMin;
    
    // Accents
    yMin += deltaAccentYOffset;
    _accentYOffset = yMin;
    
    // Tapping
    yMin += deltaTappingYOffset;
    _tappingYOffset = yMin;
    
    // Strumming
    yMin += deltaStrummingYOffset;
    _strummingYOffset = yMin;
    
    // Sticking
    yMin += deltaStickingYOffset;
    _stickingYOffset = yMin;
    
    // Palm Mute / Let Ring
    yMin += deltaPalmMuteYOffset;
    _palmMuteAndLetRingYOffset = yMin;
    
    // Vibrato
    yMin += deltaVibratoYOffset;
    _vibratoYOffset = yMin;
    
    // Ottavia
    yMin += deltaOttaviaYOffset;
    _ottaviaYOffset = yMin;
    
    // Text above standard staff
    yMin += deltaTextAboveYOffset;
    _textAboveYOffset = yMin;
    
    // Chord Diagrams
    yMin += deltaChordDiagramYOffset;
    _chordDiagramYOffset = yMin;
    
    // Extra space
    yMin -= 5.0;
    
    // Tuplet Low Line
    yMax += 1.0;
    _tupletLineYOffset[1] = yMax;
    yMax += deltaTupletLineYOffset[1];
    
    // Dynamics
    yMax += 0.60 * deltaDynamicsYOffset;
    _dynamicsYOffset = yMax;
    yMax += 0.40 * deltaDynamicsYOffset;
    
    // Text below standard staff
    _textBelowYOffset = yMax;
    yMax += deltaTextBelowYOffset;
    
    // Extra space
    yMax += 5.0;
    
    // Slurs
    for(const RESlur* pslur : _slurs)
    {
        yMin = std::min<float>(yMin, pslur->MinY());
        yMax = std::max<float>(yMax, pslur->MaxY());
    }
    
    if(yMin < 0.0) {
        _topSpacing = -yMin;
    }
    if(yMax > _height) {
        _bottomSpacing = yMax - _height;
    }
    
    if(_topSpacing < 18.0) _topSpacing = 18.0;
    if(_bottomSpacing < 12.0) _bottomSpacing = 12.0;
    
    _topSpacing = ceilf(_topSpacing);
    _bottomSpacing = ceilf(_bottomSpacing);
    _height = ceilf(_height);
}

void REStaff::_CloseCurrentBeam(REPainter& painter, float xStart, float yStart, float xEnd, float yEnd, float xOffset, float yOffset, float h)
{
    //painter.StrokeLine(REPoint(xStart+xOffset, yStart+yOffset), REPoint(xEnd+xOffset, yEnd+yOffset));
    
    float x0 = xStart + xOffset;
    float y0 = yStart + yOffset;
    float x1 = xEnd + xOffset;
    float y1 = yEnd + yOffset;
    
    REPoint points[4] = {REPoint(x0, y0), REPoint(x1, y1), REPoint(x1, y1+h), REPoint(x0, y0+h)};
    painter.FillQuad(points);
}

int REStaff::_CalculateInterchordBeamType(const REChord* c0, const REChord* c1, int noteValue)
{
    //printf("   [Interchord Type between %d (%d) and %d (%d) for noteValue %d\n", c0->Index(), c0->NoteValue(), c1->Index(), c1->NoteValue(), noteValue);
    
    bool c0_ok = (c0->NoteValue() >= noteValue);
    bool c1_ok = (c1->NoteValue() >= noteValue);
    
    if(c0_ok && c1_ok) {
        return 3;
    }
    
    else if(c0_ok && !c1_ok) 
    {
        if(!c0->HasFlag(REChord::BeamingStart)) {
            const REChord* xc0 = c0->PreviousSibling();
            if(xc0 && xc0->NoteValue() >= noteValue) {
                return 0;
            }
        }
        return 1;
    }
    
    else if(c1_ok && !c0_ok) 
    {
        if(!c1->HasFlag(REChord::BeamingEnd)) 
        {
            /*const REChord* xc1 = c1->NextSibling();
            if(xc1 && xc1->NoteValue() >= noteValue) {
                return 0;
            }*/
            return 0;
        }
        
        return 2;
    }
    
    return 0;
}

void REStaff::_DrawStem(REPainter& painter, const RESlice* slice, const REChord* chord, const BeamCache* beam) const
{
    float x = beam->x;
    if(beam->flags & REStaff::BeamIsStemDown) {
        float yBase = YOffsetOfLine(beam->minLine);
        painter.StrokeVerticalLine(x, beam->yStem, yBase);
    }
    else {
        float yBase = YOffsetOfLine(beam->maxLine);
        painter.StrokeVerticalLine(x, beam->yStem, yBase);
    }
}

void REStaff::_DrawBeaming(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const
{
    unsigned int voiceIndex = chord->Phrase()->Voice()->Index();
    const REChord* lastChord = chord->NextSibling();
    while(lastChord && !lastChord->HasFlag(REChord::BeamingEnd)) {
        lastChord = lastChord->NextSibling();
    }
    
    if(lastChord == 0) return;
    
    unsigned long tick0 = chord->OffsetInTicks();
    unsigned long tick1 = lastChord->OffsetInTicks();
    
    float x0, x1, y0, y1;
    
    const BeamCache* firstBeam = BeamCacheForChord(chord->Index(), voiceIndex, sbar->Index());
    const BeamCache* lastBeam = BeamCacheForChord(lastChord->Index(), voiceIndex, sbar->Index());
    x0 = firstBeam->x;
    x1 = lastBeam->x;
    y0 = firstBeam->yStem;
    y1 = lastBeam->yStem;
    
    
    //painter.SetStrokeColor(REColor(1.0, 0.0, 0.0, 0.7));
    painter.SetStrokeColor(REColor(0.0, 0.0, 0.0, 1.0));
    
    // First Beam
    float h = (0.5 * UnitSpacing());    
    REPoint points[4] = {REPoint(x0, y0), REPoint(x1, y1), REPoint(x1, y1+h), REPoint(x0, y0+h)};
    painter.FillQuad(points);
    
    // First Stem
    {
        const BeamCache* beam = firstBeam;
        _DrawStem(painter, sbar, chord, beam);
    }
    
    // TEMP: Stems
    for(const REChord* ch = chord->NextSibling(); ch != lastChord; ch = ch->NextSibling())
    {
        if(ch->IsRest()) continue;
        
        const BeamCache* beam = BeamCacheForChord(ch->Index(), voiceIndex, sbar->Index());
        _DrawStem(painter, sbar, chord, beam);
    }
    
    // Last Stem
    {
        const BeamCache* beam = lastBeam;
        _DrawStem(painter, sbar, chord, beam);
    }
    
    // Draw secondary beams
    for(int noteValue = Reflow::SixteenthNote; noteValue <= Reflow::SixtyFourthNote; ++noteValue)
    {
        float yOffset = (fabsf(h)+1) * (noteValue - Reflow::EighthNote);
        float xOffset = 0.0;
        
        const REChord* cA = chord;
        const REChord* cB = cA->NextSibling();
        if(firstBeam->flags & REStaff::BeamIsStemDown/*orientation == 1*/) {
            yOffset = -yOffset;
        }
        
        bool beamPending = false;
        float xStart, yStart, xEnd, yEnd;
        
        while(true)
        {
            float xA = BeamCacheForChord(cA->Index(), voiceIndex, sbar->Index())->x;
            float xB = BeamCacheForChord(cB->Index(), voiceIndex, sbar->Index())->x;   
            float tA = (xA - x0) / (x1 - x0);
            float tB = (xB - x0) / (x1 - x0);
            float yA = y0 + tA * (y1-y0);
            float yB = y0 + tB * (y1-y0);
            int interchordBeamType = _CalculateInterchordBeamType(cA, cB, noteValue);
            
            // Full Beam between cA and cB
            if(interchordBeamType == 3) 
            {
                if(beamPending) {
                    xEnd = xB;
                    yEnd = yB;
                }
                else {
                    xStart = xA;
                    yStart = yA;
                    xEnd = xB;
                    yEnd = yB;
                    beamPending = true;
                }
            }
            
            // Left Beam between cA and cB
            else if (interchordBeamType == 1) 
            {
                if(!beamPending) {
                    xStart = xA;
                    yStart = yA;    
                }
                
                float w = (xB - xA)/2;
                if(w > UnitSpacing()) w = UnitSpacing();
                float ratio = w / (xB - xA);
                float yh = ratio * (yB - yA);
                
                xEnd = xA + w;
//                xEnd = ((xB - xA) > UnitSpacing() ? xA + UnitSpacing() : 0.5*(xB+xA));
                yEnd = yA + yh;//0.5*(yB+yA);
                
                _CloseCurrentBeam(painter, xStart, yStart, xEnd, yEnd, xOffset, yOffset, h);
                beamPending = false;
            }
            
            // Right Beam between cA and cB
            else if (interchordBeamType == 2) 
            {
                if(beamPending) {
                    _CloseCurrentBeam(painter, xStart, yStart, xEnd, yEnd, xOffset, yOffset, h);
                    beamPending = false;
                }
                
                float w = (xB - xA)/2;
                if(w > UnitSpacing()) w = UnitSpacing();
                float ratio = w / (xB - xA);
                float yh = ratio * (yB - yA);
                xStart = xB - w;
                yStart = yB - yh;
                xEnd = xB;
                yEnd = yB;
                beamPending = true;
            }
            
            // No Beam at all between cA and cB
            else 
            {  
                if(beamPending) {
                    _CloseCurrentBeam(painter, xStart, yStart, xEnd, yEnd, xOffset, yOffset, h);
                    beamPending = false;
                }
            }
            
            if(cB->HasFlag(REChord::BeamingEnd)) {break;}
            
            cA = cB;
            cB = cB->NextSibling();
            if(cB == NULL) {break;}
        }
        
        if(beamPending) {
            _CloseCurrentBeam(painter, xStart, yStart, xEnd, yEnd, xOffset, yOffset, h);
            beamPending = false;
        }
    }
    
    painter.SetStrokeColor(REColor(0.0, 0.0, 0.0, 1.0));
}

void REStaff::DrawTimeSignature(REPainter& painter, const RETimeSignature& timeSignature, const REPoint& pt, float fontSize)
{
	if(timeSignature.numerator < 10)
	{
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.numerator), pt.x, pt.y - fontSize, fontSize);
	}
	else {
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.numerator / 10), pt.x-5.0, pt.y - fontSize, fontSize);
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.numerator % 10), pt.x+5.0, pt.y - fontSize, fontSize);
	}
	if(timeSignature.denominator < 10)
	{
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.denominator), pt.x, pt.y+fontSize+1.0, fontSize);
	}
	else {
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.denominator / 10), pt.x-5.0, pt.y+fontSize+1.0, fontSize);
        painter.DrawMusicSymbol(Reflow::NameOfNumberGlyph(timeSignature.denominator % 10), pt.x+5.0, pt.y+fontSize+1.0, fontSize);
	}
}

void REStaff::DrawTimeSignature(REPainter& painter, const RETimeSignature& timeSignature, const REPoint& pt) const
{
    painter.SetStrokeColor(REColor(0,0,0));
    painter.SetFillColor(REColor(0,0,0));
	float fontSize = UnitSpacing();
    REStaff::DrawTimeSignature(painter, timeSignature, pt, fontSize);
}

void REStaff::DrawBeamingAndRestsOfSlice(REPainter& painter, int sliceIndex) const
{
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    
    const RESlice* sbar = _parent->SystemBar(sliceIndex);        
    if(sbar->IsMultiRest()) return;
    
    unsigned int barIndex = sbar->BarIndex();
    
    // TODO: We can only render two lines of rests
    unsigned int voiceCount = _track->VoiceCountOfBarInGrandStaff(barIndex, FirstVoiceIndex() == 0 ? 0 : 1);
    if(voiceCount > 2) voiceCount = 2;
    
    for(unsigned int voiceIndex = FirstVoiceIndex(); voiceIndex < FirstVoiceIndex() + voiceCount; ++voiceIndex)
    {
        RELocator locator(song, barIndex, _track->Index(), voiceIndex);
        const REPhrase* phrase = locator.Phrase();
        if(phrase->IsEmpty() && voiceIndex > FirstVoiceIndex()) continue;
        
        float yRest = (voiceCount == 2 ? (voiceIndex == 0 ? 0.25 : 0.75) : 0.50) * Height();
        
        bool inBeaming = false;
        const REChord* chord = locator.Chord();
        while(chord)
        {
            // Rest
            if(chord->IsRest()) {
                unsigned long tick = chord->OffsetInTicks();
                float x = sbar->XOffsetOfTick(tick) /*+ sbar->XOffset()*/;
                float y = yRest;
                
                const char* restSymbol = "rest";
                switch(chord->NoteValue())
                {
                    case Reflow::WholeNote: restSymbol       = "pause";
                        if(Type() == Reflow::StandardStaff) y -= 0.5 * UnitSpacing();
                        break;
                    case Reflow::HalfNote: restSymbol        = "halfpause";
                        if(Type() == Reflow::StandardStaff) {
                            y -= 0.5 * UnitSpacing();
                            y += 0.5;
                        }
                        break;
                    case Reflow::QuarterNote: restSymbol     = "rest"; break;
                    case Reflow::EighthNote: restSymbol      = "rest8"; break;
                    case Reflow::SixteenthNote: restSymbol   = "rest16"; break;
                    case Reflow::ThirtySecondNote:restSymbol = "rest32"; break;
                    case Reflow::SixtyFourthNote: restSymbol = "rest64"; break;
                }
                painter.DrawMusicSymbol(restSymbol, x, y, 0.80 * UnitSpacing());
            }
            
            // Beaming Start ?
            if(chord->HasFlag(REChord::BeamingStart))
            {
                float y = roundf(voiceIndex == LowVoiceIndex() ? -12.0 : Height() + 12.0);
                _DrawBeaming(painter, sbar, chord, y, voiceIndex-FirstVoiceIndex());
                inBeaming = true;
            }
            else if(chord->HasFlag(REChord::BeamingEnd) && inBeaming) {
                inBeaming = false;
            }
            else if(!inBeaming) {
                float y = (voiceIndex == LowVoiceIndex() ? -12.0 : Height() + 12.0);
                if(!chord->IsRest()) {
                    _DrawSingleStem(painter, sbar, chord, y, voiceIndex-FirstVoiceIndex());
                }
            }
            
            // Tuplet Grouping ?
            if(chord->HasFlag(REChord::TupletGroupStart))
            {
                float y = roundf(voiceIndex == LowVoiceIndex() ? YOffsetOfTupletLine(1) : YOffsetOfTupletLine(0));
                _DrawTupletGroup(painter, sbar, chord, y, voiceIndex - FirstVoiceIndex());
            }
            
            if(Type() == Reflow::TablatureStaff)
            {
                // Dots ?
                if(chord->Dots() == 1)
                {
                    unsigned long tick = chord->OffsetInTicks();
                    float x = sbar->XOffsetOfTick(tick);
                    float y = roundf(voiceIndex == HighVoiceIndex() ? -12.0 : Height() + 12.0);
                    painter.DrawMusicSymbol("dot", x + 0.25 * UnitSpacing(), y, 0.75 * UnitSpacing());
                }
                else if(chord->Dots() == 2)
                {
                    unsigned long tick = chord->OffsetInTicks();
                    float x = sbar->XOffsetOfTick(tick);
                    float y = roundf(voiceIndex == HighVoiceIndex() ? -12.0 : Height() + 12.0);
                    painter.DrawMusicSymbol("dot", x + 0.25 * UnitSpacing(), y, 0.75 * UnitSpacing());
                    painter.DrawMusicSymbol("dot", x + UnitSpacing(), y, 0.75 * UnitSpacing());
                }
            }
            
            chord = locator.NextChord();
        }
    }

}

//---------------------------------------------------------------------------------------
void REStaff::_DrawTupletGroup(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const
{
    RETuplet tuplet = chord->Tuplet();
    const REChord* lastChord = chord->FindNextWithFlag(REChord::TupletGroupEnd);
    if(lastChord == NULL) {
        lastChord = chord;
    }
    
    y = roundf(YOffsetOfTupletLine(orientation));
    float dy;
    if(orientation == 0) {
        y += 4.0;
        dy = 3.0;
    }
    else {
        y += 5.0;
        dy = -3.0;
    }
    
    painter.SetFillColor(REColor(0,0,0));
    painter.SetStrokeColor(REColor(0,0,0));
    
    float unitSpacing = UnitSpacing();
    float x0 = roundf(sbar->XOffsetOfTick(chord->OffsetInTicks()) - unitSpacing/2) + 0.5;
    float x1 = roundf(sbar->XOffsetOfTick(lastChord->OffsetInTicks()) + unitSpacing/2) - 0.5;
    float xmid = (x0+x1)/2;
    float xmid0 = xmid - 5.0;
    float xmid1 = xmid + 5.0;
    
    painter.PathBegin();
    {
        painter.PathMoveToPoint(xmid0, y + 0.5);
        painter.PathLineToPoint(x0, y + 0.5);
        painter.PathLineToPoint(x0, y + 0.5 + dy);
        
        painter.PathMoveToPoint(xmid1, y + 0.5);
        painter.PathLineToPoint(x1, y + 0.5);
        painter.PathLineToPoint(x1, y + 0.5 + dy);
    }
    painter.PathStroke();
    
    // Draw Tuplet
    std::ostringstream oss; oss << (int)tuplet.tuplet;
    painter.DrawText(oss.str(), REPoint(xmid-2.5, y-6.0), "Arial", REPainter::Italic, 9.0, REColor(0,0,0));
}

void REStaff::DrawBeamingAndRests(REPainter& painter) const
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    const RERange& barRange = _parent->BarRange();
    
    painter.SetFillColor(REColor(0,0,0));
    painter.SetStrokeColor(REColor(0,0,0));
    
    for(unsigned int i=0; i<barRange.count; ++i)
    {
        const RESlice* sbar = _parent->SystemBar(i);        
        unsigned int barIndex = sbar->BarIndex();
        
        float dx = sbar->XOffset();
        painter.Translate(dx, 0);
        DrawBeamingAndRestsOfSlice(painter, i);
        painter.Translate(-dx, 0);
    }
}

const REStaff::BeamCache* REStaff::BeamCacheForChord(unsigned int chordIndex, unsigned int voiceIndex, unsigned int sbarIndex) const
{
    const REStaff::BeamCacheVector& beamVector = _beams[voiceIndex];
    for(unsigned int i=0; i<beamVector.size(); ++i)
    {
        const REStaff::BeamCache* beam = &beamVector[i];
        if(beam->sbarIndex == sbarIndex && beam->chordIndex == chordIndex) {
            return beam;
        }
    }
    return 0;
}


#define BEAM_SEXY_HEIGHT (22.0)


void REStaff::_ClearBeamCache()
{
    for(unsigned int i=0; i<REFLOW_MAX_VOICES; ++i) {
        _beams[i].clear();
    }
}

void REStaff::_CalculateBeamCache()
{
    const RESystem* system = _parent;
    const REScore* score = system->Score();
    const RESong* song = score->Song();
    bool transposed = !score->Settings().InConcertTone();
    unsigned int sbarCount = _parent->SystemBarCount();
    
    _ClearBeamCache();
    
    for(unsigned int voiceIndex=FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        BeamCacheVector& beamVector = _beams[voiceIndex];
        for(unsigned int sbarIndex=0; sbarIndex < sbarCount; ++sbarIndex)
        {
            RESlice* sbar = _parent->SystemBar(sbarIndex);
            int barIndex = sbar->BarIndex();
            const REPhrase* phrase = _track->Voice(voiceIndex)->Phrase(barIndex);
            bool inBeaming = false;
            bool firstBeamDown = false;
            for(unsigned int chordIndex=0; chordIndex < phrase->ChordCount(); ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                unsigned long tick = chord->OffsetInTicks();
                int minLine=127, maxLine=-127;
                if(Type() == Reflow::StandardStaff) {
                    chord->CalculateLineRange(transposed, &minLine, &maxLine);
                }
                else {
                    chord->CalculateStringRange(&minLine, &maxLine);
                }

                BeamCache beam;
                beam.sbarIndex = sbarIndex;
                beam.chordIndex = chordIndex;
                beam.minLine = minLine;
                beam.maxLine = maxLine;
                beam.flags = 0;
                if(chord->IsRest()) {beam.flags |= REStaff::BeamIsRest;}
                
                if(!inBeaming)
                {
                    if(chord->IsStemDirectionAutomatic())
                    {
                        if(voiceIndex == LowVoiceIndex()) {beam.flags |= REStaff::BeamIsStemDown;}
                    }
                    else {
                        if(chord->HasFlag(REChord::ForceStemDown)) {beam.flags |= REStaff::BeamIsStemDown;}
                    }
                    
                    if(chord->HasFlag(REChord::BeamingStart)) {
                        inBeaming = true;
                        firstBeamDown = (beam.flags & REStaff::BeamIsStemDown);
                    }
                }
                else {
                    if(firstBeamDown) beam.flags |= REStaff::BeamIsStemDown;
                    if(chord->HasFlag(REChord::BeamingEnd)) {
                        inBeaming = false;
                    }
                }
                
                float yMaxLine = YOffsetOfLine(beam.maxLine);
                float yMinLine = YOffsetOfLine(beam.minLine);
                
                beam.x = sbar->XOffsetOfTick(tick) /*+ sbar->XOffset()*/;
                
                //std::cout << "Bar " << barIndex << " - Chord " << chordIndex << " Tick: " << tick << " -> x: " << beam.x;
                
                if(Type() == Reflow::StandardStaff)
                {
                    if(beam.flags & REStaff::BeamIsStemDown) {
                        bool stackedSeconds = chord->HasVerticallyStackedSeconds(false, transposed);
                        if(!stackedSeconds) beam.x -= 0.5 * UnitSpacing();
                        beam.yStem = yMaxLine + BEAM_SEXY_HEIGHT;
                    }
                    else {
                        bool stackedSeconds = chord->HasVerticallyStackedSeconds(true, transposed);
                        if(!stackedSeconds) beam.x += 0.5 * UnitSpacing();
                        beam.yStem = yMinLine - BEAM_SEXY_HEIGHT;
                    }
                }
                else {
                    beam.x = roundf(beam.x) - 0.5;
                }
                
                //std::cout << "; yStem: " << beam.yStem << std::endl;
                beamVector.push_back(beam);
            }
        }
    }
    
    // Calculate Beaming coordinates here
    for(unsigned int voiceIndex=FirstVoiceIndex(); voiceIndex <= LastVoiceIndex(); ++voiceIndex)
    {
        BeamCacheVector& beamVector = _beams[voiceIndex];
        int beamIndex = 0;
        for(unsigned int sbarIndex=0; sbarIndex < sbarCount; ++sbarIndex)
        {
            RESlice* sbar = _parent->SystemBar(sbarIndex);
            int barIndex = sbar->BarIndex();
            const REPhrase* phrase = _track->Voice(voiceIndex)->Phrase(barIndex);
            for(unsigned int chordIndex=0; chordIndex < phrase->ChordCount(); ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                if(chord->HasFlag(REChord::BeamingStart)) 
                {
                    const REChord* lastChord = chord->FindNextWithFlag(REChord::BeamingEnd);
                    if(lastChord != NULL) {
                        BeamCache* cache = beamVector.data() + beamIndex;
                        CalculateBeamingCoordinates(chord, lastChord, cache);
                    }
                }
                beamIndex++;
            }
        }
    }
}

void REStaff::CalculateBeamingCoordinates(const REChord* firstChord, const REChord* lastChord, BeamCache* beamCache)
{
    int firstChordIndex = firstChord->Index();
    int lastChordIndex = lastChord->Index();
    int nbChords = lastChordIndex - firstChordIndex + 1;
    BeamCache* firstBeam = &beamCache[0];
    BeamCache* lastBeam = &beamCache[nbChords-1];
    bool beamStemDown = (0 != (firstBeam->flags & REStaff::BeamIsStemDown));
    bool forceHorizontal = false;
    
    float x0 = firstBeam->x;
    float x1 = lastBeam->x;
    float y0 = firstBeam->yStem;
    float y1 = lastBeam->yStem;
    
    // Passe #1: Remonter les y0 et y1 pour que les notes au milieu aient la place necessaire
    // --------------------------------------------------------------------------------------
    for(int i=1; i<=nbChords-2; ++i)
    {
        BeamCache* beam = &beamCache[i];
        float dx = (beam->x - x0) / (x1-x0);
		float y = (1.0-dx)*y0 + dx*y1;
		if (!beamStemDown && beam->yStem < y) {
			float dy = y - beam->yStem;
			y0 -= dy;
			y1 -= dy;
		}
		else if(beamStemDown && beam->yStem > y) {
			float dy = beam->yStem - y;
			y0 += dy;
			y1 += dy;
		}
    }
    if(forceHorizontal) {
		if(beamStemDown) {
			if(y0 < y1) y0 = y1;
			if(y1 < y0) y1 = y0;
		}
		else {
			if(y0 > y1) y0 = y1;
			if(y1 > y0) y1 = y0;
		}
	}
	else {
		float beamMaxDeltaY = 10.0;
		if(beamStemDown) {
			if(y0 > y1 && (y0-y1 >= beamMaxDeltaY)) {
				y1 = y0 - beamMaxDeltaY;
			}
			if(y1 > y0 && (y1-y0 >= beamMaxDeltaY)) {
				y0 = y1 - beamMaxDeltaY;
			}
		}
		else {
			if(y0 < y1 && (y1-y0 >= beamMaxDeltaY)) {
				y1 = y0 + beamMaxDeltaY;
			}
			if(y1 < y0 && (y0-y1 >= beamMaxDeltaY)) {
				y0 = y1 + beamMaxDeltaY;
			}
		}
	}
    firstBeam->yStem = y0;
    lastBeam->yStem = y1;
    
    // Passe #2: Remonter les yStem de tous les elements entre 2 pour qu'ils soient a leur bonne hauteur
    // --------------------------------------------------------------------------------------
    for(int i=1; i<=nbChords-2; ++i)
    {
        BeamCache* beam = &beamCache[i];
        float dx = (beam->x - x0) / (x1-x0);
		float y = (1.0-dx)*y0 + dx*y1;
		beam->yStem = y;
    }
}


bool REStaff::FindCenterOfNote(const RENote* note, REPoint* outCenter) const
{
    const REChord* chord = note->Chord();
    const REPhrase* phrase = chord->Phrase();
    const RESlice* slice = _parent->SystemBarWithBarIndex(phrase->Index());
    if(slice == NULL) {
        return false;
    }
    
    // Retrieve X
    float x = slice->XOffsetOfTick(chord->OffsetInTicks()) + slice->XOffset();
    
    // And Y
    float y = 0;
    const REScore* score = _parent->Score();
    if(_type == Reflow::TablatureStaff) {
        y = YOffsetOfLine(note->String());
    }
    else if(_type == Reflow::StandardStaff) {
        const RENote::REStandardRep& representation = note->Representation(score->IsTransposing());
        y = YOffsetOfLine(representation.line) + YOffset();
    }
    
    *outCenter = REPoint(x,y);
    return true;
}

void REStaff::DrawSlur(REPainter& painter, const RESlur& slur) const
{
    int firstBarIndex = slur.StartBeat().bar;
    int lastBarIndex = slur.EndBeat().bar;
    int startTick = Reflow::TimeDivToTicks(slur.StartBeat().timeDiv);
    int endTick = Reflow::TimeDivToTicks(slur.EndBeat().timeDiv);
    
    const RESystem* system = _parent;
    if(!system) return;
    
    if(system->BarRange().IsInRange(firstBarIndex) && system->BarRange().IsInRange(lastBarIndex))
    {
        const RESlice* startSlice = system->SystemBarWithBarIndex(firstBarIndex);
        const RESlice* endSlice = system->SystemBarWithBarIndex(lastBarIndex);
        
        float x0 = startSlice->XOffset() + startSlice->XOffsetOfTick(startTick);
        float x1 = endSlice->XOffset() + endSlice->XOffsetOfTick(endTick);
        
        slur.Draw(painter, x0, x1, 0.0f, REColor::Black);
    }
    else
    {
        const REScore* score = _parent->Score();
        float cx = score->ContinuousXOffsetOfSystem(system);
        
        const RESystem* startSystem = score->SystemWithBarIndex(firstBarIndex);
        const RESystem* endSystem = score->SystemWithBarIndex(lastBarIndex);
        const RESlice* startSlice = (startSystem ? startSystem->SystemBarWithBarIndex(firstBarIndex) : nullptr);
        const RESlice* endSlice = (endSystem ? endSystem->SystemBarWithBarIndex(lastBarIndex) : nullptr);
        
        if(startSlice && endSlice)
        {
            float startCX = score->ContinuousXOffsetOfSystem(startSystem);
            float endCX = score->ContinuousXOffsetOfSystem(endSystem);
            float startDX = startCX - cx;
            float endDX = endCX - cx;
            
            float x0 = startDX + startSlice->XOffset() + startSlice->XOffsetOfTick(startTick);
            float x1 = endDX + endSlice->XOffset() + endSlice->XOffsetOfTick(endTick);
            
            slur.Draw(painter, x0, x1, 0.0f, REColor::Black);
        }
    }
}

void REStaff::_DrawOttaviaBandPartial(REPainter& painter, Reflow::OttaviaType ottavia, float startX, float endX, float y) const
{
    if(ottavia == Reflow::NoOttavia) {
        return;
    }
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    float yDashes = y + 8.5;
    REPoint pt (startX-9.0, y);
    std::string txt = Reflow::NameOfOttavia(ottavia);
    painter.DrawText(txt, pt, "Times New Roman", 0, 9.0, REColor::Black);
    
    if(startX != endX) 
    {
        if(!painter.IsForcedToBlack()) {painter.SetStrokeColor(REColor::Gray);}
        
        REReal dashPattern[] = {3.0, 2.0};
        painter.SetLineDash(dashPattern, 2, 0.0);
        painter.StrokeLine(REPoint(startX+12.0, yDashes), REPoint(endX, yDashes));
        
        painter.SetLineDash(NULL, 0, 0.0);
        painter.StrokeLine(REPoint(endX, yDashes-3.0), REPoint(endX, yDashes+3.0));
    }
}

void REStaff::_DrawBandPartial(REPainter& painter, int flag, float startX, float endX, float y) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor::Blue);
    painter.StrokeLine(REPoint(startX, y), REPoint(endX, y));
#endif*/
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    if(flag == REChord::Vibrato)
    {
        RERect bbox = painter.BoundingBoxOfMusicSymbol("vibrato", 6.0);
        float dy = bbox.HalfHeight();
        float x = startX;
        while(x <= endX) {
            painter.DrawMusicSymbol("vibrato", x, y+dy, 6.0);
            x += bbox.size.w;
        }
    }
    else if(flag == REChord::PalmMute || flag == REChord::LetRing)
    {
        float dy = -3;
        y += dy;
     	float yDashes = y + 8.5;
        float dx = 0;
        REPoint pt (startX-9.0, y);
        std::string txt = "";
        switch(flag) {
            case REChord::PalmMute: txt = "P.M."; dx = 12; break;
            case REChord::LetRing: txt = "let ring"; dx = 22; break;
        }
        painter.DrawText(txt, pt, "Times New Roman", 0, 10.0, REColor::Black);
        
        if(startX != endX) 
        {
            if(!painter.IsForcedToBlack()) {painter.SetStrokeColor(REColor::Gray);}
            
            REReal dashPattern[] = {3.0, 2.0};
            painter.SetLineDash(dashPattern, 2, 0.0);
            painter.StrokeLine(REPoint(startX + dx, yDashes), REPoint(endX, yDashes));
            
            painter.SetLineDash(NULL, 0, 0.0);
            painter.StrokeLine(REPoint(endX, yDashes-3.0), REPoint(endX, yDashes+3.0));
        }
    }
}

void REStaff::_DrawBand(REPainter& painter, int flag, float y, const RERange& sliceRange) const
{
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    //const RERange& barRange = _parent->BarRange();
	bool inBand = false;
	float startX = 0, endX = 0;
    
    int lastSliceIndex = sliceRange.index + sliceRange.count;
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        
        if(_track->IsBarEmptyOrRest(barIndex))
        {
            // Close current vibrato if pending
            if (inBand)
            {
                // Finish pm
				inBand = false;
				_DrawBandPartial(painter, flag, startX, endX, y);
            }
            
            // Next Slice
            continue;
        }
        
        bool hasFlag = false;
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        do 
        {
            int tick = it.Tick();
            hasFlag = it.HasAnyChordAtCurrentTickThisFlag(flag);
            
            if (inBand) 
            {
                if(!hasFlag) 
                {
                    inBand = false;
       				_DrawBandPartial(painter, flag, startX, endX, y);
                }
                else {
                    endX = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                }
            }
            else 
            {
                if(hasFlag) 
                {
                    inBand = true;
                    startX = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                    endX = startX;
                }
            }
            
        } while (it.Next());
    }
    
    // Pending band ?
	if (inBand) {
		inBand = false;
        _DrawBandPartial(painter, flag, startX, endX, y);
	}
}

void REStaff::DrawBrush(REPainter& painter, float x, float y0, float y1, bool down) const
{
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    painter.StrokeLine(REPoint(x,y0), REPoint(x,y1));

    REPoint ap0, ap1, ap2;

    if(down) {
        // Arrow goes up
        ap0 = REPoint(x, y0);
        ap1 = REPoint(ap0.x + 2.5, y0 + 5.5);
        ap2 = REPoint(ap0.x - 2.5, y0 + 5.5);
    }
    else {
        // Arrow goes down
        ap0 = REPoint(x, y1);
        ap1 = REPoint(ap0.x + 2.5, y1 - 5.5);
        ap2 = REPoint(ap0.x - 2.5, y1 - 5.5);
    }
    
    painter.PathBegin();
    {
        painter.PathMoveToPoint(ap0);
        painter.PathLineToPoint(ap1);
        painter.PathLineToPoint(ap2);
        painter.PathLineToPoint(ap0);
    }
    painter.PathClose();
    painter.PathFill();
}

void REStaff::DrawArpeggio(REPainter& painter, float x, float y0, float y1, bool down) const
{
    RERect bbox = painter.BoundingBoxOfMusicSymbol("arpeggio", UnitSpacing());
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    for(float y=y0+14.0; y<=y1; y+=bbox.size.h) {
        REPoint pt = REPoint(x-1, y);
        painter.DrawMusicSymbol("arpeggio", pt, UnitSpacing());
    }

    REPoint ap0, ap1, ap2;
    
    if(down) {
        // Arrow goes up
        ap0 = REPoint(x, y0);
        ap1 = REPoint(ap0.x + 2.5, y0 + 5.5);
        ap2 = REPoint(ap0.x - 2.5, y0 + 5.5);
    }
    else {
        // Arrow goes down
        ap0 = REPoint(x, y1);
        ap1 = REPoint(ap0.x + 2.5, y1 - 5.5);
        ap2 = REPoint(ap0.x - 2.5, y1 - 5.5);
    }
    
    painter.PathBegin();
    {
        painter.PathMoveToPoint(ap0);
        painter.PathLineToPoint(ap1);
        painter.PathLineToPoint(ap2);
        painter.PathLineToPoint(ap0);
    }
    painter.PathClose();
    painter.PathFill();
}

void REStaff::DrawPickstrokeBand(REPainter& painter, float y, const RERange& sliceRange) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor(1.0, 1.0, 0.25));
    painter.StrokeLine(REPoint(0, y), REPoint(_parent->Width(), y));
#endif*/
    
    const float dy = 2.0;
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);

    int lastSliceIndex = sliceRange.index + sliceRange.count;    
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        if(!it.IsValid()) continue;
        
        do {
            int tick = it.Tick();
            for(int i=FirstVoiceIndex(); i<=LastVoiceIndex(); ++i) {
                const REChord* chord = it.Chord(i);
                if(chord && chord->HasFlag(REChord::PickStroke)) {
                    float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                    if(chord->HasFlag(REChord::StrumUpwards)) 
                    {
                        // Up Stroke
                        painter.DrawMusicSymbol("upstroke", x-UnitSpacing()/2, y + dy, UnitSpacing());
                    }
                    else 
                    {
                        // Down Stroke
                        painter.DrawMusicSymbol("downstroke", x-UnitSpacing()/2, y + dy, UnitSpacing());
                    }
                }
            }
            
        } while(it.Next());
    }
}

void REStaff::DrawTSPBand(REPainter& painter, float y, const RERange& sliceRange) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor(1.0, 1.0, 0.25));
    painter.StrokeLine(REPoint(0, y), REPoint(_parent->Width(), y));
#endif*/
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    painter.BeginTextBatched("Times New Roman", 0, 10.0, REColor::Black);
    const float dy = -2;

    int lastSliceIndex = sliceRange.index + sliceRange.count;    
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        if(!it.IsValid()) continue;
        
        do {
            int tick = it.Tick();
            for(int i=FirstVoiceIndex(); i<=LastVoiceIndex(); ++i) {
                const REChord* chord = it.Chord(i);
                if(!chord) continue;
                
                if(chord->HasFlag(REChord::Tap)) {
                    float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick)) - 3;
                    painter.DrawTextBatched("T", REPoint(x,y+dy));
                }
                else if(chord->HasFlag(REChord::Slap)) {
                    float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick)) - 3;
                    painter.DrawTextBatched("S", REPoint(x,y+dy));
                }
                else if(chord->HasFlag(REChord::Pop)) {
                    float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick)) - 3;
                    painter.DrawTextBatched("P", REPoint(x,y+dy));
                }
            }
            
        } while(it.Next());
    }
    
    painter.EndTextBatched();
}

void REStaff::DrawVibratoBand(REPainter& painter, float y, const RERange& sliceRange) const
{
    _DrawBand(painter, REChord::Vibrato, y, sliceRange);
}

void REStaff::DrawPalmMuteBand(REPainter& painter, float y, const RERange& sliceRange) const
{
    _DrawBand(painter, REChord::PalmMute, y, sliceRange);
}

void REStaff::DrawLetRingBand(REPainter& painter, float y, const RERange& sliceRange) const
{
    _DrawBand(painter, REChord::LetRing, y, sliceRange);
}

void REStaff::DrawOttaviaBand(REPainter& painter, Reflow::OttaviaType ottavia, float y, const RERange& sliceRange) const
{
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    const RERange& barRange = _parent->BarRange();
	bool inBand = false;
	float startX = 0, endX = 0;

    int lastSliceIndex = sliceRange.index + sliceRange.count;    
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        const REOttaviaRangeModifier& modifier = phrase->OttaviaModifier();
        
        if(_track->IsBarEmptyOrRest(barIndex))
        {
            // Close current vibrato if pending
            if (inBand)
            {
                // Finish pm
				inBand = false;
				_DrawOttaviaBandPartial(painter, ottavia, startX, endX, y);
            }
            
            // Next Slice
            continue;
        }
        
        bool hasFlag = false;
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        do 
        {
            int tick = it.Tick();
            const REOttaviaRangeModifierElement* elem = modifier.ItemAppliedAt(tick);
            hasFlag = (elem != NULL && elem->value == ottavia);
            
            if (inBand) 
            {
                if(!hasFlag) 
                {
                    inBand = false;
       				_DrawOttaviaBandPartial(painter, ottavia, startX, endX, y);
                }
                else {
                    endX = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                }
            }
            else 
            {
                if(hasFlag) 
                {
                    inBand = true;
                    startX = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                    endX = startX;
                }
            }
            
        } while (it.Next());
    }
    
    // Pending band ?
	if (inBand) {
		inBand = false;
        _DrawOttaviaBandPartial(painter, ottavia, startX, endX, y);
	}
}

void REStaff::DrawTextBand(REPainter& painter, Reflow::TextPositioning positioning, float y, const RERange& sliceRange) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor::Red);
    painter.StrokeLine(REPoint(0, y), REPoint(_parent->Width(), y));
#endif*/
    
    const float dy = -3;
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    int lastSliceIndex = sliceRange.index + sliceRange.count;    
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        if(!it.IsValid()) continue;
        
        do {
            int tick = it.Tick();
            for(int i=FirstVoiceIndex(); i<=LastVoiceIndex(); ++i) {
                const REChord* chord = it.Chord(i);
                if(chord && chord->HasTextAttached() && chord->TextPositioning() == positioning) {
                    std::string txt = chord->TextAttached();
                    float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
                    painter.DrawText(txt, REPoint(x,y+dy), "Times New Roman", 0, 11.0, REColor::Black);
                }
            }
            
        } while(it.Next());
    }
}

void REStaff::DrawChordDiagramBand(REPainter& painter, float y, const RERange& sliceRange) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor(0.25, 1.0, 0.25));
    painter.StrokeLine(REPoint(0, y), REPoint(_parent->Width(), y));
#endif*/
    
    y = roundf(y);
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    int lastSliceIndex = sliceRange.index + sliceRange.count;
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        
        // Draw Chord Diagrams
        for(int i=0; i<phrase->ChordDiagramCount(); ++i)
        {
            const REChordDiagram* chordDiagram = phrase->ChordDiagramAtIndex(i);
            std::string txt = chordDiagram->ChordName().ToString();
            int tick = phrase->TickOfChordDiagramAtIndex(i);
            float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick));
            
            // Chord Name
            float fontSize = 9.0;
            const char* fontName = "Helvetica";
            REPoint pt = REPoint(x + 0.5, y + 0.5);
            
            // Text metrics
            RESize sz = painter.SizeOfText(txt, fontName, 0, fontSize);
            painter.DrawText(txt, REPoint(pt.x - sz.w/2, pt.y + 0.5), fontName, 0, fontSize, REColor(0,0,0));
            
            // Chord Diagram
            chordDiagram->Draw(painter, REPoint(x,y+12.0), ChordDiagramSize());
        }
    }
}

void REStaff::DrawDynamicsBand(REPainter& painter, float y, const RERange& sliceRange) const
{
/*#ifdef DEBUG
    painter.SetStrokeColor(REColor::Green);
    painter.StrokeLine(REPoint(0, y), REPoint(_parent->Width(), y));
#endif*/
    
    const float dy = 1.0;
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    Reflow::DynamicsType lastDynamics = Reflow::DynamicsUndefined;

    int lastSliceIndex = sliceRange.index + sliceRange.count;    
    for(int sliceIndex=sliceRange.index; sliceIndex < lastSliceIndex; ++sliceIndex)
    {
        const RESlice* slice = _parent->SystemBar(sliceIndex);
        int barIndex = slice->BarIndex();
        const REPhrase* phrase = _track->Voice(0)->Phrase(barIndex);
        
        REMultivoiceIterator it = _track->MultivoiceIteratorOnBar(barIndex, FirstVoiceIndex(), LastVoiceIndex());
        if(!it.IsValid()) continue;
        
        do {
            int tick = it.Tick();
            for(int i=FirstVoiceIndex(); i<=LastVoiceIndex(); ++i) {
                const REChord* chord = it.Chord(i);
                if(chord) 
                {
                    Reflow::DynamicsType dt = chord->Dynamics();
                    if(dt != Reflow::DynamicsUndefined && dt != lastDynamics)
                    {
                        lastDynamics = dt;
                        std::string txt = Reflow::NameOfDynamics(dt);
                        float x = roundf(slice->XOffset() + slice->XOffsetOfTick(tick) - UnitSpacing());
                        //painter.DrawText(txt, REPoint(x,y+dy), "Times New Roman", REPainter::Italic, 10.0, REColor::Black);
                        painter.DrawMusicSymbol(txt.c_str(), REPoint(x, y+dy), UnitSpacing());
                    }
                }
            }
            
        } while(it.Next());
    }
}

