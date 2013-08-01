//
//  RETrack.cpp
//  ReflowMac
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RETrack.h"
#include "RESong.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "REFunctions.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REMidiClip.h"
#include "RENote.h"
#include "REBar.h"

#include <cmath>

RETrack::RETrack()
: _parent(0), _index(-1), _type(Reflow::StandardTrack), _midiProgram(0), _volume(0.80), _pan(0.50), _solo(false), _mute(false), _capo(0), _flags(0), _deviceUUID(-1), _tablatureInstrumentType(Reflow::GuitarInstrument), _bot(nullptr)
{
    
}

RETrack::RETrack(Reflow::TrackType type)
: _parent(0), _index(-1), _type(type), _midiProgram(0), _volume(0.80), _pan(0.50), _solo(false), _mute(false), _capo(0), _flags(0), _deviceUUID(-1), _tablatureInstrumentType(Reflow::GuitarInstrument), _bot(nullptr)
{
    if(type == Reflow::TablatureTrack) {
        SetStringCount(6);
    }
}

RETrack::~RETrack()
{
    Clear();
}

void RETrack::Clear()
{
    for(REVoiceVector::const_iterator it = _voices.begin(); it != _voices.end(); ++it) {
        delete *it;
    }
    _voices.clear();
    _tuning.clear();
    SetBot(nullptr);
}

void RETrack::SetBot(REBot* bot)
{
    if(_bot) {delete _bot; _bot = nullptr;}
    _bot = bot;
}

void RETrack::Refresh()
{
    _clefTimeline.RemoveIdenticalSiblingItems();
    _clefTimelineLeftHand.RemoveIdenticalSiblingItems();
    
    for(REVoiceVector::const_iterator it = _voices.begin(); it != _voices.end(); ++it) {
        (*it)->Refresh();
    }
}

const REVoice* RETrack::Voice(int idx) const
{
    if(idx >= 0 && idx < _voices.size()) {
        return _voices[idx];
    }
    return 0;
}

REVoice* RETrack::Voice(int idx)
{
    if(idx >= 0 && idx < _voices.size()) {
        return _voices[idx];
    }
    return 0;
}

void RETrack::InsertVoice(REVoice* voice, int idx)
{
    _voices.insert(_voices.begin() + idx, voice);
    voice->_parent = this;
    _UpdateIndices();
}

void RETrack::RemoveVoice(int idx)
{
    if(idx >= 0 && idx < _voices.size()) {
        REVoice* voice = _voices[idx];
        delete voice;
        _voices.erase(_voices.begin() + idx);
        _UpdateIndices();
    }
}

void RETrack::_UpdateIndices() {
    for(unsigned int i=0; i<_voices.size(); ++i) {
        _voices[i]->_index = i;
    }
}

bool RETrack::operator==(const RETrack& rhs) const 
{
    if(VoiceCount() != rhs.VoiceCount()) {
        return false;
    }
    
    for(unsigned int i=0; i<VoiceCount(); ++i) {
        if(*Voice(i) != *rhs.Voice(i)) {
            return false;
        }
    }
    return true;
}

unsigned int RETrack::StringCount() const
{
    return (unsigned int)_tuning.size();
}

unsigned int RETrack::TuningForString(unsigned int stringIndex) const
{
    if(stringIndex < StringCount()) {
        return _tuning[stringIndex];
    }
    return 0;
}
void RETrack::SetTuningForString(unsigned int stringIndex, uint8_t midi)
{
    if(stringIndex < StringCount()) {
        _tuning[stringIndex] = midi;
        // TODO: Invalidate pitch of all notes
    }
}

void RETrack::SetTuning(const int* tuningArray, int stringCount)
{
    if(_type != Reflow::TablatureTrack) {
        return;
    }
    
    assert(stringCount >= 1 && stringCount <= REFLOW_MAX_STRINGS);
    _tuning.clear();
    for(unsigned int i=0; i<stringCount; ++i) {
        _tuning.push_back(tuningArray[i]);
    }
    
    Refresh();
}

std::string RETrack::TuningNoteNames() const
{
    if(_tuning.empty()) return "";
    
    std::string noteNames = "";
    for(int i=(int)_tuning.size()-1; i>=0; --i)
    {
        RENotePitch pitches[3];
        Reflow::PitchSetFromMidi(_tuning[i], pitches);
        noteNames += pitches[0].NoteName();
    }
    return noteNames;
}

void RETrack::SetStringCount(unsigned int stringCount)
{
    if(_type != Reflow::TablatureTrack) {
        return;
    }
    
    assert(stringCount >= 1 && stringCount <= REFLOW_MAX_STRINGS);
    _tuning.clear();
    for(unsigned int i=0; i<stringCount; ++i) {
        _tuning.push_back(Reflow::StandardTuningForString(i));
    }
    
    // TODO: Invalidate pitch of notes
}

bool RETrack::IsBarEmptyOrRest(int barIndex) const
{
    for(unsigned int i=0; i<_voices.size(); ++i) {
        const REPhrase* phrase = _voices[i]->Phrase(barIndex);
        if(phrase && !phrase->IsEmptyOrRest()) {
            return false;
        }
    }
    return true;
}

bool RETrack::IsBarEmptyAndCollapsibleWithNextSibling(int barIndex) const
{
    int barCount = _parent->BarCount();
    if(barIndex >= (barCount-1)) {
        return false;
    }

    if(!_parent->IsBarCollapsibleWithNextSibling(barIndex)) {
        return false;
    }
    
    bool firstIsEmpty = IsBarEmptyOrRest(barIndex);
    bool secondIsEmpty = IsBarEmptyOrRest(barIndex+1);
    return firstIsEmpty && secondIsEmpty;
}

