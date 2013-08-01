//
//  REMidiClip.h
//  Reflow
//
//  Created by Sebastien on 08/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMidiClip_h
#define Reflow_REMidiClip_h

#include "RETypes.h"

class REMidiClip
{
    friend class REPhrase;
    friend class RETrack;
    friend class RESequencer;
    
public:
    REMidiClip();
    REMidiClip(const REMidiClip& mc);
    virtual ~REMidiClip() {}
    
    unsigned int EventCount() const {return _events.size();}
    const REMidiEvent& Event(unsigned int idx) const;
    
    unsigned int NoteEventCount() const {return _notes.size();}
    const REMidiNoteEvent& NoteEvent(unsigned int idx) const;
    
    int MinTick() const {return _minTick;}
    int MaxTick() const {return _maxTick;}
    
    void AddEvent(const REMidiEvent& event);
    
protected:
    REMidiEventVector _events;
    REMidiNoteEventVector _notes;
    int32_t _minTick;
    int32_t _maxTick;
    uint16_t _channelUsed;
};

typedef std::vector<REMidiClip*> REMidiClipVector;




class REInstantMidiClip : public REMidiClip
{
public:
    REInstantMidiClip();
    REInstantMidiClip(const REMidiClip& mc, double bpm, int dpitch);
    REInstantMidiClip(const REInstantMidiClip& mc);
    virtual ~REInstantMidiClip() {}
    
    void SetDeltaPitch(int dpitch) {_dpitch = dpitch;}
    int DeltaPitch() const {return _dpitch;}
    
    void SetBPM(double bpm) {_bpm = bpm;}
    double BPM() const {return _bpm;}
    
protected:
    double _bpm;
    int _dpitch;
};

typedef REPacketRingBuffer<REInstantMidiClip, 16> REInstantMidiClipRingBuffer;

#endif
