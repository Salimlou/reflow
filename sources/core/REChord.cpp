//
//  REChord.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REChord.h"
#include "RENote.h"
#include "REPhrase.h"
#include "RETrack.h"
#include "RELocator.h"
#include "REFunctions.h"
#include "RESymbol.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#include <algorithm>

REChord::REChord()
: _parent(0), _index(-1), _noteValue(Reflow::QuarterNote), _dots(0), _tuplet(0,0), _flags(0), _text(NULL), _dynamics(Reflow::DynamicsUndefined)
{
    _RefreshDuration();
#ifdef REFLOW_2
    REPickstrokeSymbol* sym = new REPickstrokeSymbol;
    sym->SetOffset(REPoint(0, -10));
    _symbols.push_back(sym);
    
    RETextSymbol* ts = new RETextSymbol;
    ts->SetOffset(REPoint(0, 25));
    ts->SetText("pouet");
    ts->SetFont({std::string("Times New Roman"), 10.0, false, false});
    ts->SetBounds(Reflow::BoundsOfText(ts->Text(), ts->Font()));
    _symbols.push_back(ts);
#endif
}

REChord::~REChord()
{
    Clear();
}

const RENote* REChord::Note(int idx) const
{
    if(idx >= 0 && idx < _notes.size()) {
        return _notes[idx];
    }
    return 0;
}

RENote* REChord::Note(int idx)
{
    if(idx >= 0 && idx < _notes.size()) {
        return _notes[idx];
    }
    return 0;    
}

const REChord* REChord::NextSibling() const
{
    const REPhrase* phrase = Phrase();
    if(!phrase) return 0;
    
    return phrase->Chord(Index()+1);
}

const REChord* REChord::PreviousSibling() const
{
    const REPhrase* phrase = Phrase();
    if(!phrase) return 0;
    
    return phrase->Chord(Index()-1);
}

REChord* REChord::NextSibling()
{
    REPhrase* phrase = Phrase();
    if(!phrase) return 0;
    
    return phrase->Chord(Index()+1);
}

REChord* REChord::PreviousSibling()
{
    REPhrase* phrase = Phrase();
    if(!phrase) return 0;
    
    return phrase->Chord(Index()-1);
}

REChord* REChord::NextSiblingOverMultipleBarlines()
{
    REChord* chord = NextSibling();
    if(chord == NULL) {
        REPhrase* ph = Phrase()->NextSibling();
        while(ph && ph->ChordCount() == 0) {
            ph = ph->NextSibling();
        }
        if(ph) {
            chord = ph->FirstChord();
        }
    }
    return chord;
}

const REChord* REChord::NextSiblingOverMultipleBarlines() const
{
    const REChord* chord = NextSibling();
    if(chord == NULL) {
        const REPhrase* ph = Phrase()->NextSibling();
        while(ph && ph->ChordCount() == 0) {
            ph = ph->NextSibling();
        }
        if(ph) {
            chord = ph->FirstChord();
        }
    }
    return chord;
}

REChord* REChord::NextSiblingOverBarline()
{
    REChord* chord = NextSibling();
    if(chord == NULL) {
        REPhrase* ph = Phrase()->NextSibling();
        if(ph) {
            chord = ph->FirstChord();
        }
    }
    return chord;
}

REChord* REChord::PreviousSiblingOverBarline()
{
    REChord* chord = PreviousSibling();
    if(chord == NULL) {
        REPhrase* ph = Phrase()->PreviousSibling();
        if(ph) {
            chord = ph->LastChord();
        }
    }
    return chord;
}

const REChord* REChord::NextSiblingOverBarline() const
{
    const REChord* chord = NextSibling();
    if(chord == NULL) {
        const REPhrase* ph = Phrase()->NextSibling();
        if(ph) {
            chord = ph->FirstChord();
        }
    }
    return chord;
}
const REChord* REChord::PreviousSiblingOverBarline() const
{
    const REChord* chord = PreviousSibling();
    if(chord == NULL) {
        const REPhrase* ph = Phrase()->PreviousSibling();
        if(ph) {
            chord = ph->LastChord();
        }
    }
    return chord;
}

