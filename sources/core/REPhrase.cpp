//
//  REPhrase.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REPhrase.h"
#include "REChord.h"
#include "REVoice.h"
#include "RETrack.h"
#include "REBar.h"
#include "REFunctions.h"
#include "RENote.h"
#include "RESong.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REPitch.h"
#include "REStandardNotationCompiler.h"

REPhrase::REPhrase()
: _parent(0), _index(-1)
{
}

REPhrase::~REPhrase()
{
    Clear();
}

const REChord* REPhrase::Chord(int idx) const
{
    if(idx >= 0 && idx < _chords.size()) {
        return _chords[idx];
    }
    return 0;
}
REChord* REPhrase::Chord(int idx)
{
    if(idx >= 0 && idx < _chords.size()) {
        return _chords[idx];
    }
    return 0;
}
void REPhrase::Clear()
{
    for(REChordVector::const_iterator it = _chords.begin(); it != _chords.end(); ++it) {
        delete *it;
    }
    _chords.clear();
    _ottaviaModifier.Clear();
}

const REPhrase* REPhrase::NextSibling() const
{
    if(_parent) {return _parent->Phrase(Index()+1);}
    return NULL;
}

const REPhrase* REPhrase::PreviousSibling() const
{
    if(_parent) {return _parent->Phrase(Index()-1);}
    return NULL;    
}

REPhrase* REPhrase::NextSibling()
{
    if(_parent) {return _parent->Phrase(Index()+1);}
    return NULL;
}

REPhrase* REPhrase::PreviousSibling()
{
    if(_parent) {return _parent->Phrase(Index()-1);}
    return NULL;    
}

RELocator REPhrase::Locator() const
{
    const REVoice* voice = Voice();
	if(!voice) return RELocator();
    
	const RETrack* track = voice->Track();
	if(!track) return RELocator();
    
	const RESong* song = track->Song();
	if(!song) return RELocator();
    
	return RELocator(song, Index(), track->Index(), voice->Index());   
}

void REPhrase::AddChord(REChord* chord)
{
    _chords.push_back(chord);
    chord->_parent = this;
    _UpdateIndices();
}

void REPhrase::InsertChord(REChord* chord, int idx)
{
    _chords.insert(_chords.begin() + idx, chord);
    chord->_parent = this;
    _UpdateIndices();
}

void REPhrase::RemoveChord(int idx)
{
    if(idx >= 0 && idx < _chords.size()) {
        REChord* chord = _chords[idx];
        delete chord;
        _chords.erase(_chords.begin() + idx);
        _UpdateIndices();
    }
}

void REPhrase::RemoveChord(REChord* chord)
{
    RemoveChord(chord->Index());
}

void REPhrase::_UpdateIndices() {
    for(unsigned int i=0; i<_chords.size(); ++i) {
        _chords[i]->_index = i;
    }
}

bool REPhrase::operator==(const REPhrase &rhs) const {
    if(ChordCount() != rhs.ChordCount()) {
        return false;
    }
    
    for(unsigned int i=0; i<_chords.size(); ++i) {
        if(*Chord(i) != *rhs.Chord(i)) {
            return false;
        }
    }
    return true;
}

void REPhrase::Refresh(bool fixTieFlags)
{
    RETrack* track = Track();
    
    RETimeDiv offset(0);
    for(unsigned int i=0; i<_chords.size(); ++i) 
    {
        REChord* chord = _chords[i];
        if(track->IsDrums()) {
        }
        else if(track->IsTablature())
        {
            // Sanitize notes outside string range
            REConstNoteVector notesToRemove;
            for(const RENote* note : chord->Notes()) {
                if(note->String() >= track->StringCount()) {
                    notesToRemove.push_back(note);
                }
            }
            for(const RENote* noteToRemove : notesToRemove) {
                chord->RemoveNote(noteToRemove->Index());
            }
            
            _CalculatePitchesFromTablature(chord);
        }
        chord->_offset = offset;
        offset += chord->Duration();
    }
    _duration = offset;
    
    if(track->IsDrums()) {
        _CalculateDrumsRepresentation();
    }
    else {
        _CalculateStandardRepresentation(false);
        _CalculateStandardRepresentation(true);
    }
    _CalculateBeamingGroups();
    _CalculateTupletGroups();
    
    // Fix Tie flags
	if(fixTieFlags) {
		_FixTieFlags();
	}
}

