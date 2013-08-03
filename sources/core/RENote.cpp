//
//  RENote.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RENote.h"
#include "REChord.h"
#include "REPhrase.h"
#include "RETrack.h"
#include "REFunctions.h"
#include "REBend.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#ifdef REFLOW_TRACE_INSTANCES
int RENote::_instanceCount = 0;
#endif

RENote::RENote()
: _parent(0), _index(-1), _fret(-1), _string(-1), _flags(0x00), _enharmonicHints(0), 
  _slideIn(Reflow::NoSlideIn), _slideOut(Reflow::NoSlideOut)
{
#ifdef REFLOW_TRACE_INSTANCES
    ++_instanceCount;
#endif
}

RENote::~RENote() 
{
#ifdef REFLOW_TRACE_INSTANCES
    --_instanceCount;
#endif
    
    ClearGraceNotes();
}

/*void RENote::Clear() {
    if(_bend) {
        delete _bend; _bend = NULL;
    }
}*/

bool RENote::operator==(const RENote &rhs) const {
    return _fret == rhs.Fret() && _string == rhs.String();
}

void RENote::SetEnharmonicHints(int ehints)
{
    unsigned int nbPitches = Reflow::EnharmonicEquivalentCount(_pitch.midi);
    if(ehints < 0) ehints = 0;
    if(ehints >= nbPitches) ehints = nbPitches-1;
    _enharmonicHints = ehints;
}

int RENote::NextEnharmonicHints() const
{
    unsigned int nbPitches = Reflow::EnharmonicEquivalentCount(_pitch.midi);
    return (_enharmonicHints + 1) % nbPitches;
}

RENotePitch RENote::DetermineNewPitch(int midi) const
{
    return DetermineNewPitch(midi, _enharmonicHints);
}

RENotePitch RENote::DetermineNewPitch(int midi, int enharmonicHints) const
{
    RENotePitch pitches[3];
    unsigned int nbPitches = Reflow::PitchSetFromMidi(midi, pitches);
    unsigned int enharmonic = std::min<unsigned int>(nbPitches, enharmonicHints);
    return pitches[enharmonic];
}

void RENote::ToggleEnharmonicHints()
{
    _enharmonicHints = NextEnharmonicHints();
    SetPitchFromMIDI(_pitch.midi);
}

void RENote::SetPitchFromMIDI(int midi)
{
    _pitch = DetermineNewPitch(midi);
}

void RENote::SetPitch(const RENotePitch& pitch)
{
    _pitch = pitch;
}

const RENote* RENote::FindNextNoteOnSameString() const
{
    const RETrack* track = _parent->Phrase()->Track();
    if(track->Type() != Reflow::TablatureTrack) {return NULL;}
    
    REChord* nextChord = _parent->NextSibling();
    return (nextChord ? nextChord->NoteOnString(String()) : NULL);
}

const RENote* RENote::FindPreviousNoteOnSameString() const
{
    const RETrack* track = _parent->Phrase()->Track();
    if(track->Type() != Reflow::TablatureTrack) {return NULL;}
    
    REChord* prevChord = _parent->PreviousSibling();
    return (prevChord ? prevChord->NoteOnString(String()) : NULL);    
}

const RENote* RENote::FindLastSiblingNoteOnSameStringInLegatoSuite() const
{
    const RENote* dest = FindNextNoteOnSameString();
    while(dest && dest->HasFlag(RENote::Legato))
    {
        const RENote* next = dest->FindNextNoteOnSameString();
        if(next == NULL) return dest;
        
        dest = next;
    }
    return dest;
}

RENote* RENote::FindDestinationOfTiedNote()
{
    const RETrack* track = _parent->Phrase()->Track();
    REChord* nextChord = _parent->NextSiblingOverBarline();
    if(nextChord == NULL) return NULL;
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        return nextChord->NoteOnString(String());
    }
    else
    {
        return nextChord->NoteWithMidi(Pitch().midi);
    }
}

RENote* RENote::FindOriginOfTiedNote()
{
    const RETrack* track = _parent->Phrase()->Track();
    REChord* prevChord = _parent->PreviousSiblingOverBarline();
    if(prevChord == NULL) return NULL;
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        return prevChord->NoteOnString(String());
    }
    else
    {
        return prevChord->NoteWithMidi(Pitch().midi);
    }
}