RENote* REChord::NoteWithMidi(int midi)
{
    for(RENoteVector::const_iterator it = _notes.begin(); it != _notes.end(); ++it) {
        RENote* note = *it;
        if(note->Pitch().midi == midi) {
            return note;
        }
    }
    return NULL;
}

const RENote* REChord::NoteWithMidi(int midi) const
{
    for(RENoteVector::const_iterator it = _notes.begin(); it != _notes.end(); ++it) {
        const RENote* note = *it;
        if(note->Pitch().midi == midi) {
            return note;
        }
    }
    return NULL;
}

RELocator REChord::Locator() const 
{
    const REPhrase* phrase = Phrase();
    if(!phrase) return RELocator();
    
    RELocator locator = phrase->Locator();
    locator.SetChordIndex(Index());
    return locator;
}

const RESymbol* REChord::Symbol(int index) const
{
    return (index >= 0 && index < _symbols.size() ? _symbols[index] : nullptr);
}

RESymbol* REChord::Symbol(int index)
{
    return (index >= 0 && index < _symbols.size() ? _symbols[index] : nullptr);
}

int REChord::IndexOfSymbol(const RESymbol* symbol) const
{
    for(int i=0; i<_symbols.size(); ++i) {
        if(_symbols[i] == symbol) {
            return i;
        }
    }
    return -1;
}
RESymbol* REChord::TakeSymbolAtIndex(int index)
{
    if(index >= 0 && index < _symbols.size())
    {
        RESymbol* symbol = _symbols[index];
        _symbols.erase(_symbols.begin() + index);
        return symbol;
    }
    return nullptr;
}

void REChord::RemoveSymbol(const RESymbol* symbol)
{
    RemoveSymbolAtIndex(IndexOfSymbol(symbol));
}

void REChord::RemoveSymbolAtIndex(int index)
{
    RESymbol* symbol = TakeSymbolAtIndex(index);
    delete symbol;
}

void REChord::AddSymbol(RESymbol* symbol)
{
    if(symbol) _symbols.push_back(symbol);
}

bool REChord::HasTextAttached() const {
    return HasFlag(REChord::Text) && _text != NULL && !_text->TextUTF8().empty();
}

std::string REChord::TextAttached() const {
    if(HasTextAttached()) {
        return _text->TextUTF8();
    }
    return "";
}

void REChord::SetTextAttached(const std::string& text) {
    if(_text) {
        delete _text; _text = NULL;
    }
    
    if(!text.empty()) {
        _text = new REBeatText(text);
        SetFlag(REChord::Text);
    }
    else {
        UnsetFlag(REChord::Text);
    }
}

Reflow::TextPositioning REChord::TextPositioning() const 
{
    if(HasTextAttached()) {
        return _text->_positioning;
    }
    return Reflow::TextAboveStandardStaff;
}

void REChord::SetTextPositioning(Reflow::TextPositioning positioning) 
{
    if(HasTextAttached()) {
        _text->_positioning = positioning;
    }
}


void REChord::Clear()
{
    for(RENoteVector::const_iterator it = _notes.begin(); it != _notes.end(); ++it) {
        //delete *it;
        (*it)->Release();
    }
    _notes.clear();
    if(_text) {
        delete _text; _text = NULL;
    }
    
    for(auto sym : _symbols) {delete sym;}
    _symbols.clear();
}

void REChord::InsertNote(RENote *note, int idx)
{
    note->Retain();
    _notes.insert(_notes.begin() + idx, note);
    note->_parent = this;
    _UpdateIndices();
}

void REChord::RemoveNote(int idx)
{
    if(idx >= 0 && idx < _notes.size()) {
        RENote* note = _notes[idx];
        //delete note;
        note->Release();
        _notes.erase(_notes.begin() + idx);
        _UpdateIndices();
    }
}

void REChord::_UpdateIndices() {
    for(unsigned int i=0; i<_notes.size(); ++i) {
        _notes[i]->_index = i;
    }
}

bool REChord::operator==(const REChord &rhs) const {
    if(NoteCount() != rhs.NoteCount()) {
        return false;
    }
    
    for(unsigned int i=0; i<_notes.size(); ++i) {
        if(*Note(i) != *rhs.Note(i)) {
            return false;
        }
    }
    return true;
}

