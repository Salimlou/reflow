//
//  REStandardNotationCompiler.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/12/12.
//
//

#include "REStandardNotationCompiler.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RETrack.h"
#include "REBar.h"
#include "REFunctions.h"
#include "RENote.h"
#include "REPitch.h"

#include <boost/foreach.hpp>


static int alteration_for_key_step [15][7] = {
	{3,3,3,3,3,3,3},
	{3,3,3,0,3,3,3},
	{0,3,3,0,3,3,3},
	{0,3,3,0,0,3,3},
	{0,0,3,0,0,3,3},
	{0,0,3,0,0,0,3},
	{0,0,0,0,0,0,3},
	
	{0,0,0,0,0,0,0},
	
	{0,0,0,1,0,0,0},
	{1,0,0,1,0,0,0},
	{1,0,0,1,1,0,0},
	{1,1,0,1,1,0,0},
	{1,1,0,1,1,1,0},
	{1,1,1,1,1,1,0},
	{1,1,1,1,1,1,1}
};

REStandardNotationCompiler::REStandardNotationCompiler()
: _transposed(false)
{
    memset(_accidentalOnLine, 0, sizeof(_accidentalOnLine));
}

void REStandardNotationCompiler::InitializeWithPhrase(const REPhrase* phrase, bool transposed)
{
    const RETrack* track = phrase->Track();
    _transposed = transposed;
    _transposingInterval = (transposed ? track->TransposingInterval() : REPitchClass::C);
    
    bool leftHand = phrase->OnLeftHandStaff();
    const REClefItem* clefAtBar = track->ClefTimeline(leftHand).ItemAt(phrase->Index(), RETimeDiv(0));
    _clef = (clefAtBar ? clefAtBar->clef : Reflow::TrebleClef);
    _ottavia = (clefAtBar ? clefAtBar->ottavia : Reflow::NoOttavia);
    
    // Initialize accidentals of the key signature
    const REBar* bar = phrase->Locator().Bar();
    _key = (bar != NULL ? bar->KeySignature().key : REFLOW_KEY_Cmajor);
    InitializeKeySignatureAccidentals(_key);
}

RENotePitch REStandardNotationCompiler::FindPitchInPhrase(const REPhrase* phrase, int chordIndex_, int lineIndex) const
{
    Reflow::Accidental accidentalOnLine = Reflow::NoAccidental;
    
    int step;
    if(_clef == Reflow::TrebleClef) {
        step = (REFLOW_STEP_F1 - lineIndex) % 7;
    }
    else {
        step = (REFLOW_STEP_A_minus_1 - lineIndex) % 7;
    }
    
    accidentalOnLine = (Reflow::Accidental) alteration_for_key_step[_key][step];
    
    int maxChordIndex = std::min<unsigned int>(phrase->ChordCount(), chordIndex_+1);
    for(unsigned int chordIndex=0; chordIndex<maxChordIndex; ++chordIndex)
    {
        const REChord* chord = phrase->Chord(chordIndex);
        for(unsigned int noteIndex=0; noteIndex<chord->NoteCount(); ++noteIndex)
        {
            const RENote* note = chord->Note(noteIndex);
            const RENote::REStandardRep& rep = note->Representation(_transposed);
            if(rep.line == lineIndex)
            {
                accidentalOnLine = Reflow::Accidental(rep.accidental);
            }
        }
    }
    
    int octaveShift = 0;
    switch(_ottavia) {
        case Reflow::Ottavia_8vb:  octaveShift = -1; break;
        case Reflow::Ottavia_15mb: octaveShift = -2; break;
        case Reflow::Ottavia_8va:  octaveShift = 1; break;
        case Reflow::Ottavia_15ma: octaveShift = 2; break;
        default:break;
    }
    
    return Reflow::PitchFromStd(lineIndex, _clef, octaveShift, accidentalOnLine);
}