// Since we have Grand Staff, this method is not called anymore
unsigned int RETrack::VoiceCountOfBar(int barIndex) const
{
    if(!Voice(3)->Phrase(barIndex)->IsEmptyOrRest()) {return 4;}
    if(!Voice(2)->Phrase(barIndex)->IsEmptyOrRest()) {return 3;}
    if(!Voice(1)->Phrase(barIndex)->IsEmptyOrRest()) {return 2;}
    if(!Voice(0)->Phrase(barIndex)->IsEmptyOrRest()) {return 1;}
    return 0;
}

unsigned int RETrack::VoiceCountOfBarInGrandStaff(int barIndex, int grandStaff) const
{
    if(grandStaff == 0) {
        if(!Voice(1)->Phrase(barIndex)->IsEmptyOrRest()) {return 2;}
        if(!Voice(0)->Phrase(barIndex)->IsEmptyOrRest()) {return 1;}
    }
    else {
        if(!Voice(3)->Phrase(barIndex)->IsEmptyOrRest()) {return 2;}
        if(!Voice(2)->Phrase(barIndex)->IsEmptyOrRest()) {return 1;}
    }
    //return 0;
    return 1;
}

int RETrack::MaxDurationInTicksOfBar(int barIndex) const
{
    int max = 0;
    for(unsigned int i=0; i<_voices.size(); ++i) {
        const REPhrase* phrase = _voices[i]->Phrase(barIndex);
        if(phrase) {
            int duration = phrase->DurationInTicks();
            if(duration > max) max = duration;
        }
    }
    return max;
}

const REClefTimeline& RETrack::ClefTimeline(bool leftHand) const 
{
    return leftHand ? _clefTimelineLeftHand : _clefTimeline;
}

REClefTimeline& RETrack::ClefTimeline(bool leftHand)
{
    return leftHand ? _clefTimelineLeftHand : _clefTimeline;
}

bool RETrack::HasClefChangeAtBar(int barIndex) const
{
    return HasClefChangeAtBarOnHand(barIndex, false) || HasClefChangeAtBarOnHand(barIndex, true);
}

bool RETrack::HasClefChangeAtBarOnHand(int barIndex, bool leftHand) const
{
    bool exact = false;
    int idx = ClefTimeline(leftHand).IndexOfItemAt(barIndex, RETimeDiv(0), &exact);
    return exact;
}

REMultivoiceIterator RETrack::MultivoiceIteratorOnBar(int barIndex, int firstVoiceIndex, int lastVoiceIndex) const
{
    return REMultivoiceIterator(this, barIndex, firstVoiceIndex, lastVoiceIndex);
}

static void _InitBendArray(REFloatVector& vec, int size) {
    vec.clear();
    vec.reserve(size);
    vec.insert(vec.begin(), size, 0.0f);
}