bool REPhrase::_FixTieFlags()
{
    bool modifiedPreviousSibling = false;
	for(unsigned int i=0; i<_chords.size(); ++i) 
    {
        REChord* chord = _chords[i];
		bool firstChord = (i==0);
		
        for(unsigned int j=0; j<chord->NoteCount(); ++j)
        {
            RENote* note = chord->Note(j);
			if(note->HasFlag(RENote::TieDestination))
            {
                RENote* tied = note->FindOriginOfTiedNote();
                if(tied) {
                    tied->SetFlag(RENote::TieOrigin);
                    note->SetFret(tied->Fret());
					if(firstChord) modifiedPreviousSibling = true;
                }
                else {
                    note->UnsetFlag(RENote::TieDestination);
                }
            }
            
			if(note->HasFlag(RENote::TieOrigin)) 
			{
                RENote* tied = note->FindDestinationOfTiedNote();
                if(tied == NULL || !tied->HasFlag(RENote::TieDestination)) {
                    note->UnsetFlag(RENote::TieOrigin);
                }
            }
        }
    }
	return modifiedPreviousSibling;
}

const REBar* REPhrase::Bar() const
{
    const RETrack* track = Track();
    if(track == NULL) return NULL;
    
    const RESong* song = track->Song();
    if(song == NULL) return NULL;
    
    return song->Bar(Index());
}

REBar* REPhrase::Bar()
{
    RETrack* track = Track();
    if(track == NULL) return NULL;
    
    RESong* song = track->Song();
    if(song == NULL) return NULL;
    
    return song->Bar(Index());
}

const REChord* REPhrase::ChordAtTick(long tick) const
{
    for(int i=0; i<_chords.size(); ++i) {
        const REChord* chord = Chord(i);
        if(chord->OffsetInTicks() == tick) {
            return chord;
        }
    }
    return NULL;
}

const REChord* REPhrase::ChordAtTimeDiv(const RETimeDiv& div) const
{
    for(int i=0; i<_chords.size(); ++i) {
        const REChord* chord = Chord(i);
        if(chord->Offset() == div) {
            return chord;
        }
    }
    return nullptr;
}

REConstChordPair REPhrase::ChordsSurroundingTick(long tick) const
{
    const REChord* left = NULL;
    const REChord* right = NULL;
    ChordsSurroundingTick(tick, &left, &right);
    return REConstChordPair(left, right);
}

void REPhrase::ChordsSurroundingTick(long tick, const REChord** chordLeft, const REChord** chordRight) const
{    
    if(ChordCount() == 0) {
        *chordLeft = NULL;
        *chordRight = NULL;
    }
    else if(ChordCount() == 1) {
        const REChord* chord = _chords[0];
        if((long)chord->OffsetInTicks() >= tick) {
            *chordLeft = chord;
            *chordRight = NULL;
        }
        else {
            *chordLeft = NULL;
            *chordRight = chord;
        }
    }
    else {
        const REChord* firstChord = _chords.front();
        const REChord* lastChord = _chords.back();
        if(tick < (long)firstChord->OffsetInTicks()) {
            *chordLeft = NULL;
            *chordRight = firstChord;
        }
        else if(tick >= (long)lastChord->OffsetInTicks()) {
            *chordLeft = lastChord;
            *chordRight = NULL;
        }
        else {
            *chordLeft = _chords[0];
            *chordRight = _chords[1];
            while((long)((*chordRight)->OffsetInTicks()) <= tick) {
                *chordLeft = *chordRight;
                *chordRight = (*chordRight)->NextSibling();
            }
        }
    }
}

const RETrack* REPhrase::Track() const
{
    return (_parent != NULL ? _parent->Track() : NULL);
}
RETrack* REPhrase::Track()
{
    return (_parent != NULL ? _parent->Track() : NULL);
}

void REPhrase::_CalculatePitchesFromTablature(REChord *chord)
{
    RETrack* track = Track();
    for(unsigned int i=0; i<chord->NoteCount(); ++i)
    {
        RENote* note = chord->Note(i);
        if(note->String() == -1 || note->Fret() == -1) continue;
        
        // Calculate Midi value of the note
        int midi = note->Fret() + (track != NULL ? 
                                   track->TuningForString(note->String()) :
                                   Reflow::StandardTuningForString(note->String()));
        

        
        // Calculate Pitch of the note
        note->SetPitchFromMIDI(midi);
        
        for(int graceIndex = 0; graceIndex < note->GraceNoteCount(); ++graceIndex)
        {
            REGraceNote* graceNote = note->GraceNote(graceIndex);
            if(graceNote->Fret() == -1) continue;
            
            // Calculate Midi value of the note
            int midi = graceNote->Fret() + (track != NULL ?
                                       track->TuningForString(note->String()) :
                                       Reflow::StandardTuningForString(note->String()));
            
            
            
            // Calculate Pitch of the note
            graceNote->SetPitchFromMIDI(midi);
        }
    }
}