const RENote* RENote::FindDestinationOfTiedNote() const 
{
    const RETrack* track = _parent->Phrase()->Track();
    const REChord* nextChord = _parent->NextSiblingOverBarline();
    if(nextChord == NULL) return NULL;
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        return nextChord->NoteOnString(String());
    }
    else
    {
        return nextChord->NoteWithMidi(Pitch().midi);
    }
}

const RENote* RENote::FindOriginOfTiedNote() const 
{
    const RETrack* track = _parent->Phrase()->Track();
    const REChord* prevChord = _parent->PreviousSiblingOverBarline();
    if(prevChord == NULL) return NULL;
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        return prevChord->NoteOnString(String());
    }
    else
    {
        return prevChord->NoteWithMidi(Pitch().midi);
    }
}

RELocator RENote::Locator() const 
{
    const REChord* chord = Chord();
    if(!chord) return RELocator();
    
    RELocator locator = chord->Locator();
    locator.SetNoteIndex(Index());
    return locator;
}

bool RENote::HasBend() const {
    return HasFlag(RENote::BendEffect) && _bend.Type() != Reflow::NoBend;
}

void RENote::SetBend(const REBend& bend) {
    _bend = bend;
    if(_bend.Type() != Reflow::NoBend) {
        SetFlag(RENote::BendEffect);
    }
    else {
        UnsetFlag(RENote::BendEffect);
    }
}

void RENote::IncrementPitch()
{
    IncrementNotePitch(this);
}

void RENote::DecrementPitch()
{
    DecrementNotePitch(this);
}

void RENote::IncrementNotePitch(const RENote* baseNote)
{
    const RETrack* track = baseNote->_parent->Phrase()->Track();
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        if(_fret < 99) ++_fret;
    }
    else if(track->Type() == Reflow::StandardTrack)
    {
        if(_pitch.midi < 127) SetPitchFromMIDI(_pitch.midi + 1);
    }
}
void RENote::DecrementNotePitch(const RENote* baseNote)
{
    const RETrack* track = baseNote->_parent->Phrase()->Track();
    
    if(track->Type() == Reflow::TablatureTrack)
    {
        if(_fret > 0) --_fret;
    }
    else if(track->Type() == Reflow::StandardTrack)
    {
        if(_pitch.midi > 0) SetPitchFromMIDI(_pitch.midi - 1);
    }
}

int RENote::GraceNoteCount() const
{
    return _graceNotes.size();
}
const REGraceNote* RENote::GraceNote(int idx) const
{
    return (idx >= 0 && idx < GraceNoteCount() ? _graceNotes[idx] : NULL);
}
REGraceNote* RENote::GraceNote(int idx)
{
    return (idx >= 0 && idx < GraceNoteCount() ? _graceNotes[idx] : NULL);
}
void RENote::AddGraceNote(REGraceNote* gnote)
{
    _graceNotes.push_back(gnote);
}
void RENote::RemoveGraceNote(int idx)
{
    REGraceNote* gnote = GraceNote(idx);
    if(gnote) {
        gnote->Release();
        _graceNotes.erase(_graceNotes.begin() + idx);
    }
}
void RENote::ClearGraceNotes()
{
    for(REGraceNote* gnote : _graceNotes)
    {
        gnote->Release();
    }
    _graceNotes.clear();
}

float RENote::CalculateGraceNoteMetrics(bool transposed, float unitSpacing, REGraceNoteMetrics* metrics) const
{
    if(metrics) metrics->_columns.clear();
    float x = 0.5 * unitSpacing;
    for(const REGraceNote* gnote : _graceNotes)
    {
        const RENote::REStandardRep& rep = gnote->Representation(transposed);
        REGraceNoteMetrics::Column col;
        
        col.accidentalX = x;
        if(rep.accidental == Reflow::DoubleFlat) {
            x += 1.6 * unitSpacing;
        }
        else if(rep.accidental != Reflow::NoAccidental) {
            x += 0.8 * unitSpacing;
        }
        
        col.noteX = x;
        if(metrics) metrics->_columns.push_back(col);
        
        x += 1.2 * unitSpacing;
    }
    if(metrics) metrics->_width = x;
    return x;
}