void RETrack::CalculateMidiClipEventsForGraceNote(REMidiClip *clip, const REChord *chord, const RENote* parentNote, const REGraceNote* note, double startTime, double velocity) const
{
    // Pitch
    int pitch = note->Pitch().midi;
    
    double duration = REFLOW_PULSES_PER_QUARTER / 8;
    startTime -= 1.1 * duration;
    
    // Channel
    uint8_t channel = 0;
    if(IsDrums()) channel = 0x09;
    else if(IsTablature()) channel = note->String();
    int program = (IsDrums() ? 0 : MIDIProgram());
    int bank = (IsDrums() ? 128 : 0);
    
    // Channel is not used ?
    if(note->HasFlag(RENote::DeadNote))
    {
        // Bank Select
        REMidiEvent ccEvent;
        ccEvent.tick = int32_t(startTime);
        ccEvent.data[0] = 0xB0 | channel;
        ccEvent.data[1] = 0x00;     // Bank Select Controller Type
        ccEvent.data[2] = 0;     // Bank 0
        clip->_events.push_back(ccEvent);
        
        // Program Change
        REMidiEvent pcEvent;
        pcEvent.tick = int32_t(startTime);
        pcEvent.data[0] = 0xC0 | channel;
        pcEvent.data[1] = 28;         // MIDI Program 29: E. Guitar Muted
        pcEvent.data[2] = 0;
        clip->_events.push_back(pcEvent);
        
        // Unuse channel to allow a new Program Change / Bank Select message to be sent on the next note
        clip->_channelUsed &= ~(1 << channel);
    }
    else
    {
        if(0 == (clip->_channelUsed & (1 << channel)))
        {
            // Bank Select
            REMidiEvent ccEvent;
            ccEvent.tick = int32_t(startTime);
            ccEvent.data[0] = 0xB0 | channel;
            ccEvent.data[1] = 0x00;     // Bank Select Controller Type
            ccEvent.data[2] = bank;  // Value
            clip->_events.push_back(ccEvent);
            
            // Program Change
            REMidiEvent pcEvent;
            pcEvent.tick = int32_t(startTime);
            pcEvent.data[0] = 0xC0 | channel;
            pcEvent.data[1] = program;
            pcEvent.data[2] = 0;
            clip->_events.push_back(pcEvent);
            
            clip->_channelUsed |= (1 << channel);
        }
    }
    
    // Duration Modifiers
    if(chord->HasFlag(REChord::Staccato)) duration *= 0.50;
    if(chord->HasFlag(REChord::PalmMute)) duration *= 0.75;
    if(note->HasFlag(RENote::DeadNote)) duration *= 0.25;
    
    // Velocity Modifiers
    if(chord->HasFlag(REChord::Accent)) velocity += 20.0;
    else if(chord->HasFlag(REChord::StrongAccent)) velocity += 30.0;
    if(chord->HasFlag(REChord::StrumUpwards)) velocity -= 4.0;
    if(note->HasFlag(RENote::LeftStick)) velocity -= 4.0;
    if(note->HasFlag(RENote::GhostNote)) velocity *= 0.35;
    if(note->HasFlag(RENote::DeadNote)) velocity *= 0.50;
    
    // Pitch Bend
    bool hasBend = note->HasBend();
    bool hasVibrato = chord->HasFlag(REChord::Vibrato);
    Reflow::SlideInType slideIn = note->SlideIn();
    Reflow::SlideOutType slideOut = note->SlideOut();
    if(hasBend || hasVibrato || slideIn || slideOut)
    {
        double pitchMod;
        double resolution = 16.0; //0.05;
        int nbDivs = 1 + (duration / resolution);
        double nbDivsFloat = (double)nbDivs;
        
        // TODO: Multiple Grace Notes
        const RENote* rightNote = parentNote; // note->FindNextNoteOnSameString();
        int rightMidiNote = (rightNote ? rightNote->Pitch().midi : pitch+4);
        
        // Fix bad slide out
        if(slideOut == Reflow::ShiftSlide && rightNote == NULL) {
            slideOut = Reflow::SlideOutHigh;
        }
        
        float bendInitialPitch = 0.0;
        float bendMiddlePitch = 0.0;
        float bendFinalPitch = 0.0;
        if(hasBend) {
            note->Bend().PitchFactors(&bendInitialPitch, &bendMiddlePitch, &bendFinalPitch);
        }
        
        const double shiftSlideStart = 0.50;
        const double shiftSlideEnd = 0.95;
        const double vibratoInterval = 0.40 * REFLOW_PULSES_PER_QUARTER;
        const double vibratoAmp = 0.80;
        
        for(int i=0; i<=nbDivs; ++i)
        {
            pitchMod = 0.0;
            
            double t = (double)i / nbDivsFloat;
            if (t > 1.0) t = 1.0;
            
            double time = startTime + t * duration;
            
            // Bend
            if(hasBend)
            {
                const REBend& bend = note->Bend();
                double value = 0.0;
                
                // First part of bend
                if(i < (nbDivs/2))
                {
                    float x = 2.0 * t;
                    value = (1.0-x) * bendInitialPitch + (x * bendMiddlePitch);
                }
                
                // Second Part of bend
                else {
                    float x = (2.0 * t) - 1.0;
                    value = (1.0-x) * bendMiddlePitch + (x * bendFinalPitch);
                }
                
                pitchMod += value;
            }
            
            // Shift Slide (between 0.20 .. 0.80)
            if(slideOut == Reflow::ShiftSlide)
            {
                if(shiftSlideStart <= t && t <= shiftSlideEnd)
                {
                    double x = (t - shiftSlideStart) / (shiftSlideEnd - shiftSlideStart);
                    double dpitchSemitones = (double)(rightMidiNote - pitch);
                    double value = (x * dpitchSemitones);
                    pitchMod += 0.5 * value;
                }
                else if(t > shiftSlideEnd) {
                    double dpitchSemitones = (double)(rightMidiNote - pitch);
                    pitchMod += 0.5 * dpitchSemitones;
                }
            }
            else if(slideOut == Reflow::SlideOutHigh || slideOut == Reflow::SlideOutLow)
            {
                double dpitchSemitones = (slideOut == Reflow::SlideOutHigh ? 4.0 : -4.0);
                if(shiftSlideStart <= t)
                {
                    double x = (t - shiftSlideStart) / (1.0 - shiftSlideStart);
                    double value = (x * dpitchSemitones);
                    pitchMod += 0.5 * value;
                }
            }
            
            if(slideIn == Reflow::SlideInFromAbove || slideIn == Reflow::SlideInFromBelow)
            {
                double dpitchSemitones = (slideIn == Reflow::SlideInFromBelow ? -4.0 : 4.0);
                if(t <= shiftSlideStart)
                {
                    double x = (t) / (shiftSlideStart);
                    double value = (1.0-x) * dpitchSemitones;
                    pitchMod += 0.5 * value;
                }
            }
            
            // Vibrato
            if(hasVibrato)
            {
                double x = fmod((double)i * resolution, vibratoInterval) / vibratoInterval;
                if(x >= 0.5) {
                    pitchMod += vibratoAmp * (1.0 - x);
                }
                else {
                    pitchMod += vibratoAmp * x;
                }
            }
            
            // delta [-1..1], doit correspondre a une variation de pitch de -24..+24 demitons
            // apres l'envoi des messages de reglage du RPN0 (pitch wheel sensitivity)
            double delta = pitchMod / 12.0;
            int pitchBend;
            if(delta > 1.0) pitchBend = 16384;		// 2^14 bits
            else if(delta < -1.0) pitchBend = 0;
            else {
                pitchBend = (unsigned int) (16384.0 * (0.5 + (0.5 * delta)));
            }
            
            // Pitch Wheel Command
            REMidiEvent pb;
            pb.tick = int32_t(time);
            pb.data[0] = 0xE0 | channel;
            pb.data[1] = pitchBend & 0x7F;				// Fine Tuning is byte data 1
            pb.data[2] = (pitchBend >> 7) & 0x7F;     // Coarse Tuning is byte data 2
            clip->_events.push_back(pb);
        }
        
    }
    else
    {
        // No Pitch bend at all
        int pitchBend = 8192;
        
        // Pitch Wheel Command
        REMidiEvent pb;
        pb.tick = int32_t(startTime);
        pb.data[0] = 0xE0 | channel;
        pb.data[1] = pitchBend & 0x7F;				// Fine Tuning is byte data 1
        pb.data[2] = (pitchBend >> 7) & 0x7F;     // Coarse Tuning is byte data 2
        clip->_events.push_back(pb);
    }
    
    // Midi Note Event (embeds NoteOn / NoteOff)
    REMidiNoteEvent noteEvent;
    noteEvent.channel = channel;
    noteEvent.tick = int32_t(startTime);
    noteEvent.duration = uint32_t(0.99 * duration);
    noteEvent.pitch = pitch;
    noteEvent.velocity = uint8_t(Reflow::Clamp(velocity, 0.0, 127.0));
    noteEvent.flags = 0x00;
    
    // Tablature tracks must cut down the sound quickly for this string if we don't let ring
    if(IsTablature()) {
        noteEvent.flags |= REMidiNoteEvent::UseSoundOff;
    }
    
    clip->_notes.push_back(noteEvent);
}