unsigned long REChord::DurationInTicks() const
{
    return Reflow::TimeDivToTicks(_duration);
}

unsigned long REChord::OffsetInTicks() const
{
    return Reflow::TimeDivToTicks(_offset);
}

void REChord::_RefreshDuration() const
{
    switch(_noteValue)
    {
        case Reflow::WholeNote: _duration = RETimeDiv(4); break;
        case Reflow::HalfNote: _duration = RETimeDiv(2); break;
        case Reflow::QuarterNote: _duration = RETimeDiv(1); break;
        case Reflow::EighthNote: _duration = RETimeDiv(1,2); break;
        case Reflow::SixteenthNote: _duration = RETimeDiv(1,4); break;
        case Reflow::ThirtySecondNote: _duration = RETimeDiv(1,8); break;
        case Reflow::SixtyFourthNote: _duration = RETimeDiv(1,16); break;
    }
    
    if(_dots == 1) {
        _duration += _duration/2;
    }
    else if(_dots == 2) {
        _duration += _duration/2 + _duration/4;
    }
    
    if (_tuplet.tuplet != 0 && _tuplet.tupletFor != 0) {
        _duration *= RETimeDiv(_tuplet.tupletFor, _tuplet.tuplet);
    }
}

void REChord::SetNoteValue(Reflow::NoteValue noteValue)
{
    _noteValue = noteValue;
    _RefreshDuration();
}

void REChord::IncreaseNoteValue()
{
    if(_noteValue < Reflow::SixtyFourthNote) {
        _noteValue = static_cast<Reflow::NoteValue>((int)_noteValue + 1);
        _RefreshDuration();
    }
}

void REChord::DecreaseNoteValue()
{
    if(_noteValue > Reflow::WholeNote) {
        _noteValue = static_cast<Reflow::NoteValue>((int)_noteValue - 1);
        _RefreshDuration();
    }    
}

void REChord::SetDots(unsigned int dots)
{
    _dots = dots;
    _RefreshDuration();
}

void REChord::SetTuplet(const RETuplet& tuplet)
{
    _tuplet = tuplet;
    _RefreshDuration();
}

bool REChord::IsRest() const
{
    return _notes.empty();
}

const RENote* REChord::NoteOnString(unsigned int stringIndex) const
{
    for(const RENote* note : _notes) {
        if(note->String() == stringIndex) {
            return note;
        }
    }
    return 0;
}

RENote* REChord::NoteOnString(unsigned int stringIndex)
{
    for(RENote* note : _notes) {
        if(note->String() == stringIndex) {
            return note;
        }
    }
    return 0;
}

bool REChord::HasNoteOnString(unsigned int stringIndex) const
{
    return NoteOnString(stringIndex) != 0;
}

int REChord::FindUnusedStringForMidi(int midi, int* fret) const
{
    const RETrack* track = Locator().Track();
    if(track == NULL) return -1;
    
    unsigned int nbStrings = track->StringCount();
    for(unsigned int stringIndex=0; stringIndex<nbStrings; ++stringIndex) 
    {
        if(HasNoteOnString(stringIndex)) continue;
        
        int tuning = track->TuningForString(stringIndex);
        int dt = midi - tuning;
        if(dt >= 0 && dt <= track->MaxFret()) {
            if(fret != NULL) {
                *fret = dt;
            }
            return (int)stringIndex;
        }
    }
    return -1;
}

const RENote* REChord::NoteOnStaffLine(int staffLine, bool transposingScore) const
{
    for(const RENote* note : _notes) {
        const RENote::REStandardRep& rep = note->Representation(transposingScore);
        if(rep.line == staffLine) {
            return note;
        }
    }
    return NULL;
}


RENote* REChord::NoteOnStaffLine(int staffLine, bool transposingScore)
{
    for(RENote* note : _notes) {
        const RENote::REStandardRep& rep = note->Representation(transposingScore);
        if(rep.line == staffLine) {
            return note;
        }
    }
    return NULL;
}

