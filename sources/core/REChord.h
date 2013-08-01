//
//  REChord.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RECHORD_H_
#define _RECHORD_H_

#include "RETypes.h"
#include "REObject.h"

class REBeatText
{
    friend class REChord;
    
public:
    REBeatText();
    REBeatText(const std::string& text);
    
    const std::string& TextUTF8() const {return _text;}
    void SetTextUTF8(const std::string& text) {_text = text;}
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
private:
    std::string _text;
    Reflow::TextPositioning _positioning;
};


class REChord
{
	friend class REPhrase;
    friend class RESong;
    friend class REScore;
	friend class REArchive;
    
public:
    enum ChordFlag {
        BeamingStart = (1 << 0),
        BeamingEnd = (1 << 1),
        TupletGrouping = (1 << 2),
        TupletGroupStart = (1 << 3),
        TupletGroupEnd = (1 << 4),
        
        Accent = (1 << 5),
        StrongAccent = (1 << 6),
        Staccato = (1 << 7),
            
        ForceStemUp = (1 << 10),
        ForceStemDown = (1 << 11),
        
        Vibrato = (1 << 12),
        PalmMute = (1 << 13),
        LetRing = (1 << 14),
        
        Brush = (1 << 15),
        Arpeggio = (1 << 16),
        PickStroke = (1 << 17),
        StrumUpwards = (1 << 18),           // Otherwise Strumming is downwards
        Tap = (1 << 19),
        Slap = (1 << 20),
        Pop = (1 << 21),
        
        Text = (1 << 24),
    };
    
public:
    REChord();
    ~REChord();
	
public:
	const RENoteVector& Notes() const {return _notes;}
	RENoteVector& Notes() {return _notes;}
	
	unsigned int NoteCount() const {return (unsigned int)_notes.size();}
    unsigned int NoteCountInStringRange(int firstString, int lastString) const;
    unsigned int NoteCountInLineRange(int firstLine, int lastLine, bool transposing) const;
    
    void FindNotesInStringRange(REConstNoteVector* notes, int firstString, int lastString) const;
    void FindNotesInLineRange(REConstNoteVector* notes, int firstLine, int lastLine, bool transposing) const;
    
	const RENote* Note(int idx) const;
	RENote* Note(int idx);
    
    const RESymbolVector& Symbols() const {return _symbols;}
    inline int SymbolCount() const {return (int)_symbols.size();}
    RESymbol* Symbol(int index);
    const RESymbol* Symbol(int index) const;
    int IndexOfSymbol(const RESymbol* symbol) const;
    RESymbol* TakeSymbolAtIndex(int index);
    void RemoveSymbol(const RESymbol* symbol);
    void RemoveSymbolAtIndex(int index);
    void AddSymbol(RESymbol* symbol);
	
	void Clear();
	
	int Index() const {return _index;}
    bool IsRest() const;
    
    REChord* Clone() const;
    REChord* CloneKeepingNotesInStringRange(int firstString, int lastString) const;
	REChord* CloneKeepingNotesInLineRange(int firstLine, int lastLine, bool transposing) const;
    
	const REPhrase* Phrase() const {return _parent;}
	REPhrase* Phrase() {return _parent;}
	
	void InsertNote(RENote* note, int idx);
	void RemoveNote(int idx);
    
    unsigned long DurationInTicks() const;
    unsigned long OffsetInTicks() const;
    
    const RETimeDiv& Duration() const {return _duration;}
    const RETimeDiv& Offset() const {return _offset;}
    
  	RELocator Locator() const;
    
    const REChord* NextSibling() const;
    const REChord* PreviousSibling() const;
    REChord* NextSibling();
    REChord* PreviousSibling();
    
    REChord* NextSiblingOverBarline();
    REChord* PreviousSiblingOverBarline();
    const REChord* NextSiblingOverBarline() const;
    const REChord* PreviousSiblingOverBarline() const;
    
    REChord* NextSiblingOverMultipleBarlines();
    const REChord* NextSiblingOverMultipleBarlines() const;
    