void RETrack::CalculateMidiClipEventsForChord(REMidiClip* clip, const REChord* chord, long tick) const
{
    // Add Notes
    long chordDuration = chord->DurationInTicks();
    unsigned int nbNotes = chord->NoteCount();
    for(unsigned int noteIndex=0; noteIndex<nbNotes; ++noteIndex)
    {
        double startTime = (double)tick;
        double velocity = Reflow::DynamicsToVelocity(chord->Dynamics());
        double duration = (double)chordDuration;
        
        const RENote* note = chord->Note(noteIndex);
        if(note->HasFlag(RENote::TieDestination)) continue;
        
        for(const REGraceNote* graceNote : note->GraceNotes()) {
            CalculateMidiClipEventsForGraceNote(clip, chord, note, graceNote, startTime, 0.75 * velocity);
        }
        
        // Pitch
        int pitch = note->Pitch().midi;
        
        // Channel
        uint8_t channel = 0;
        if(IsDrums()) channel = 0x09;
        else if(IsTablature()) channel = note->String();
        int program = (IsDrums() ? 0 : MIDIProgram());
        int bank = (IsDrums() ? 128 : 0);
        
        // Channel is not used ?
        if(note->HasFlag(RENote::DeadNote))
        {
            // Bank Select
            REMidiEvent ccEvent;
            ccEvent.tick = int32_t(startTime);
            ccEvent.data[0] = 0xB0 | channel;
            ccEvent.data[1] = 0x00;     // Bank Select Controller Type
            ccEvent.data[2] = 0;     // Bank 0
            clip->_events.push_back(ccEvent);
            
            // Program Change
            REMidiEvent pcEvent;
            pcEvent.tick = int32_t(startTime);
            pcEvent.data[0] = 0xC0 | channel;
            pcEvent.data[1] = 28;         // MIDI Program 29: E. Guitar Muted
            pcEvent.data[2] = 0;
            clip->_events.push_back(pcEvent);
            
            // Unuse channel to allow a new Program Change / Bank Select message to be sent on the next note
            clip->_channelUsed &= ~(1 << channel);
        }
        else 
        {
            if(0 == (clip->_channelUsed & (1 << channel)))
            {
                // Bank Select
                REMidiEvent ccEvent;
                ccEvent.tick = int32_t(startTime);
                ccEvent.data[0] = 0xB0 | channel;
                ccEvent.data[1] = 0x00;     // Bank Select Controller Type
                ccEvent.data[2] = bank;  // Value
                clip->_events.push_back(ccEvent);
                
                // Program Change
                REMidiEvent pcEvent;
                pcEvent.tick = int32_t(startTime);
                pcEvent.data[0] = 0xC0 | channel;
                pcEvent.data[1] = program;
                pcEvent.data[2] = 0;
                clip->_events.push_back(pcEvent);
                
                clip->_channelUsed |= (1 << channel);
            }
        }
        
        
        // Note Tie Origin
        if(note->HasFlag(RENote::TieOrigin)) {
            const RENote* tiedNote = note->FindDestinationOfTiedNote();
            while(tiedNote) 
            {
                duration += (double) tiedNote->Chord()->DurationInTicks();
                tiedNote = tiedNote->FindDestinationOfTiedNote();
            }
        }
        
        // Duration Modifiers
        if(chord->HasFlag(REChord::Staccato)) duration *= 0.50;
        if(chord->HasFlag(REChord::PalmMute)) duration *= 0.75;
        if(note->HasFlag(RENote::DeadNote)) duration *= 0.25;
        
        // Velocity Modifiers
        if(chord->HasFlag(REChord::Accent)) velocity += 20.0;
        else if(chord->HasFlag(REChord::StrongAccent)) velocity += 30.0;
        if(chord->HasFlag(REChord::StrumUpwards)) velocity -= 4.0;
        if(note->HasFlag(RENote::LeftStick)) velocity -= 4.0;
        if(note->HasFlag(RENote::GhostNote)) velocity *= 0.35;
        if(note->HasFlag(RENote::DeadNote)) velocity *= 0.50;
        
        // Pitch Bend
        bool hasBend = note->HasBend();
        bool hasVibrato = chord->HasFlag(REChord::Vibrato);
        Reflow::SlideInType slideIn = note->SlideIn();
        Reflow::SlideOutType slideOut = note->SlideOut();
        if(hasBend || hasVibrato || slideIn || slideOut)
        {
            double pitchMod;
            double resolution = 16.0; //0.05;
            int nbDivs = 1 + (duration / resolution);
            double nbDivsFloat = (double)nbDivs;
            
            const RENote* rightNote = note->FindNextNoteOnSameString();
            int rightMidiNote = (rightNote ? rightNote->Pitch().midi : pitch+4);
            
            // Fix bad slide out
            if(slideOut == Reflow::ShiftSlide && rightNote == NULL) {
                slideOut = Reflow::SlideOutHigh;
            }
            
            float bendInitialPitch = 0.0;
            float bendMiddlePitch = 0.0;
            float bendFinalPitch = 0.0;
            if(hasBend) {
                note->Bend().PitchFactors(&bendInitialPitch, &bendMiddlePitch, &bendFinalPitch);
            }
            
            const double shiftSlideStart = 0.50;
            const double shiftSlideEnd = 0.95;
            const double vibratoInterval = 0.40 * REFLOW_PULSES_PER_QUARTER;
            const double vibratoAmp = 0.80;
            
            for(int i=0; i<=nbDivs; ++i) 
            {
                pitchMod = 0.0;
                
                double t = (double)i / nbDivsFloat;
                if (t > 1.0) t = 1.0;
                
                double time = startTime + t * duration;
                
                // Bend
                if(hasBend)
                {
                    const REBend& bend = note->Bend();
                    double value = 0.0;
                    
                    // First part of bend
                    if(i < (nbDivs/2))
                    {
                        float x = 2.0 * t;
                        value = (1.0-x) * bendInitialPitch + (x * bendMiddlePitch);
                    }
                    
                    // Second Part of bend
                    else {
                        float x = (2.0 * t) - 1.0;
                        value = (1.0-x) * bendMiddlePitch + (x * bendFinalPitch);
                    }
                    
                    pitchMod += value;
                }
                
                // Shift Slide (between 0.20 .. 0.80)
                if(slideOut == Reflow::ShiftSlide) 
                {
                    if(shiftSlideStart <= t && t <= shiftSlideEnd) 
                    {
                        double x = (t - shiftSlideStart) / (shiftSlideEnd - shiftSlideStart);
                        double dpitchSemitones = (double)(rightMidiNote - pitch);
                        double value = (x * dpitchSemitones);
                        pitchMod += 0.5 * value;
                    }
                    else if(t > shiftSlideEnd) {
                        double dpitchSemitones = (double)(rightMidiNote - pitch);
                        pitchMod += 0.5 * dpitchSemitones;
                    }
                }
                else if(slideOut == Reflow::SlideOutHigh || slideOut == Reflow::SlideOutLow)
                {
                    double dpitchSemitones = (slideOut == Reflow::SlideOutHigh ? 4.0 : -4.0);
                    if(shiftSlideStart <= t) 
                    {
                        double x = (t - shiftSlideStart) / (1.0 - shiftSlideStart);
                        double value = (x * dpitchSemitones);
                        pitchMod += 0.5 * value;
                    }
                }
                
                if(slideIn == Reflow::SlideInFromAbove || slideIn == Reflow::SlideInFromBelow)
                {
                    double dpitchSemitones = (slideIn == Reflow::SlideInFromBelow ? -4.0 : 4.0);
                    if(t <= shiftSlideStart) 
                    {
                        double x = (t) / (shiftSlideStart);
                        double value = (1.0-x) * dpitchSemitones;
                        pitchMod += 0.5 * value;
                    }
                }
                
                // Vibrato
                if(hasVibrato)
                {
                    double x = fmod((double)i * resolution, vibratoInterval) / vibratoInterval;
                    if(x >= 0.5) {
                        pitchMod += vibratoAmp * (1.0 - x);
                    }
                    else {
                        pitchMod += vibratoAmp * x;
                    }
                }
                
                // delta [-1..1], doit correspondre a une variation de pitch de -24..+24 demitons 
                // apres l'envoi des messages de reglage du RPN0 (pitch wheel sensitivity)
                double delta = pitchMod / 12.0;
                int pitchBend;
                if(delta > 1.0) pitchBend = 16384;		// 2^14 bits
                else if(delta < -1.0) pitchBend = 0;
                else {
                    pitchBend = (unsigned int) (16384.0 * (0.5 + (0.5 * delta)));
                }
                
                // Pitch Wheel Command
                REMidiEvent pb;
                pb.tick = int32_t(time);
                pb.data[0] = 0xE0 | channel;
                pb.data[1] = pitchBend & 0x7F;				// Fine Tuning is byte data 1
                pb.data[2] = (pitchBend >> 7) & 0x7F;     // Coarse Tuning is byte data 2
                clip->_events.push_back(pb);
            }
            
        }
        else 
        {
            // No Pitch bend at all
            int pitchBend = 8192;
            
            // Pitch Wheel Command
            REMidiEvent pb;
            pb.tick = int32_t(startTime);
            pb.data[0] = 0xE0 | channel;
            pb.data[1] = pitchBend & 0x7F;				// Fine Tuning is byte data 1
            pb.data[2] = (pitchBend >> 7) & 0x7F;     // Coarse Tuning is byte data 2
            clip->_events.push_back(pb);
        }
        
        // Midi Note Event (embeds NoteOn / NoteOff)
        REMidiNoteEvent noteEvent;
        noteEvent.channel = channel;
        noteEvent.tick = int32_t(startTime);
        noteEvent.duration = uint32_t(0.99 * duration);
        noteEvent.pitch = pitch;
        noteEvent.velocity = uint8_t(Reflow::Clamp(velocity, 0.0, 127.0));
        noteEvent.flags = 0x00;
        
        // Tablature tracks must cut down the sound quickly for this string if we don't let ring
        if(IsTablature() && !chord->HasFlag(REChord::LetRing)) {
            const REChord* next = chord->NextSiblingOverBarline();
            if(next && !next->HasNoteOnString(note->String())) {
                noteEvent.flags |= REMidiNoteEvent::UseSoundOff;
            }
        }
        
        clip->_notes.push_back(noteEvent);
    }
}