void REChord::CalculateLineRange(bool transposed, int* outMinLine, int* outMaxLine) const
{
    for(const RENote* note : _notes) {
        const RENote::REStandardRep& rep = note->Representation(transposed);
        if(rep.line < *outMinLine) *outMinLine = rep.line;
        if(rep.line > *outMaxLine) *outMaxLine = rep.line;
    }
}

void REChord::CalculateStringRange(int* outMinString, int* outMaxString) const
{
    for(const RENote* note : _notes)
    {
        int string = note->String();
        if(string < *outMinString) *outMinString = string;
        if(string > *outMaxString) *outMaxString = string;
    }
}

const REChord* REChord::FindNextWithFlag(ChordFlag flag) const
{
    const REChord* chord = this;
    while(chord && !chord->HasFlag(flag)) {
        chord = chord->NextSibling();
    }
    return chord;
}

/*RENotePitch REChord::PitchOnStaffLine(int staffLine, int staff, bool transposingScore, Reflow::Accidental accidental) const
{
    
}*/

Reflow::OttaviaType REChord::FindOttaviaApplied() const
{
    int tick = OffsetInTicks();
    
    RELocator locator = Locator();
    locator.SetVoiceIndex(0);
    const REPhrase* phrase = locator.Phrase();
    if(phrase == NULL) return Reflow::NoOttavia;
    
    const REOttaviaRangeModifier& modifier = phrase->OttaviaModifier();
    const REOttaviaRangeModifierElement* elem = modifier.ItemAppliedAt(tick);
    return (elem ? elem->value : Reflow::NoOttavia);
}

REChord* REChord::Clone() const
{
    REBufferOutputStream coder;
    this->EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REChord* chord = new REChord;
    chord->DecodeFrom(decoder);
    return chord;
}

REChord* REChord::CloneKeepingNotesInStringRange(int firstString, int lastString) const
{
    REBufferOutputStream coder;
    this->EncodeKeepingNotesInStringRange(coder, firstString, lastString);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REChord* chord = new REChord;
    chord->DecodeFrom(decoder);
    return chord;
}

REChord* REChord::CloneKeepingNotesInLineRange(int firstLine, int lastLine, bool transposing) const
{
    REBufferOutputStream coder;
    this->EncodeKeepingNotesInLineRange(coder, firstLine, lastLine, transposing);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REChord* chord = new REChord;
    chord->DecodeFrom(decoder);
    return chord;
}

unsigned int REChord::NoteCountInStringRange(int firstString, int lastString) const
{
    unsigned int count = 0;
    for(RENote* note : _notes) {
        if(firstString <= note->String() && note->String() <= lastString) {++count;}
    }
    return count;
}

unsigned int REChord::NoteCountInLineRange(int firstLine, int lastLine, bool transposing) const
{
    unsigned int count = 0;
    for(RENote* note : _notes) {
        const RENote::REStandardRep &rep = note->Representation(transposing);
        if(firstLine <= rep.line && rep.line <= lastLine) {
            ++count;
        }
    }  
    return count;
}

void REChord::FindNotesInStringRange(REConstNoteVector* notes, int firstString, int lastString) const
{
    for(RENote* note : _notes) {
        if(firstString <= note->String() && note->String() <= lastString) {
            notes->push_back(note);
        }
    }
}
void REChord::FindNotesInLineRange(REConstNoteVector* notes, int firstLine, int lastLine, bool transposing) const
{
    for(RENote* note : _notes) {
        const RENote::REStandardRep &rep = note->Representation(transposing);
        if(firstLine <= rep.line && rep.line <= lastLine) {
            notes->push_back(note);
        }
    }  
}

void REChord::PerformOperationOnAllNotes(RENoteOperation op)
{
    std::for_each(_notes.begin(), _notes.end(), op);
}

void REChord::PerformOperationOnNotesInStringRange(RENoteOperation op, int firstString, int lastString)
{
    for(RENote* note : _notes) {
        if(firstString <= note->String() && note->String() <= lastString) {
            op(note);
        }
    }
}

void REChord::PerformOperationOnNotesInLineRange(RENoteOperation op, int firstLine, int lastLine, bool transposing)
{
    for(RENote* note : _notes)
    {
        const RENote::REStandardRep &rep = note->Representation(transposing);
        if(firstLine <= rep.line && rep.line <= lastLine) {
            op(note);
        }
    }    
}