    void PerformOperationOnAllNotes(RENoteOperation op);
    void PerformOperationOnNotesInStringRange(RENoteOperation op, int firstString, int lastString);
    void PerformOperationOnNotesInLineRange(RENoteOperation op, int firstLine, int lastLine, bool transposing);
    bool AtLeastOneNoteVerifies(RENotePredicate pred) const;
    bool AtLeastOneNoteInStringRangeVerifies(RENotePredicate pred, int firstString, int lastString) const;
    bool AtLeastOneNoteInLineRangeVerifies(RENotePredicate pred, int firstLine, int lastLine, bool transposing) const;
    
    Reflow::NoteValue NoteValue() const {return _noteValue;}
    void SetNoteValue(Reflow::NoteValue noteValue);
    void IncreaseNoteValue();
    void DecreaseNoteValue();
    
    void CopyRhythmFrom(const REChord& chord);
    
    unsigned int Dots() const {return _dots;}
    void SetDots(unsigned int dots);
    
    RETuplet Tuplet() const {return _tuplet;}
    bool HasValidTuplet() const {return _tuplet.IsValid();}
    void SetTuplet(const RETuplet& tuplet);
    
    bool HasNoteOnString(unsigned int stringIndex) const;
    const RENote* NoteOnString(unsigned int stringIndex) const;
    RENote* NoteOnString(unsigned int stringIndex);
    int FindUnusedStringForMidi(int midi, int* fret) const;
    
    RENote* NoteWithMidi(int midi);
    const RENote* NoteWithMidi(int midi) const;
    const RENote* FindNoteWithNoteFlag(unsigned long flag) const;
    
    const RENote* NoteOnStaffLine(int staffLine, bool transposingScore) const;
    RENote* NoteOnStaffLine(int staffLine, bool transposingScore);
    
    Reflow::OttaviaType FindOttaviaApplied() const;
    
    Reflow::DynamicsType Dynamics() const {return (Reflow::DynamicsType)_dynamics;}
    void SetDynamics(Reflow::DynamicsType dynamics) {_dynamics = dynamics;}
    
    bool HasTextAttached() const;
    
    std::string TextAttached() const;
    Reflow::TextPositioning TextPositioning() const;
    
    void SetTextAttached(const std::string& text);
    void SetTextPositioning(Reflow::TextPositioning positioning);
    
    //RENotePitch PitchOnStaffLine(int staffLine, int staff, bool transposingScore, Reflow::Accidental accidental) const;
    
    void CalculateLineRange(bool transposed, int* outMinLine, int* outMaxLine) const;
	void CalculateStringRange(int* outMinString, int* outMaxString) const;
    
    bool HasFlag(ChordFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(ChordFlag flag) {_flags |= flag;}
    void UnsetFlag(ChordFlag flag) {_flags &= ~flag;}
    const REChord* FindNextWithFlag(ChordFlag flag) const;
    
    bool IsStemDirectionAutomatic() const {return !HasFlag(REChord::ForceStemUp) && !HasFlag(REChord::ForceStemDown);}
    bool HasVerticallyStackedSeconds(bool withStemUp, bool transposed) const;
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    void EncodeKeepingNotesInStringRange(REOutputStream& coder, int firstString, int lastString) const;
    void EncodeKeepingNotesInLineRange(REOutputStream& coder, int firstLine, int lastLine, bool transposing) const;

    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);

    void _CalculateSpacing(float* leftSpacing, float* rightSpacing, float unitSpacing, bool transposing) const;
    
public:
    bool operator==(const REChord& rhs) const;
    bool operator!=(const REChord& rhs) const {return !(*this == rhs);}
    
private:
	void _UpdateIndices();
    void _RefreshDuration() const;

private:
	REPhrase* _parent;
	RENoteVector _notes;
    RESymbolVector _symbols;
    int _index;
    Reflow::NoteValue _noteValue;
    int8_t _dots;
    int8_t _dynamics;
    RETuplet _tuplet;
    REBeatText* _text;
    uint32_t _flags;
    
    mutable RETimeDiv _duration;
    mutable RETimeDiv _offset;
};


#endif
