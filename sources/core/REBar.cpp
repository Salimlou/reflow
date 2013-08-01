//
//  REBar.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REBar.h"
#include "RESong.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REFunctions.h"

REBar::REBar()
: _parent(0), _index(-1), _flags(0), _repeatCount(0), _offsetInTicks(0), _alternateEndings(0), _directionTarget(0), _directionJump(0)
{
    _beamingPattern.Clear();
}

REBar::~REBar()
{
    
}

bool REBar::operator==(const REBar& rhs) const {
    return true;
}

const REBar* REBar::PreviousBar() const
{
    if(_parent && _index > 0) {
        return _parent->Bar(_index-1);
    }
    return 0;
}
REBar* REBar::PreviousBar()
{
    if(_parent && _index > 0) {
        return _parent->Bar(_index-1);
    }
    return 0;    
}

unsigned long REBar::OffsetInTicks() const
{
    return _offsetInTicks;
}

RETimeDiv REBar::TheoricDuration() const
{
    return RETimeDiv(4*_timeSignature.numerator, _timeSignature.denominator);
}

unsigned long REBar::TheoricDurationInTicks() const
{
    return Reflow::TimeDivToTicks(TheoricDuration());
}

bool REBar::HasTimeSignatureChange() const
{
    const REBar* prev = PreviousBar();
    if(prev) {
        return prev->TimeSignature() != TimeSignature();
    }
    return true;
}

bool REBar::HasKeySignatureChange() const
{
    const REBar* prev = PreviousBar();
    if(prev) {
        return prev->KeySignature() != KeySignature();
    }
    return true;
}

bool REBar::HasTempoMarker() const
{
    const RETempoTimeline& tempoMarkers = _parent->TempoTimeline();
    return tempoMarkers.HasItemInBarRange(Index(), Index());
}

int REBar::AccidentalCountOnKeySignature() const
{
    const REBar* prev = PreviousBar();
    int accidentals[7];
    int naturals[7];
    int nbAccidentals = _keySignature.DetermineLinesOfAccidentals(Reflow::TrebleClef, accidentals);
    int nbNaturals = (prev ? _keySignature.DetermineLinesOfNaturals(Reflow::TrebleClef, prev->KeySignature(), naturals) : 0);
    return nbAccidentals + nbNaturals;
}

const REChordName* REBar::ChordNameAtIndex(int idx) const
{
    if(idx >= 0 && idx < _chordNames.size()) {
        return &_chordNames[idx].second;
    }
    return NULL;
}

const REChordName* REBar::ChordNameAtTick(int tick) const
{
    return ChordNameAtIndex(IndexOfChordNameAtTick(tick));
}

void REBar::InsertChordName(int tick, const REChordName& chordName)
{
    int idx = IndexOfChordNameAtTick(tick);
    if(idx == -1)
    {
        idx = 0;
        int count = _chordNames.size();
        while(idx < count && _chordNames[idx].first < tick) {
            ++idx;
        }
        _chordNames.insert(_chordNames.begin()+idx, REChordNameTickPair(tick, chordName));
    }
    else {
        _chordNames[idx] = REChordNameTickPair(tick, chordName);
    }
}

void REBar::RemoveAllChordNames()
{
    _chordNames.clear();
}

bool REBar::HasChordNameAtTick(int tick) const
{
    REBarChordNameVector::const_iterator it = _chordNames.begin();
    for(; it != _chordNames.end(); ++it) {
        const REChordNameTickPair& p = *it;
        if(p.first == tick) {
            return true;
        }
    }
    return false;
}

int REBar::IndexOfChordNameAtTick(int tick) const
{
    int chordNameCount = _chordNames.size();
    for(int i=0; i<chordNameCount; ++i) {
        const REChordNameTickPair& p = _chordNames[i];
        if(p.first == tick) {
            return i;
        }
    }
    return -1;
}

int REBar::TickOfChordNameAtIndex(int idx) const
{
    if(idx >= 0 && idx < _chordNames.size()) {
        return _chordNames[idx].first;
    }
    return 0;
}