REMidiClip* RETrack::CalculateMidiClipForBar(int barIndex) const
{
    const int nbChannels = 16;
    const int resolution = 16;
    
    const REBar* bar = _parent->Bar(barIndex);
    int barTickDuration = bar->TheoricDurationInTicks();
    int nbDivs = barTickDuration / resolution;
    
    REMidiClip* clip = new REMidiClip;
    clip->_channelUsed = 0;
    for(unsigned int voiceIndex=0; voiceIndex<VoiceCount(); ++voiceIndex)
    {
        const REVoice* voice = Voice(voiceIndex);
        const REPhrase* phrase = voice->Phrase(barIndex);
        if(!phrase) continue;
        
        long tick = 0;
        unsigned int nbChords = phrase->ChordCount();
        for(unsigned int chordIndex=0; chordIndex<nbChords; ++chordIndex)
        {
            const REChord* chord = phrase->Chord(chordIndex);
            long chordDuration = chord->DurationInTicks();
            
            CalculateMidiClipEventsForChord(clip, chord, tick);
            
            tick += chordDuration;
        }
    }
    return clip;
}

bool RETrack::HasAnyChordInBarRangeSetThisFlag(int flag, const RERange& barRange) const
{
    for(int voiceIndex=0; voiceIndex < VoiceCount(); ++voiceIndex)
    {
        const REVoice* voice = Voice(voiceIndex);
        for(int barIndex=barRange.FirstIndex(); barIndex <= barRange.LastIndex(); ++barIndex)
        {
            const REPhrase* phrase = voice->Phrase(barIndex);
            int chordCount = phrase->ChordCount();
            for(int chordIndex=0; chordIndex < chordCount; ++chordIndex) {
                const REChord* chord = phrase->Chord(chordIndex);
                if(chord->HasFlag((REChord::ChordFlag)flag)) {
                    return true;
                }
            }
        }
    }
    return false;
}