void RENote::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt16(_flags);
    coder.WriteInt8(_fret);
    coder.WriteInt8(_string);
    coder.WriteInt8(_liveVelocity);
    coder.WriteInt16(_liveTickDuration);
    coder.WriteInt16(_liveTickOffset);
    coder.WriteUInt8(_enharmonicHints);
    coder.WriteInt8(_slideOut);
    coder.WriteInt8(_slideIn);
    _bend.EncodeTo(coder);
    
    coder.WriteInt8(_pitch.midi);
    coder.WriteInt8(_pitch.step);
    coder.WriteInt8(_pitch.octave);
    coder.WriteInt8(_pitch.alter);
    coder.WriteInt8(_repCT.line);          
    //coder.WriteInt8(_repCT.staff);         
    coder.WriteInt8(_repCT.accidental);    
    coder.WriteInt8(_repCT.noteHeadSymbol);
    coder.WriteUInt8(_repCT.accidentalOffset);    
    coder.WriteUInt8(_repCT.flags);    

    coder.WriteInt8(_repIT.line);          
    //coder.WriteInt8(_repIT.staff);         
    coder.WriteInt8(_repIT.accidental);    
    coder.WriteInt8(_repIT.noteHeadSymbol);
    coder.WriteUInt8(_repIT.accidentalOffset);
    coder.WriteUInt8(_repIT.flags);
    
    uint8_t noteCount = _graceNotes.size();
    coder.WriteUInt8(noteCount);
    for(uint8_t i=0; i<noteCount; ++i) {
        const REGraceNote* note = _graceNotes[i];
        note->EncodeTo(coder);
    }
}

void RENote::DecodeFrom(REInputStream& decoder)
{
    _flags = decoder.ReadUInt16();
    _fret = decoder.ReadInt8();
    _string = decoder.ReadInt8();
    _liveVelocity = decoder.ReadInt8();
    _liveTickDuration = decoder.ReadInt16();
    _liveTickOffset = decoder.ReadInt16();
    _enharmonicHints = decoder.ReadUInt8();
    _slideOut = (Reflow::SlideOutType)decoder.ReadInt8();
    _slideIn = (Reflow::SlideInType) decoder.ReadInt8();
    _bend.DecodeFrom(decoder);
    
    _pitch.midi = decoder.ReadInt8();
    _pitch.step = decoder.ReadInt8();
    _pitch.octave = decoder.ReadInt8();
    _pitch.alter = decoder.ReadInt8();
    _repCT.line = decoder.ReadInt8();          
    if(decoder.Version() < REFLOW_IO_VERSION_1_5_2) {
        /*_repCT.staff =*/ decoder.ReadInt8();         
    }
    _repCT.accidental = decoder.ReadInt8();    
    _repCT.noteHeadSymbol = decoder.ReadInt8();
    if(decoder.Version() >= REFLOW_IO_VERSION_1_5_2) {
        _repCT.accidentalOffset = decoder.ReadUInt8();
        _repCT.flags = decoder.ReadUInt8();
    }
    else {
        _repCT.accidentalOffset = 0;
        _repCT.flags = 0;
    }
    
    _repIT.line = decoder.ReadInt8();          
    if(decoder.Version() < REFLOW_IO_VERSION_1_5_2) {
        /*_repIT.staff =*/ decoder.ReadInt8();         
    }
    _repIT.accidental = decoder.ReadInt8();    
    _repIT.noteHeadSymbol = decoder.ReadInt8();
    if(decoder.Version() >= REFLOW_IO_VERSION_1_5_2) {
        _repIT.accidentalOffset = decoder.ReadUInt8();
        _repIT.flags = decoder.ReadUInt8();
    }
    else {
        _repIT.accidentalOffset = 0;
        _repIT.flags = 0;
    }
    
    if(decoder.Version() >= REFLOW_IO_VERSION_1_7_0)
    {
        uint8_t noteCount = decoder.ReadUInt8();
        for(uint8_t i=0; i<noteCount; ++i)
        {
            REGraceNote* note = new REGraceNote;
            note->Retain();
            note->_index = i;
            note->_parent = NULL;
            _graceNotes.push_back(note);
            
            note->DecodeFrom(decoder);
        }
    }
}


REGraceNoteMetrics::REGraceNoteMetrics()
: _width(0.0f)
{
}

float REGraceNoteMetrics::XOffsetOfNote(int column) const
{
    return column >= 0 && column < ColumnCount() ? _columns[column].noteX : 0.0f;
}

float REGraceNoteMetrics::XOffsetOfAccidental(int column) const
{
    return column >= 0 && column < ColumnCount() ? _columns[column].accidentalX : 0.0f;
}