void REStandardNotationCompiler::CalculateStandardRepresentationOfNote(RENote* note, bool graceNote)
{
    RENote::REStandardRep& rep = note->Representation(_transposed);
    rep.flags = 0;
    rep.accidentalOffset = 0;
    
    RENotePitch notePitch = note->Pitch();
    REPitch pitch(REPitchClass::WithDiatonicStepAndAlteration(notePitch.step, notePitch.alter), notePitch.octave);
    if(_transposed) {
        pitch = pitch - _transposingInterval;
    }
    int octave = pitch.Octave();
    switch(_ottavia) {
        case Reflow::Ottavia_8vb:  octave += 1; break;
        case Reflow::Ottavia_15mb: octave += 2; break;
        case Reflow::Ottavia_8va:  octave -= 1; break;
        case Reflow::Ottavia_15ma: octave -= 2; break;
        default:break;
    }
    
    if(_clef == Reflow::TrebleClef) {
        rep.line = REFLOW_STEP_F1 - (7 * octave + pitch.DiatonicStep());
    }
    else if(_clef == Reflow::BassClef) {
        rep.line = REFLOW_STEP_A_minus_1 - (7 * octave + pitch.DiatonicStep());
    }
    else {
        rep.line = 0;
    }
    
    Reflow::Accidental previousAccidental = (Reflow::Accidental) _accidentalOnLine[rep.line + 127];
    
    switch(pitch.Alteration())
    {
        case -2: rep.accidental = Reflow::DoubleFlat; break;
        case -1: rep.accidental = Reflow::Flat; break;
        case  0: rep.accidental = Reflow::NoAccidental; break;
        case  1: rep.accidental = Reflow::Sharp; break;
        case  2: rep.accidental = Reflow::DoubleSharp; break;
    }
    
    if(rep.accidental != previousAccidental)
    {
        _accidentalOnLine[rep.line + 127] = rep.accidental;
        if(rep.accidental == Reflow::NoAccidental) {
            rep.accidental = Reflow::Natural;
        }
        
        // There is an accidental on this line
        if(!graceNote) _accidentedNotesMappedByLine[rep.line] = note;
    }
    else {
        rep.accidental = Reflow::NoAccidental;
    }
    
    rep.noteHeadSymbol = Reflow::DefaultNoteHead;
    if(note->HasFlag(RENote::DeadNote)) {
        rep.noteHeadSymbol = Reflow::DoubleSharpNoteHead;
    }
    
    // Add Line to our Line set
    if(!graceNote) _lines.insert(rep.line);
}

void REStandardNotationCompiler::ProcessChord(REChord* chord)
{
    _accidentedNotesMappedByLine.clear();
    _lines.clear();
    
    for(unsigned int noteIndex=0; noteIndex<chord->NoteCount(); ++noteIndex)
    {
        RENote* note = chord->Note(noteIndex);
        
        // Calculate representation of grace notes before
        int graceNoteCount = note->GraceNoteCount();
        for(int i=0; i<graceNoteCount; ++i)
        {
            REGraceNote* graceNote = note->GraceNote(i);
            CalculateStandardRepresentationOfNote(graceNote, true);
        }
        
        CalculateStandardRepresentationOfNote(note, false);
    }
    
    // Determine column offsets of accidentals to avoid collisions
    CalculateAccidentalOffsets();
    
    // Determine second intervals
    CalculateSecondIntervalStacking(chord);
}

void REStandardNotationCompiler::InitializeKeySignatureAccidentals(int key)
{
    for(int i=0; i<256; ++i)
    {
        int line = i - 127;
        int step;
        if(_clef == Reflow::TrebleClef) {
            step = (REFLOW_STEP_F1 - line) % 7;
        }
        else {
            step = (REFLOW_STEP_A_minus_1 - line) % 7;
        }
        
        _accidentalOnLine[i] = alteration_for_key_step[key][step];
    }
}

void REStandardNotationCompiler::CalculateAccidentalOffsets()
{
    int lineOfAccidentalOffset[6] = {-1000, -1000, -1000, -1000, -1000, -1000};
    std::map<int, RENote*>::const_iterator it = _accidentedNotesMappedByLine.begin();
    for(; it != _accidentedNotesMappedByLine.end(); ++it)
    {
        int line = it->first;
        RENote* note = it->second;
        RENote::REStandardRep& rep = note->Representation(_transposed);
        
        rep.accidentalOffset = 0;
        for(int col=0; col<6; ++col)
        {
            if(line - lineOfAccidentalOffset[col] > 6) {
                lineOfAccidentalOffset[col] = line;
                rep.accidentalOffset = col;
                break;
            }
        }
    }
    
}

void REStandardNotationCompiler::CalculateSecondIntervalStacking(REChord* chord)
{
    int lineCount = _lines.size();
    if(lineCount > 1)
    {
        // When stem is down
        {
            bool lastReversed = false;
            REIntSet::const_iterator it = _lines.begin();
            int last = *it;
            ++it;
            while(it != _lines.end())
            {
                int line = *it;
                if((line - last) == 1)
                {
                    RENote* note = chord->NoteOnStaffLine(line, _transposed);
                    if(note && !lastReversed)
                    {
                        RENote::REStandardRep& rep = note->Representation(_transposed);
                        rep.flags |= RENote::StackedSecondOnOppositeSideWithStemDown;
                        lastReversed = true;
                    }
                    else lastReversed = false;
                }
                else {
                    lastReversed = false;
                }
                last = line;
                ++it;
            }
        }
        
        // When stem is up
        {
            bool lastReversed = false;
            REIntSet::const_reverse_iterator it = _lines.rbegin();
            int last = *it;
            ++it;
            while(it != _lines.rend())
            {
                int line = *it;
                if((last - line) == 1)
                {
                    RENote* note = chord->NoteOnStaffLine(line, _transposed);
                    if(note && !lastReversed)
                    {
                        RENote::REStandardRep& rep = note->Representation(_transposed);
                        rep.flags |= RENote::StackedSecondOnOppositeSideWithStemUp;
                        lastReversed = true;
                    }
                    else lastReversed = false;
                }
                else {
                    lastReversed = false;
                }
                last = line;
                ++it;
            }
        }
    }
}