const RESlurVector& RETrack::Slurs(Reflow::StaffIdentifier id_) const
{
    switch(id_) {
        case Reflow::FirstStandardStaffIdentifier: return _firstStaffSlurs;
        case Reflow::SecondStandardStaffIdentifier: return _secondStaffSlurs;
        case Reflow::TablatureStaffIdentifier: return _tablatureStaffSlurs;
        default: return _firstStaffSlurs;
    }
}

RESlurVector& RETrack::Slurs(Reflow::StaffIdentifier id_)
{
    switch(id_) {
        case Reflow::FirstStandardStaffIdentifier: return _firstStaffSlurs;
        case Reflow::SecondStandardStaffIdentifier: return _secondStaffSlurs;
        case Reflow::TablatureStaffIdentifier: return _tablatureStaffSlurs;
        default: return _firstStaffSlurs;
    }
}

void RETrack::FindSlursInBarRange(const RERange& barRange, Reflow::StaffIdentifier staffId, REConstSlurPtrVector& slurs) const
{
    for(const RESlur& slur : Slurs(staffId))
    {
        if(slur.BarRange().IntersectsRange(barRange)) {
            slurs.push_back(&slur);
        }
    }
}

RESlur* RETrack::Slur(Reflow::StaffIdentifier staffId, int idx)
{
    RESlurVector& slurs = Slurs(staffId);
    if(idx >= 0 && idx < slurs.size())
    {
        return &slurs[idx];
    }
    return nullptr;
}

int RETrack::IndexOfSlur(const RESlur* slur, Reflow::StaffIdentifier staffId) const
{
    const RESlurVector& slurs = Slurs(staffId);
    for(int i=0; i<slurs.size(); ++i) {
        if(slur == &slurs[i]) return i;
    }
    return -1;
}

void RETrack::RemoveSlurAtIndex(int idx, Reflow::StaffIdentifier staffId)
{
    RESlurVector& slurs = Slurs(staffId);
    if(idx >= 0 && idx < slurs.size()) {
        slurs.erase(slurs.begin() + idx);
    }
}

RETrack* RETrack::Clone()
{
    REBufferOutputStream coder;
    EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    RETrack* clonedTrack = new RETrack;
    clonedTrack->DecodeFrom(decoder);
    clonedTrack->_deviceUUID = -1;
    return clonedTrack;
}