Reflow::DirectionJump REBar::FindDaCapoOrDalSegnoOrDalSegnoSegno() const
{
    if      (_directionJump & 0x0001) return Reflow::DaCapo;
    else if (_directionJump & 0x0002) return Reflow::DaCapo_AlFine;
    else if (_directionJump & 0x0004) return Reflow::DaCapo_AlCoda;
    else if (_directionJump & 0x0008) return Reflow::DaCapo_AlDoubleCoda;
    else if (_directionJump & 0x0010) return Reflow::DalSegno;
    else if (_directionJump & 0x0020) return Reflow::DalSegno_AlFine;
    else if (_directionJump & 0x0040) return Reflow::DalSegno_AlCoda;
    else if (_directionJump & 0x0080) return Reflow::DalSegno_AlDoubleCoda;
    else if (_directionJump & 0x0100) return Reflow::DalSegnoSegno;
    else if (_directionJump & 0x0200) return Reflow::DalSegnoSegno_AlFine;
    else if (_directionJump & 0x0400) return Reflow::DalSegnoSegno_AlCoda;
    else if (_directionJump & 0x0800) return Reflow::DalSegnoSegno_AlDoubleCoda;
    
    return Reflow::DaCapo;
}

REBar* REBar::Clone(unsigned long cloneOptions) const
{
    REBufferOutputStream coder;
    EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REBar* clonedBar = new REBar;
    clonedBar->DecodeFrom(decoder);
    
    if(cloneOptions & Reflow::IgnoreRehearsals) {
        clonedBar->UnsetFlag(REBar::RehearsalSign);
        clonedBar->SetRehearsalSignText("");
    }
    
    return clonedBar;
}

void REBar::CopyFrom(const REBar& bar)
{
    REBufferOutputStream coder;
    bar.EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    DecodeFrom(decoder);
}

void REBar::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    writer.EndObject();
}

void REBar::ReadJson(const REJsonValue& obj, uint32_t version)
{
}

void REBar::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt32(_flags);
    coder.WriteUInt8(_alternateEndings);
    coder.WriteUInt8(_directionTarget);
    coder.WriteUInt16(_directionJump);    
    
    coder.WriteUInt8(_timeSignature.numerator);
    coder.WriteUInt8(_timeSignature.denominator);
    
    coder.WriteInt8(_keySignature.key);
    coder.WriteUInt8(_keySignature.minor ? 1 : 0);
    
    coder.WriteInt8(_chordNames.size());
    for(int i=0; i<_chordNames.size(); ++i) {
        const REChordNameTickPair& p = _chordNames[i];
        coder.WriteInt32(p.first);
        p.second.EncodeTo(coder);
    }
    
    if(HasFlag(REBar::RehearsalSign)) {
        coder.WriteString(_rehearsalText);
    }
    
    if(HasFlag(REBar::RepeatEnd)) {
        coder.WriteUInt8(_repeatCount);
    }
    
    int groupCount = _beamingPattern.GroupCount();
    coder.WriteUInt8(groupCount);
    for(int i=0; i<groupCount; ++i) {
        coder.WriteUInt8(_beamingPattern.Group(i));
    }
}
void REBar::DecodeFrom(REInputStream& decoder)
{
    _flags = decoder.ReadUInt32();
    _alternateEndings = decoder.ReadUInt8();
    _directionTarget = decoder.ReadUInt8();
    _directionJump = decoder.ReadUInt16();
    
    _timeSignature.numerator = decoder.ReadUInt8();
    _timeSignature.denominator = decoder.ReadUInt8();
    
    _keySignature.key = decoder.ReadInt8();
    _keySignature.minor = (decoder.ReadUInt8() ? 1 : 0);
    
    int nbChordNames = decoder.ReadInt8();
    _chordNames.clear();
    for(int i=0; i<nbChordNames; ++i) {
        int tick = decoder.ReadInt32();
        REChordName chordName; chordName.DecodeFrom(decoder);
        _chordNames.push_back(REChordNameTickPair(tick, chordName));
    }
    
    if(HasFlag(REBar::RehearsalSign)) {
        _rehearsalText = decoder.ReadString();
    }
    
    if(HasFlag(REBar::RepeatEnd)) {
        _repeatCount = decoder.ReadUInt8();
    }
    
    // Reflow 1.7 introduces Beaming Pattern
    if(decoder.Version() >= REFLOW_IO_VERSION_1_7_0)
    {
        _beamingPattern._groupCount = decoder.ReadUInt8();
        for(int i=0; i<_beamingPattern._groupCount; ++i)
        {
            _beamingPattern._groups[i] = decoder.ReadUInt8();
        }
    }
    else
    {
        _beamingPattern.Clear();
    }
}