bool REChord::AtLeastOneNoteVerifies(RENotePredicate pred) const
{
    return std::find_if(_notes.begin(), _notes.end(), pred) != _notes.end();
}

bool REChord::AtLeastOneNoteInStringRangeVerifies(RENotePredicate pred, int firstString, int lastString) const
{
    for(const RENote* note : _notes) {
        if(firstString <= note->String() && note->String() <= lastString && pred(note)) {
            return true;
        }
    }
    return false;
}

bool REChord::AtLeastOneNoteInLineRangeVerifies(RENotePredicate pred, int firstLine, int lastLine, bool transposing) const
{
    for(const RENote* note : _notes)
    {
        const RENote::REStandardRep &rep = note->Representation(transposing);
        if(firstLine <= rep.line && rep.line <= lastLine && pred(note)) {
            return true;
        }
    }
    return false;
}

void REChord::CopyRhythmFrom(const REChord& chord)
{
    SetNoteValue(chord.NoteValue());
    SetDots(chord.Dots());
    SetTuplet(chord.Tuplet());
}

void REChord::_CalculateSpacing(float* leftSpacing, float* rightSpacing, float unitSpacing, bool transposing) const
{
    *leftSpacing = unitSpacing;
    *rightSpacing = unitSpacing;
    
    switch(NoteValue()) 
    {
        case Reflow::WholeNote:     *rightSpacing = 3.5 * unitSpacing; break;
        case Reflow::HalfNote:      *rightSpacing = 3.0 * unitSpacing; break;
        case Reflow::QuarterNote:   *rightSpacing = 2.2 * unitSpacing; break;
        case Reflow::EighthNote:    *rightSpacing = 1.7 * unitSpacing; break;            
        case Reflow::SixteenthNote: *rightSpacing = 1.4 * unitSpacing; break;            
        default:                    *rightSpacing = 1.1 * unitSpacing; break;            
    }
    
    float accidentalOffset = 0.0;
    float graceNoteOffset = 0.0;
    bool oneNoteIsGhost = false;
    for(const RENote* note : _notes)
    {
        const RENote::REStandardRep &rep = note->Representation(transposing);
        
        if(rep.accidental == Reflow::DoubleFlat) {
            accidentalOffset = 1.6 * unitSpacing + (unitSpacing * rep.accidentalOffset);
        }
        else if(rep.accidental != Reflow::NoAccidental) {
            float offset = 0.8 * unitSpacing + (unitSpacing * rep.accidentalOffset);
            if(accidentalOffset < offset) {
                accidentalOffset = offset;
            }
        }
        
        // Ghost Note
        if(note->HasFlag(RENote::GhostNote)) {
            oneNoteIsGhost = true;
        }
        
        // Bend ?
        if(note->HasBend()) {
            *rightSpacing += 2.0 * unitSpacing;
        }
        
        // Grace Note
        if(note->GraceNoteCount() > 0)
        {
            float offset = note->CalculateGraceNoteMetrics(transposing, unitSpacing, nullptr);
            if(offset > graceNoteOffset) graceNoteOffset = offset;
        }
    }
    
    
    if(oneNoteIsGhost) *leftSpacing += unitSpacing;
    *leftSpacing += accidentalOffset;
    
    if(HasFlag(REChord::Brush) || HasFlag(REChord::Arpeggio)) {
        *leftSpacing += 2.0 * unitSpacing;
    }
    
    if(graceNoteOffset > 0) {
        *leftSpacing += graceNoteOffset;
    }
    
    if(Dots() >= 1) {
        *rightSpacing += unitSpacing;
    }
    if(Dots() == 2) {
        *rightSpacing += unitSpacing;
    }
}

bool REChord::HasVerticallyStackedSeconds(bool withStemUp, bool transposed) const
{
    if(withStemUp)
    {
        for(const RENote* note : _notes) {
            const RENote::REStandardRep &rep = note->Representation(transposed);
            if(rep.flags & RENote::StackedSecondOnOppositeSideWithStemUp) {
                return true;
            }
        }
    }
    else 
    {
        for(const RENote* note : _notes) {
            const RENote::REStandardRep &rep = note->Representation(transposed);
            if(rep.flags & RENote::StackedSecondOnOppositeSideWithStemDown) {
                return true;
            }
        }        
    }
    return false;
}

