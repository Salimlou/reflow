//
//  RETrack.h
//  ReflowMac
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RETRACK_H_
#define _RETRACK_H_

#include "RETypes.h"
#include "RETimeline.h"
#include "REMultivoiceIterator.h"
#include "REPitchClass.h"
#include "RESlur.h"

class REBot;

class RETrack
{
	friend class RESong;
    friend class REGuitarProParser;
    friend class RESequencer;
    friend class REArchive;
    friend class REScoreController;
	
public:
    enum TrackFlag {
        GrandStaff = (1 << 0),
        BanjoDroneString = (1 << 1)
    };
    
public:
    RETrack();
    RETrack(Reflow::TrackType type);
    ~RETrack();
    
public:
    const REVoiceVector& Voices() const {return _voices;}
    REVoiceVector& Voices() {return _voices;}
    
    unsigned int VoiceCount() const {return (unsigned int)_voices.size();}
    const REVoice* Voice(int idx) const;
    REVoice* Voice(int idx);
    
	const RESong* Song() const {return _parent;}
	RESong* Song() {return _parent;}
    
    Reflow::TrackType Type() const {return _type;}
    bool IsDrums() const {return _type == Reflow::DrumsTrack;}
    bool IsTablature() const {return _type == Reflow::TablatureTrack;}
    bool IsStandard() const {return _type == Reflow::StandardTrack;}
    
    Reflow::TablatureInstrumentType TablatureInstrumentType() const {return _tablatureInstrumentType;}
    void SetTablatureInstrumentType(Reflow::TablatureInstrumentType tablatureType) {_tablatureInstrumentType = tablatureType;}
    
    int Capo() const {return _capo;}
    void SetCapo(int capo) {_capo = capo;}
    
    unsigned int MaxFret() const {return 30;}
    
    const std::string& Name() const {return _name;}
    void SetName(const std::string& name) {_name=name;}
    
    const std::string& ShortName() const {return _shortName;}
    void SetShortName(const std::string& shortName) {_shortName=shortName;}
    
    void SetMIDIProgram(int midiProgram) {_midiProgram=midiProgram;}
    int MIDIProgram() const {return _midiProgram;}
    
    void SetSolo (bool solo) {_solo = solo;}
    bool IsSolo () const {return _solo;}
    
    void SetMute (bool mute) {_mute = mute;}
    bool IsMute () const {return _mute;}
    
    void SetVolume(float volume) {_volume = volume;}
    float Volume() const {return _volume;}
    
    void SetPan (float pan) {_pan = pan;}
    float Pan () const {return _pan;}
    
    bool IsGrandStaff() const {return HasFlag(RETrack::GrandStaff);}
    
    unsigned int StringCount() const;
    void SetStringCount(unsigned int stringCount);
    unsigned int TuningForString(unsigned int stringIndex) const;
    void SetTuningForString(unsigned int stringIndex, uint8_t midi);
	void SetTuning(const int* tuningArray, int stringCount);
    const uint8_t * TuningArray() const {return _tuning.data();}
    std::string TuningNoteNames() const;
    
    void SetTransposingInterval(const REPitchClass transposingInterval) {_transposingInterval = transposingInterval;}
    const REPitchClass& TransposingInterval() const {return _transposingInterval;}
    
	void Clear();
    void Refresh();
    RETrack* Clone();
	
	int Index() const {return _index;}
    
	void InsertVoice(REVoice* voice, int idx);
	void RemoveVoice(int idx);
    
    bool IsBarEmptyOrRest(int barIndex) const;
    bool IsBarEmptyAndCollapsibleWithNextSibling(int barIndex) const;
    int MaxDurationInTicksOfBar(int barIndex) const;
    unsigned int VoiceCountOfBar(int barIndex) const;
    unsigned int VoiceCountOfBarInGrandStaff(int barIndex, int grandStaff) const;
	
    bool HasClefChangeAtBar(int barIndex) const;
    bool HasClefChangeAtBarOnHand(int barIndex, bool leftHand) const;
    
    const REClefTimeline& ClefTimeline(bool leftHand) const ;
    REClefTimeline& ClefTimeline(bool leftHand);
    
    REMultivoiceIterator MultivoiceIteratorOnBar(int barIndex, int firstVoiceIndex, int lastVoiceIndex) const;
    
    bool HasAnyChordInBarRangeSetThisFlag(int flag, const RERange& barRange) const;
    
    REMidiClip* CalculateMidiClipForBar(int barIndex) const;
    void CalculateMidiClipEventsForChord(REMidiClip* clip, const REChord* chord, long tick) const;
    
    bool HasFlag(TrackFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(TrackFlag flag) {_flags |= flag;}
    void UnsetFlag(TrackFlag flag) {_flags &= ~flag;}
    
    RESlur* Slur(Reflow::StaffIdentifier staffId, int idx);
    RESlurVector& Slurs(Reflow::StaffIdentifier staffId);
    const RESlurVector& Slurs(Reflow::StaffIdentifier staffId) const;
    void FindSlursInBarRange(const RERange& barRange, Reflow::StaffIdentifier staffId, REConstSlurPtrVector& slurs) const;
    int IndexOfSlur(const RESlur* slur, Reflow::StaffIdentifier staffId) const;
    void RemoveSlurAtIndex(int idx, Reflow::StaffIdentifier staffId);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    RETrack* CloneKeepingPhrasesInRange(int firstBar, int lastBar, unsigned long flags=0) const;
    void EncodeKeepingPhrasesInRange(REOutputStream& coder, int firstBar, int lastBar, unsigned long flags=0) const;
    
    void InvalidateDeviceUUID() const {_deviceUUID = -1;}
    
    void SetBot(REBot* bot);
    const REBot* Bot() const {return _bot;}
    REBot* Bot() {return _bot;}
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    bool operator==(const RETrack& rhs) const;
    bool operator!=(const RETrack& rhs) const {return !(*this == rhs);}
    
private:
	void _UpdateIndices();
	
    void CalculateMidiClipEventsForGraceNote(REMidiClip *clip, const REChord *chord, const RENote* parentNote, const REGraceNote* note, double startTime, double velocity) const;
    
private:
    RESong * _parent;
    REVoiceVector _voices;
    uint32_t _flags;
	int _index;
    std::string _name, _shortName;
    std::vector<uint8_t> _tuning;
    Reflow::TrackType _type;
    Reflow::TablatureInstrumentType _tablatureInstrumentType;
    REClefTimeline _clefTimeline;
    REClefTimeline _clefTimelineLeftHand;
    int8_t _capo;
    REPitchClass _transposingInterval;
    REBot* _bot;
    
    RESlurVector _firstStaffSlurs;
    RESlurVector _secondStaffSlurs;
    RESlurVector _tablatureStaffSlurs;
    
    int _midiProgram;
    float _volume;
    float _pan;
    bool _solo;
    bool _mute;
    mutable int32_t _deviceUUID;        // used for internal sequencer tracking
};


#endif