bool REPhrase::OnLeftHandStaff() const
{
    return _parent->Index() >= 2;
}


void REPhrase::_CalculateStandardRepresentation(bool transposed)
{
    REStandardNotationCompiler notation;
    notation.InitializeWithPhrase(this, transposed);
    
    for(unsigned int chordIndex=0; chordIndex<_chords.size(); ++chordIndex) 
    {
        REChord* chord = _chords[chordIndex];
        notation.ProcessChord(chord);
    }
}

RENotePitch REPhrase::PitchFromStd(unsigned int chordIndex_, int lineIndex, bool transposingScore) const
{
    REStandardNotationCompiler notation;
    notation.InitializeWithPhrase(this, transposingScore);
    return notation.FindPitchInPhrase(this, chordIndex_, lineIndex);
}


void REPhrase::_CalculateDrumsRepresentation()
{
    for(unsigned int chordIndex=0; chordIndex<_chords.size(); ++chordIndex) 
    {
        REChord* chord = _chords[chordIndex];
        for(unsigned int noteIndex=0; noteIndex<chord->NoteCount(); ++noteIndex)
        {
            RENote* note = chord->Note(noteIndex);
            if(note->GraceNoteCount() > 0)
            {
                for(REGraceNote* graceNote : note->GraceNotes())
                {
                    RENote::REStandardRep& rep = graceNote->Representation(false);
                    int midi = graceNote->Pitch().midi;
                    
                    const REDrumMapping& mapping = Reflow::StandardDrumMapping(midi);
                    rep.line = mapping.line;
                    rep.noteHeadSymbol = mapping.noteHeadSymbol;
                    rep.accidental = Reflow::NoAccidental;
                    rep.flags = 0;
                }
            }
            
            RENote::REStandardRep& rep = note->Representation(false);
            int midi = note->Pitch().midi;
            
            const REDrumMapping& mapping = Reflow::StandardDrumMapping(midi);
            rep.line = mapping.line;
            rep.noteHeadSymbol = mapping.noteHeadSymbol;
            rep.accidental = Reflow::NoAccidental;
            rep.flags = 0;
        }
    }
}

void REPhrase::_BeamElements(int firstIndex, int lastIndex)
{
    // Trim leading rests and not beamable
    while(firstIndex < lastIndex)
    {
        REChord* slash = Chord(firstIndex);
        if(slash->IsRest()) {
            ++firstIndex;
        }
        else if(slash->NoteValue() <= Reflow::QuarterNote) {
            //_CalculateBeamingCoordinates(firstIndex, firstIndex);
            ++firstIndex;
        }
        else break;
    }
    
    // Trim trailing rests and not beamable
    while(lastIndex > firstIndex) 
    {
        REChord* slash = Chord(lastIndex);
        if(slash->IsRest()) {
            --lastIndex;
        }
        else if(slash->NoteValue() <= Reflow::QuarterNote) {
            //_CalculateBeamingCoordinates(lastIndex, lastIndex);
            --lastIndex;
        }
        else break;
    }
    
    // Nothing to beam
    if(firstIndex == lastIndex) {
        return;
    }
    
    REChord* e0 = Chord(firstIndex);
    REChord* e1 = Chord(lastIndex);
    e0->SetFlag(REChord::BeamingStart);
    e1->SetFlag(REChord::BeamingEnd);
}


void REPhrase::_CalculateBeamingGroups()
{
    if(ChordCount() == 0) return;
    
    int currentBeam = 0;
    int nbBeaming = 1;
    unsigned long beamingPattern[REBeamingPattern::MaxGroupCount] = {REFLOW_PULSES_PER_QUARTER};
    const REBar* bar = Bar();
    if(bar)
    {
        nbBeaming = bar->BeamingPattern().ToTickPattern(beamingPattern);
    }
    int groupingFrom = 0;
    int groupingTo = 0;
    unsigned long beamingStartTick = 0;
    unsigned long beamingLastTick = beamingStartTick + beamingPattern[currentBeam % nbBeaming];
    unsigned long tick = 0;
    
    unsigned int i = 0;
    unsigned int chordCount = ChordCount();
    while(i < chordCount)
    {
        REChord* chord = Chord(i);
        chord->UnsetFlag(REChord::BeamingStart);
        chord->UnsetFlag(REChord::BeamingEnd);
        unsigned long duration = chord->DurationInTicks();
        if(tick >= beamingStartTick &&
           tick < beamingLastTick && 
           (tick + duration <= beamingLastTick) &&
           chord->NoteValue() > Reflow::QuarterNote)
        {
            groupingTo = i;
            ++i;
            tick += duration;
        }
        else {
            // Close current beaming
            _BeamElements(groupingFrom, groupingTo);
            
            // And start a new beaming
            currentBeam = (currentBeam + 1) % nbBeaming;
            beamingStartTick = beamingLastTick;
            beamingLastTick = beamingStartTick + beamingPattern[currentBeam];
            groupingFrom = i;
            groupingTo = i;
            ++i;
            
            tick += duration;
        }
    }
 
    _BeamElements(groupingFrom, groupingTo);
}