void RETrack::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    writer.String("name"); writer.String(_name.data(), _name.size());
    writer.String("short_name"); writer.String(_shortName.data(), _shortName.size());
    writer.String("type"); writer.String(Reflow::NameOfTrackType(_type));
    if(_type == Reflow::TablatureTrack)
    {
        // Guitar Type
        writer.String("guitar_type"); writer.String(Reflow::NameOfTablatureInstrumentType(_tablatureInstrumentType));
        
        // Tuning
        writer.String("tuning");
        {
            writer.StartObject();
            
            writer.String("strings");
            {
                writer.StartArray();
                for(uint8_t i=0; i<_tuning.size(); ++i) {
                    writer.Int(_tuning[i]);
                }
                writer.EndArray();
            }
            
            writer.EndObject();
        }
        
        // Capo
        if(_capo) {
            writer.String("capo"); writer.Int(_capo);
        }
    }
    
    // Sound
    writer.String("sound");
    {
        writer.StartObject();
        
        writer.String("program"); writer.Int(_midiProgram);
        writer.String("volume"); writer.Double(_volume);
        writer.String("pan"); writer.Double(_pan);
        writer.String("solo"); writer.Bool(_solo);
        writer.String("mute"); writer.Bool(_mute);
        
        writer.EndObject();
    }
    
    // Staffs
    writer.String("staves");
    {
        writer.StartArray();
        
        // Right Hand
        writer.StartObject();
        {
            writer.String("clef_changes"); _clefTimeline.WriteJson(writer, version);
        }
        writer.EndObject();
        
        if(IsGrandStaff()) {
            writer.StartObject();
            {
                writer.String("clef_changes"); _clefTimelineLeftHand.WriteJson(writer, version);
            }
            writer.EndObject();
        }
        
        writer.EndArray();
    }
    
    // Voices
    writer.String("voices");
    {
        writer.StartArray();
        for(unsigned int i=0; i<_voices.size(); ++i) {
            _voices[i]->WriteJson(writer, version);
        }
        writer.EndArray();
    }
    
    writer.EndObject();
}

void RETrack::ReadJson(const REJsonValue& obj, uint32_t version)
{
    Clear();
    
    const REJsonValue& name = obj["name"];
    const REJsonValue& short_name = obj["short_name"];
    const REJsonValue& type = obj["type"];
    const REJsonValue& tablatureType = obj["guitar_type"];
    const REJsonValue& tuning = obj["tuning"];
    const REJsonValue& capo = obj["capo"];
    const REJsonValue& sound = obj["sound"];
    const REJsonValue& staves = obj["staves"];
    const REJsonValue& voices_ = obj["voices"];
    
    if(name.IsString()) {
        _name = std::string(name.GetString(), name.GetStringLength());
    }
    
    if(short_name.IsString()) {
        _shortName = std::string(short_name.GetString(), short_name.GetStringLength());
    }
    
    if(type.IsString()) {
        _type = Reflow::ParseTrackType(std::string(type.GetString(), type.GetStringLength()));
    }
    
    if(tablatureType.IsString() && _type == Reflow::TablatureTrack)
    {
        _tablatureInstrumentType = Reflow::ParseTablatureInstrumentType(std::string(tablatureType.GetString(), tablatureType.GetStringLength()));
    }
    
    // Tuning
    if(tuning.IsObject() && _type == Reflow::TablatureTrack)
    {
        const REJsonValue& strings = tuning["strings"];
        if(strings.IsArray()) for(auto it = strings.Begin(); it != strings.End(); ++it)
        {
            _tuning.push_back(it->GetInt());
        }
    }
    
    // Capo
    if(capo.IsInt() && _type == Reflow::TablatureTrack) {
        _capo = capo.GetInt();
    }
    
    // Sound
    if(sound.IsObject())
    {
        const REJsonValue& program = sound["program"];
        const REJsonValue& volume = sound["volume"];
        const REJsonValue& pan = sound["pan"];
        const REJsonValue& solo = sound["solo"];
        const REJsonValue& mute = sound["mute"];
        
        if(program.IsInt()) _midiProgram = program.GetInt();
        if(volume.IsDouble()) _volume = volume.GetDouble();
        if(pan.IsDouble()) _pan = pan.GetDouble();
        if(solo.IsBool()) _solo = solo.GetBool();
        if(mute.IsBool()) _mute = mute.GetBool();
    }
    
    // Staffs
    if(staves.IsArray())
    {
        rapidjson::SizeType nbStaves = staves.Size();
        if(nbStaves == 2) {
            _flags |= RETrack::GrandStaff;
        }
        for(rapidjson::SizeType i = 0; i<nbStaves; ++i)
        {
            const REJsonValue& staff = staves[i];
            if(staff.IsObject())
            {
                const REJsonValue& clef_changes = staff["clef_changes"];
                if(clef_changes.IsArray()) {
                    if(i==0) {
                        _clefTimeline.ReadJson(clef_changes, version);
                    }
                    else {
                        _clefTimelineLeftHand.ReadJson(clef_changes, version);
                    }
                }
            }
        }
    }
    
    // Voices
    if(voices_.IsArray()) {
        for(auto it = voices_.Begin(); it != voices_.End(); ++it)
        {
            REVoice* voice = new REVoice;
            voice->_index = _voices.size();
            voice->_parent = this;
            _voices.push_back(voice);
            
            voice->ReadJson(*it, version);
        }
    }
}

void RETrack::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt8(_type);
    coder.WriteUInt8(_tablatureInstrumentType);
    coder.WriteUInt32(_flags);
    coder.WriteInt32(_deviceUUID);
    
    // Tuning for Tablature tracks
    if(_type == Reflow::TablatureTrack)
    {
        coder.WriteUInt8(_tuning.size());   // string count
        for(uint8_t i=0; i<_tuning.size(); ++i) {
            coder.WriteUInt8(_tuning[i]);
        }
    }
    
    // Capo
    coder.WriteInt8(_capo);
    
    // Name
    coder.WriteString(_name);
    coder.WriteString(_shortName);
    
    // MIDI program
    coder.WriteInt8(_midiProgram);
    coder.WriteFloat(_volume);
    coder.WriteFloat(_pan);
    unsigned long flags = 0x00000000;
    if(_solo) flags |= 0x01;
    if(_mute) flags |= 0x02;
    coder.WriteUInt32(flags);
    
    // Clef Timeline
    _clefTimeline.EncodeTo(coder);
    _clefTimelineLeftHand.EncodeTo(coder);
    
    // Write Voices
    coder.WriteUInt8(_voices.size());
    for(unsigned int i=0; i<_voices.size(); ++i) {
        _voices[i]->EncodeTo(coder);
    }
    
    // Slurs for different staves
    coder.WriteUInt32((uint32_t)_firstStaffSlurs.size());
    for(const RESlur& slur : _firstStaffSlurs) slur.EncodeTo(coder);
    coder.WriteUInt32((uint32_t)_secondStaffSlurs.size());
    for(const RESlur& slur : _secondStaffSlurs) slur.EncodeTo(coder);
    coder.WriteUInt32((uint32_t)_tablatureStaffSlurs.size());
    for(const RESlur& slur : _tablatureStaffSlurs) slur.EncodeTo(coder);
}

