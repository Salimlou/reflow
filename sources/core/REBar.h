//
//  REBar.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REBAR_H_
#define _REBAR_H_

#include "RETypes.h"
#include "REChordName.h"

class REBar
{
	friend class RESong;
    friend class REArchive;
    
public:
    typedef std::pair<int, REChordName> REChordNameTickPair;
    typedef std::vector<REChordNameTickPair> REBarChordNameVector;
    
public:
    enum BarFlag {
        RehearsalSign = 0x01,
        RepeatStart = 0x02,
        RepeatEnd = 0x04
    };
	
public:
    REBar();
    ~REBar();
  
public:
	const RESong* Song() const {return _parent;}
	RESong* Song() {return _parent;}
	
	int Index() const {return _index;}
    
    const REBar* PreviousBar() const;
    REBar* PreviousBar();
    
    REBar* Clone(unsigned long cloneOptions=0) const;
    void CopyFrom(const REBar& bar);
	
    const RETimeSignature& TimeSignature() const {return _timeSignature;}
    void SetTimeSignature(const RETimeSignature& ts) {_timeSignature = ts;}
    void SetTimeSignatureNumerator(unsigned int num) {_timeSignature.numerator = num;}
    void SetTimeSignatureDenominator(unsigned int den) {_timeSignature.denominator = den;}
    
    const REBeamingPattern& BeamingPattern() const {return _beamingPattern;}
    void SetBeamingPattern(const REBeamingPattern& pattern) {_beamingPattern = pattern;}
    
    const REKeySignature& KeySignature() const {return _keySignature;}
    void SetKeySignature(const REKeySignature& ks) {_keySignature=ks;}
    
    int AccidentalCountOnKeySignature() const;
    
    RETimeDiv TheoricDuration() const;
    unsigned long TheoricDurationInTicks() const;
    unsigned long OffsetInTicks() const;
    
    bool HasTimeSignatureChange() const;
    bool HasKeySignatureChange() const;
    bool HasTempoMarker() const;
    
    bool HasFlag(BarFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(BarFlag flag) {_flags |= flag;}
    void UnsetFlag(BarFlag flag) {_flags &= ~flag;}

    bool HasDirectionJump(Reflow::DirectionJump flag) const {return 0 != (_directionJump & (1<<flag));}
    void SetDirectionJump(Reflow::DirectionJump flag) {_directionJump |= (1<<flag);}
    void UnsetDirectionJump(Reflow::DirectionJump flag) {_directionJump &= ~(1<<flag);}
    void ClearDirectionJumps() {_directionJump = 0;}
    bool HasDaCapoOrDalSegnoOrDalSegnoSegno() const {return 0 != (_directionJump & ((1 << Reflow::ToCoda)-1));}
    Reflow::DirectionJump FindDaCapoOrDalSegnoOrDalSegnoSegno() const;
    
    bool HasDirectionTarget(Reflow::DirectionTarget flag) const {return 0 != (_directionTarget & (1<<flag));}
    void SetDirectionTarget(Reflow::DirectionTarget flag) {_directionTarget |= (1<<flag);}
    void UnsetDirectionTarget(Reflow::DirectionTarget flag) {_directionTarget &= ~(1<<flag);}
    
    bool HasAlternateEnding(uint8_t ending) const {return 0 != (_alternateEndings & (1<<ending));}
    bool HasAnyAlternateEnding() const {return 0 != _alternateEndings;}
    void SetAlternateEnding(uint8_t ending) {_alternateEndings |= (1<<ending);}
    void UnsetAlternateEnding(uint8_t ending) {_alternateEndings &= ~(1<<ending);}
    
    const std::string& RehearsalSignText() const {return _rehearsalText;}
    void SetRehearsalSignText(const std::string &txt) {_rehearsalText = txt;}
    
    void SetRepeatCount(unsigned int repeatCount) {_repeatCount = repeatCount;}
    unsigned int RepeatCount() const {return _repeatCount;}
    
    int ChordNameCount() const {return _chordNames.size();}
    const REChordName* ChordNameAtIndex(int idx) const;
    const REChordName* ChordNameAtTick(int tick) const;
    void InsertChordName(int tick, const REChordName& chordName);
    bool HasChordNameAtTick(int tick) const;
    int IndexOfChordNameAtTick(int tick) const;
    int TickOfChordNameAtIndex(int idx) const;
    void RemoveAllChordNames();
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    bool operator==(const REBar& rhs) const;
    bool operator!=(const REBar& rhs) const {return !(*this == rhs);}
    
private:
    RESong* _parent;
	int _index;
    uint32_t _flags;
    std::string _rehearsalText;
    REBarChordNameVector _chordNames;
    RETimeSignature _timeSignature;
    REKeySignature _keySignature;
    uint8_t _repeatCount;
    uint8_t _alternateEndings;
    uint8_t _directionTarget;
    uint16_t _directionJump;
    REBeamingPattern _beamingPattern;
    
    
    mutable unsigned long _offsetInTicks;
};

#endif