void REPhrase::_CalculateTupletGroups()
{
    bool inTuplet = false;
    RETuplet currentTuplet (0,0);
    unsigned int nbTuplets = 0;
    unsigned int tupletStartChordIdx = 0;
    unsigned int tupletEndChordIdx = 0;
    
    unsigned int chordCount = ChordCount();
    
    for(int i=0; i<chordCount; ++i)
    {
        REChord* chord = Chord(i);
        chord->UnsetFlag(REChord::TupletGroupStart);
        chord->UnsetFlag(REChord::TupletGroupEnd);
        chord->UnsetFlag(REChord::TupletGrouping);
    }
    
    unsigned int idx = 0;
    while(idx < chordCount)
    {
        REChord* chord = Chord(idx);
        RETuplet tuplet = chord->Tuplet();
        
        // Already in a tuplet group
        if(inTuplet)
        {
            if(tuplet == currentTuplet)
            {
                // Continue pending tuplet
                ++tupletEndChordIdx;
                ++nbTuplets;
                
                if(nbTuplets == currentTuplet.tuplet) {
                    // Tuplet is full, close
                    inTuplet = false;
                    _GroupTupletElements(tupletStartChordIdx, tupletEndChordIdx);
                    ++idx;
                    continue;
                }
            }
            else {
                // Break pending tuplet
                inTuplet = false;
                _GroupTupletElements(tupletStartChordIdx, tupletEndChordIdx);                
            }
        }
        
        
        if(!inTuplet && tuplet.tuplet != 0)
        {
            inTuplet = true;
            nbTuplets = 1;
            currentTuplet = tuplet;
                
            tupletStartChordIdx = idx;
            tupletEndChordIdx = idx;
        }

        ++idx;
    }
    
    if(inTuplet) {
        _GroupTupletElements(tupletStartChordIdx, tupletEndChordIdx);
    }
}

void REPhrase::_GroupTupletElements(unsigned int firstIndex, unsigned int lastIndex)
{
    REChord* firstChord = Chord(firstIndex);
    REChord* lastChord = Chord(lastIndex);
    
    unsigned int idx = firstIndex;
    while(idx <= lastIndex) 
    {
        REChord* chord = Chord(idx);
        chord->UnsetFlag(REChord::TupletGroupStart);
        chord->UnsetFlag(REChord::TupletGroupEnd);
        chord->SetFlag(REChord::TupletGrouping);
        ++idx;
    }
    
    firstChord->SetFlag(REChord::TupletGroupStart);
    lastChord->SetFlag(REChord::TupletGroupEnd);
}

const RETimeDiv& REPhrase::Duration() const
{
    return _duration;
}

unsigned long REPhrase::DurationInTicks() const
{
    return Reflow::TimeDivToTicks(Duration());
}

bool REPhrase::IsBarComplete() const
{
    if(_parent) {
        return Locator().Bar()->TheoricDuration() == Duration();
    }
    else return false;
}

bool REPhrase::IsBarCompleteOrExceeded() const
{
    if(_parent) {
        return Duration() >= Locator().Bar()->TheoricDuration();
    }
    else return false;
}

const REChord* REPhrase::FirstChord() const
{
    return Chord(0);
}
const REChord* REPhrase::LastChord() const
{
    return Chord((int)ChordCount() - 1);
}

REChord* REPhrase::FirstChord()
{
    return Chord(0);
}
REChord* REPhrase::LastChord()
{
    return Chord((int)ChordCount() - 1);
}

bool REPhrase::IsEmpty() const
{
    return _chords.empty();
}
bool REPhrase::IsEmptyOrRest() const
{
    if(IsEmpty()) return true;
    for(unsigned int i=0; i<_chords.size(); ++i) {
        const REChord* chord = _chords[i];
        if(!chord->IsRest()) return false;
    }
    return true;
}