void RETrack::DecodeFrom(REInputStream& decoder)
{
    Clear();
    _type = (Reflow::TrackType)decoder.ReadUInt8();
    if(decoder.Version() >= 1000) {
        _tablatureInstrumentType = (Reflow::TablatureInstrumentType)decoder.ReadUInt8();
    }
    else {
        _tablatureInstrumentType = Reflow::GuitarInstrument;
    }
    _flags = decoder.ReadUInt32();
    _deviceUUID = decoder.ReadInt32();
    
    // Tuning for Tablature tracks
    if(_type == Reflow::TablatureTrack)
    {
        uint8_t stringCount = decoder.ReadUInt8();  // string count
        for(uint8_t i=0; i<stringCount; ++i)
        {
            _tuning.push_back(decoder.ReadUInt8());
        }
    }
    
    // Capo
    _capo = decoder.ReadInt8();
    
    // Name
    _name = decoder.ReadString();
    _shortName = decoder.ReadString();
    
    // MIDI Program
    _midiProgram = decoder.ReadInt8();
    _volume = decoder.ReadFloat();
    _pan = decoder.ReadFloat();
    unsigned long flags = decoder.ReadUInt32();
    _solo = (0 != (flags & 0x01));
    _mute = (0 != (flags & 0x02));
    
    // Clef Timeline
    _clefTimeline.DecodeFrom(decoder);
    _clefTimelineLeftHand.DecodeFrom(decoder);
    
    // Voices
    uint8_t voiceCount = decoder.ReadUInt8();
    for(unsigned int i=0; i<voiceCount; ++i) {
        REVoice* voice = new REVoice;
        voice->_index = i;
        voice->_parent = this;
        _voices.push_back(voice);
        
        voice->DecodeFrom(decoder);
    }
    
    if(decoder.Version() >= REFLOW_IO_VERSION_1_7_0)
    {
        uint32_t nbSlursFirstStaff = decoder.ReadUInt32();
        for(uint32_t i = 0; i<nbSlursFirstStaff; ++i) {
            RESlur slur; slur.DecodeFrom(decoder); _firstStaffSlurs.push_back(slur);
        }
        uint32_t nbSlursSecondStaff = decoder.ReadUInt32();
        for(uint32_t i = 0; i<nbSlursSecondStaff; ++i) {
            RESlur slur; slur.DecodeFrom(decoder); _secondStaffSlurs.push_back(slur);
        }
        uint32_t nbSlursTablatureStaff = decoder.ReadUInt32();
        for(uint32_t i = 0; i<nbSlursTablatureStaff; ++i) {
            RESlur slur; slur.DecodeFrom(decoder); _tablatureStaffSlurs.push_back(slur);
        }
    }
}

RETrack* RETrack::CloneKeepingPhrasesInRange(int firstBar, int lastBar, unsigned long flags) const
{
    REBufferOutputStream coder;
    this->EncodeKeepingPhrasesInRange(coder, firstBar, lastBar, flags);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    RETrack* track = new RETrack;
    track->DecodeFrom(decoder);
    return track;
}

void RETrack::EncodeKeepingPhrasesInRange(REOutputStream& coder, int firstBar, int lastBar, unsigned long cloneFlags) const
{
    coder.WriteUInt8(_type);
    coder.WriteUInt8(_tablatureInstrumentType);
    coder.WriteUInt32(_flags);
    coder.WriteInt32(_deviceUUID);
    
    // Tuning for Tablature tracks
    if(_type == Reflow::TablatureTrack)
    {
        coder.WriteUInt8(_tuning.size());   // string count
        for(uint8_t i=0; i<_tuning.size(); ++i) {
            coder.WriteUInt8(_tuning[i]);
        }
    }
    
    // Capo
    coder.WriteInt8(_capo);
    
    // Name
    coder.WriteString(_name);
    coder.WriteString(_shortName);
    
    // MIDI program
    coder.WriteInt8(_midiProgram);
    coder.WriteFloat(_volume);
    coder.WriteFloat(_pan);
    unsigned long flags = 0x00000000;
    if(_solo) flags |= 0x01;
    if(_mute) flags |= 0x02;
    coder.WriteUInt32(flags);
    
    // Clef Timeline
    _clefTimeline.EncodeTo(coder);
    _clefTimelineLeftHand.EncodeTo(coder);
    
    // Write Voices
    coder.WriteUInt8(_voices.size());
    for(unsigned int i=0; i<_voices.size(); ++i) {
        _voices[i]->EncodeKeepingPhrasesInRange(coder, firstBar, lastBar);
    }
    
    coder.WriteUInt32(0);
    coder.WriteUInt32(0);
    coder.WriteUInt32(0);
}
