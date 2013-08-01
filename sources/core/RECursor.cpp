//
//  RECursor.cpp
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RECursor.h"
#include "REFunctions.h"
#include "REScore.h"
#include "RESong.h"
#include "RETrack.h"
#include "REVoice.h"
#include "RESystem.h"
#include "REStaff.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REStandardStaff.h"
#include "RETablatureStaff.h"
#include "RESlice.h"
#include "REInputStream.h"
#include "REOutputStream.h"

RECursor::RECursor()
: _score(NULL), _beat(0,0), _staffIndex(0), _lineIndex(0), _voiceSelectionType(HighVoiceSelection)
{
}

RECursor::RECursor(const REScore* score)
: _score(score), _beat(0,0), _staffIndex(0), _lineIndex(0), _voiceSelectionType(HighVoiceSelection)
{
}

RECursor& RECursor::operator=(const RECursor& rhs)
{
    _score = rhs._score;
    _beat = rhs._beat;
    _staffIndex = rhs._staffIndex;
    _lineIndex = rhs._lineIndex;
    _voiceSelectionType = rhs._voiceSelectionType;
    return *this;
}

void RECursor::SetTick(unsigned long tick)
{
    _beat.timeDiv = Reflow::TicksToTimeDiv(tick);
}

void RECursor::ForceValidPosition(bool assertChordOnBeat)
{
    const RESong* song = _score->Song();
    if(song->BarCount() == 0) {
        _beat = REGlobalTimeDiv(0, 0);
        _staffIndex = 0;
        _lineIndex = 0;
        return;
    }
    
    // Validate Bar
    int barIndex = _beat.bar;
    barIndex = Reflow::Clamp<int>(barIndex, 0, song->BarCount()-1);
    _beat.bar = barIndex;
    
    // Find system associated with this bar
    const RESystem* system = _score->SystemWithBarIndex(barIndex);
    assert(system != NULL);
    
    // Validate Staff
    if(system->StaffCount() == 0)
    {
        _lineIndex = 0;
        return;
    }
    _staffIndex = Reflow::Clamp<int>(_staffIndex, 0, system->StaffCount()-1);
    const REStaff* staff = system->Staff(_staffIndex);
    assert(staff != NULL);
    
    // Validate Line
    const RETrack* track = staff->Track();
    if(staff->Type() == Reflow::TablatureStaff) {
        _lineIndex = Reflow::Clamp<int>(_lineIndex, 0, track->StringCount()-1);
    }
    
    // Phrase Under
    if(assertChordOnBeat)
    {
        const REPhrase* phrase = Phrase();
        if(phrase && !phrase->IsEmpty())
        {
            REConstChordPair chords = phrase->ChordsSurroundingTick(Reflow::TimeDivToTicks(_beat.timeDiv));
            if(chords.first) {
                _beat.timeDiv = chords.first->Offset();
            }
            else if(chords.second) {
                _beat.timeDiv = chords.second->Offset();
            }
            else {
                _beat.timeDiv = RETimeDiv(0);
            }
        }
        else {
            _beat.timeDiv = RETimeDiv(0);
        }
    }
}

const RESong* RECursor::Song() const
{
    return (_score ? _score->Song() : NULL);
}

const REScore* RECursor::Score() const
{
    return _score;
}

const RESystem* RECursor::System() const
{
    return (_score ? _score->SystemWithBarIndex(_beat.bar) : NULL);
}

const RESlice* RECursor::Slice() const
{
    const RESystem* system = System();
    return (system != NULL ? system->SystemBarWithBarIndex(_beat.bar) : NULL);
}

const REStaff* RECursor::Staff() const
{
    const RESystem* system = System();
    return (system != NULL ? system->Staff(_staffIndex) : NULL);
}

const REStandardStaff* RECursor::StandardStaff() const
{
    const REStaff* staff = Staff();
    if(staff && staff->Type() == Reflow::StandardStaff) 
    {
        return static_cast<const REStandardStaff*>(staff);
    }
    return NULL;
}
const RETablatureStaff* RECursor::TablatureStaff() const
{
    const REStaff* staff = Staff();
    if(staff && staff->Type() == Reflow::TablatureStaff) 
    {
        return static_cast<const RETablatureStaff*>(staff);
    }
    return NULL;    
}

const RETrack* RECursor::Track() const
{
    const REStaff* staff = Staff();
    return (staff ? staff->Track() : NULL);
}

const REBar* RECursor::Bar() const
{
    const RESong* song = Song();
    return (song != NULL ? song->Bar(_beat.bar) : NULL);
}

int RECursor::VoiceIndex() const
{
    int handIndex = GrandStaffHandIndex();
    if(handIndex == -1) handIndex = 0;
    
    return (2*handIndex) + (_voiceSelectionType == RECursor::LowVoiceSelection ? 1 : 0);
}

const REVoice* RECursor::Voice() const
{
    const RETrack* track = Track();
    return track ? track->Voice(VoiceIndex()) : NULL;
}

int RECursor::GrandStaffHandIndex() const
{
    const REStandardStaff* staff = StandardStaff();
    if(staff == NULL) return -1;
    return staff->Hand();
}

const REPhrase* RECursor::Phrase() const
{
    const REVoice* voice = Voice();
    return (voice ? voice->Phrase(_beat.bar) : NULL);
}

const REChord* RECursor::Chord() const
{
    const REPhrase* phrase = Phrase();
    return (phrase ? phrase->ChordAtTick(Tick()) : NULL);
}

int RECursor::ChordIndex() const
{
    const REChord* chord = Chord();
    return (chord ? chord->Index() : 0);
}

const RENote* RECursor::Note() const
{
    const REChord* chord = Chord();
    if(chord == NULL) return NULL;
    
    const REStaff* staff = Staff();
    if(staff->Type() == Reflow::StandardStaff)
    {
        bool transposingScore = _score->IsTransposing();
        return chord->NoteOnStaffLine(_lineIndex, transposingScore);
    }
    else {
        return chord->NoteOnString(_lineIndex);
    }
}

int RECursor::Tick() const
{
    return Reflow::TimeDivToTicks(_beat.timeDiv);
}

REConstChordPair RECursor::ChordsSurroundingTick() const
{
    const REChord* chordLeft = NULL;
    const REChord* chordRight = NULL;
    const REPhrase* phrase = Phrase();
    if(phrase) {
        phrase->ChordsSurroundingTick(Tick(), &chordLeft, &chordRight);
    }
    return REConstChordPair(chordLeft, chordRight);
}

void RECursor::EncodeTo(REOutputStream& coder) const
{
    _beat.EncodeTo(coder);
    coder.WriteInt32(_staffIndex);
    coder.WriteInt32(_lineIndex);
    coder.WriteInt32((int32_t)_voiceSelectionType);
}
void RECursor::DecodeFrom(REInputStream& decoder)
{
    _beat.DecodeFrom(decoder);
    _staffIndex = decoder.ReadInt32();
    _lineIndex = decoder.ReadInt32();
    _voiceSelectionType = (RECursor::VoiceSelectionType)decoder.ReadInt32();
}