void REPhrase::CalculateLineRange(bool transposed, int* outMinLine, int* outMaxLine) const
{
    for(const REChord* chord : _chords) {
        chord->CalculateLineRange(transposed, outMinLine, outMaxLine);
    }
}


const REChordDiagram* REPhrase::ChordDiagramAtIndex(int idx) const
{
    if(idx >= 0 && idx < _chordDiagrams.size()) {
        return &_chordDiagrams[idx].second;
    }
    return NULL;
}

const REChordDiagram* REPhrase::ChordDiagramAtTick(int tick) const
{
    return ChordDiagramAtIndex(IndexOfChordDiagramAtTick(tick));
}

void REPhrase::InsertChordDiagram(int tick, const REChordDiagram& chordDiagram)
{
    int idx = IndexOfChordDiagramAtTick(tick);
    if(idx == -1)
    {
        idx = 0;
        int count = _chordDiagrams.size();
        while(idx < count && _chordDiagrams[idx].first < tick) {
            ++idx;
        }
        _chordDiagrams.insert(_chordDiagrams.begin()+idx, REChordDiagramTickPair(tick, chordDiagram));
    }
    else {
        _chordDiagrams[idx] = REChordDiagramTickPair(tick, chordDiagram);
    }
}
bool REPhrase::HasChordDiagramAtTick(int tick) const
{
    REPhraseChordDiagramVector::const_iterator it = _chordDiagrams.begin();
    for(; it != _chordDiagrams.end(); ++it) {
        const REChordDiagramTickPair& p = *it;
        if(p.first == tick) {
            return true;
        }
    }
    return false;
}

int REPhrase::IndexOfChordDiagramAtTick(int tick) const
{
    int chordDiagramCount = _chordDiagrams.size();
    for(int i=0; i<chordDiagramCount; ++i) {
        const REChordDiagramTickPair& p = _chordDiagrams[i];
        if(p.first == tick) {
            return i;
        }
    }
    return -1;
}

void REPhrase::RemoveAllChordDiagrams()
{
    _chordDiagrams.clear();
}

int REPhrase::TickOfChordDiagramAtIndex(int idx) const
{
    if(idx >= 0 && idx < _chordDiagrams.size()) {
        return _chordDiagrams[idx].first;
    }
    return 0;
}

REPhrase* REPhrase::Clone() const
{
    REBufferOutputStream coder;
    EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REPhrase* phrase = new REPhrase;
    phrase->DecodeFrom(decoder);
    return phrase;
}

void REPhrase::CopyFrom(const REPhrase& phrase)
{
    REBufferOutputStream coder;
    phrase.EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    DecodeFrom(decoder);
}

void REPhrase::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    writer.EndObject();
}

void REPhrase::ReadJson(const REJsonValue& obj, uint32_t version)
{
    Clear();
}

void REPhrase::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt32(_flags);
    
    _ottaviaModifier.EncodeTo(coder);
    
    // Chord Diagrams
    coder.WriteInt8(_chordDiagrams.size());
    for(int i=0; i<_chordDiagrams.size(); ++i) {
        const REChordDiagramTickPair& p = _chordDiagrams[i];
        coder.WriteInt32(p.first);
        p.second.EncodeTo(coder);
    }
    
    uint16_t chordCount = _chords.size();
    coder.WriteUInt16(chordCount);
    for(uint16_t i=0; i<chordCount; ++i) {
        const REChord* chord = _chords[i];
        chord->EncodeTo(coder);
    }

}
void REPhrase::DecodeFrom(REInputStream& decoder)
{
    Clear();
    _flags = decoder.ReadUInt32();
    
    _ottaviaModifier.DecodeFrom(decoder);
    
    int nbChordDiagrams = decoder.ReadInt8();
    _chordDiagrams.clear();
    for(int i=0; i<nbChordDiagrams; ++i) {
        int tick = decoder.ReadInt32();
        REChordDiagram chordDiagram; chordDiagram.DecodeFrom(decoder);
        _chordDiagrams.push_back(REChordDiagramTickPair(tick, chordDiagram));
    }
    
    uint16_t chordCount = decoder.ReadUInt16();
    for(uint16_t i=0; i<chordCount; ++i) {
        REChord* chord = new REChord;
        chord->_index = i;
        chord->_parent = this;
        _chords.push_back(chord);
        
        chord->DecodeFrom(decoder);
    }
}