void REChord::EncodeKeepingNotesInStringRange(REOutputStream& coder, int firstString, int lastString) const
{
    coder.WriteUInt8(_noteValue);
    coder.WriteUInt8(_dots);
    coder.WriteUInt8(_tuplet.tuplet);
    coder.WriteUInt8(_tuplet.tupletFor);
    coder.WriteUInt32(_flags);
    coder.WriteInt8(_dynamics);
    
    uint8_t noteCount = _notes.size();
    std::vector<const RENote*> notesToKeep;
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = _notes[i];
        if(note->String() >= firstString && note->String() <= lastString) {
            notesToKeep.push_back(note);
        }
    }
    
    noteCount = notesToKeep.size();
    coder.WriteUInt8(noteCount);
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = notesToKeep[i];
        note->EncodeTo(coder);
    }
    
    if(HasTextAttached()) {
        _text->EncodeTo(coder);
    }
}

void REChord::EncodeKeepingNotesInLineRange(REOutputStream &coder, int firstLine, int lastLine, bool transposing) const
{
    coder.WriteUInt8(_noteValue);
    coder.WriteUInt8(_dots);
    coder.WriteUInt8(_tuplet.tuplet);
    coder.WriteUInt8(_tuplet.tupletFor);
    coder.WriteUInt32(_flags);
    coder.WriteInt8(_dynamics);
    
    uint8_t noteCount = _notes.size();
    std::vector<const RENote*> notesToKeep;
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = _notes[i];
        const RENote::REStandardRep& rep = note->Representation(transposing);
        if(rep.line >= firstLine && rep.line <= lastLine) {
            notesToKeep.push_back(note);
        }
    }
    
    noteCount = notesToKeep.size();
    coder.WriteUInt8(noteCount);
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = notesToKeep[i];
        note->EncodeTo(coder);
    }
    
    if(HasTextAttached()) {
        _text->EncodeTo(coder);
    }
}

void REChord::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt8(_noteValue);
    coder.WriteUInt8(_dots);
    coder.WriteUInt8(_tuplet.tuplet);
    coder.WriteUInt8(_tuplet.tupletFor);
    coder.WriteUInt32(_flags);
    coder.WriteInt8(_dynamics);
    
    uint8_t noteCount = _notes.size();
    coder.WriteUInt8(noteCount);
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = _notes[i];
        note->EncodeTo(coder);
    }
    
    if(HasTextAttached()) {
        _text->EncodeTo(coder);
    }
}
void REChord::DecodeFrom(REInputStream& decoder)
{
    Clear();
    
    _noteValue = (Reflow::NoteValue)decoder.ReadUInt8();
    _dots = decoder.ReadUInt8();
    _tuplet.tuplet = decoder.ReadUInt8();
    _tuplet.tupletFor = decoder.ReadUInt8();
    _flags = decoder.ReadUInt32();
    _dynamics = decoder.ReadInt8();
        
    uint8_t noteCount = decoder.ReadUInt8();
    for(uint8_t i=0; i<noteCount; ++i) {
        RENote* note = new RENote;
        note->Retain();
        note->_index = i;
        note->_parent = this;
        _notes.push_back(note);
        
        note->DecodeFrom(decoder);
    }
    
    if(HasFlag(REChord::Text)) {
        _text = new REBeatText;
        _text->DecodeFrom(decoder);
    }
    
    _RefreshDuration();
}




REBeatText::REBeatText() :_text(""), _positioning(Reflow::TextAboveStandardStaff) {}

REBeatText::REBeatText(const std::string& text) 
    : _text(text), _positioning(Reflow::TextAboveStandardStaff)
{}

void REBeatText::EncodeTo(REOutputStream& coder) const {
    coder.WriteString(_text);
    coder.WriteInt8(_positioning);
}

void REBeatText::DecodeFrom(REInputStream& decoder) {
    _text = decoder.ReadString();
    _positioning = (Reflow::TextPositioning) decoder.ReadInt8();
}
