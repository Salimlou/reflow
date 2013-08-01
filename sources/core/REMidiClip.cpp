//
//  REMidiClip.cpp
//  Reflow
//
//  Created by Sebastien on 08/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REMidiClip.h"

REMidiClip::REMidiClip()
: _minTick(0), _maxTick(0), _channelUsed(0)
{    
}

REMidiClip::REMidiClip(const REMidiClip& mc)
: _minTick(mc._minTick), _maxTick(mc._maxTick), _channelUsed(mc._channelUsed)
{
    _events = mc._events;
    _notes = mc._notes;
}

const REMidiEvent& REMidiClip::Event(unsigned int idx) const
{
    return _events[idx];
}

const REMidiNoteEvent& REMidiClip::NoteEvent(unsigned int idx) const
{
    return _notes[idx];
}

void REMidiClip::AddEvent(const REMidiEvent& event)
{
    _events.push_back(event);
    if(event.tick > _maxTick) {
        _maxTick = event.tick;
    }
}

REInstantMidiClip::REInstantMidiClip()
: REMidiClip(), _bpm(120.0), _dpitch(0)
{
}

REInstantMidiClip::REInstantMidiClip(const REMidiClip& mc, double bpm, int dpitch)
: REMidiClip(mc), _bpm(bpm), _dpitch(dpitch)
{
}

REInstantMidiClip::REInstantMidiClip(const REInstantMidiClip& mc)
: REMidiClip(mc), _bpm(mc._bpm), _dpitch(mc._dpitch)
{
}
