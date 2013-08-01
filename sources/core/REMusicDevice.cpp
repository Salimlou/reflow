//
//  REMusicDevice.cpp
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REMusicDevice.h"
#include "RESequencer.h"
#include "REMonophonicSynthVoice.h"
#include "REMonoSample.h"
#include "RESF2Patch.h"
#include "RESF2Generator.h"

#ifdef REFLOW_IOS
#  include <CoreAudio/CoreAudioTypes.h>
#  include "CAStreamBasicDescription.h"
#  include <AudioUnit/AudioUnit.h>
#  include <AudioToolbox/AudioToolbox.h>
#elif defined(REFLOW_MAC)
#  include <CoreAudio/CoreAudio.h>
#  include "CAStreamBasicDescription.h"
#  include <AudioUnit/AudioUnit.h>
#  include <AudioToolbox/AudioToolbox.h>
#endif

#include <boost/foreach.hpp>
#include <deque>

#define DEBUG_AUDIO



#pragma mark Synthesizer Music Device Impl
                                              

RESynthMusicDevice::RESynthMusicDevice()
: _volume(1.0), _pan(0.5), _soundfont(NULL)
{
    
}

RESynthMusicDevice::RESynthMusicDevice(double sampleRate, RESoundFont* soundfont)
: _soundfont(soundfont), _volume(1.0), _pan(0.5)
{
    Create();
    Initialize(sampleRate);
}

RESynthMusicDevice::~RESynthMusicDevice()
{
    Shutdown();
}

Reflow::MusicDeviceType RESynthMusicDevice::Type() const {
    return Reflow::SynthMusicDevice;
}
    
void RESynthMusicDevice::Create() {
}

void RESynthMusicDevice::SetVolume(float vol)
{
    _volume = vol;
}
void RESynthMusicDevice::SetPan(float pan)
{
    _pan = pan;
}

void RESynthMusicDevice::Initialize(double sampleRate) 
{
    _sampleRate = sampleRate;

    // Create Channels
    for(unsigned int i=0; i<NumChannels; ++i)
    {
        _channels.push_back(NULL);
    }
    
    /*_defaultSample = new REMonoSample();
    _defaultSample->Initialize(4096, _sampleRate);
    _defaultSample->FillWithSawWaveForm();*/
}

void RESynthMusicDevice::Shutdown() 
{
    for(unsigned int i=0; i<NumChannels; ++i)
    {
        if(_channels[i]) {
            delete _channels[i];
            _channels[i] = NULL;
        }
    }
    
   /* if(_defaultSample) {
        delete _defaultSample;
        _defaultSample = NULL;
    }*/
}

// Called for Audio Rendering Thread
void RESynthMusicDevice::MidiEvent(unsigned int cmdByte, unsigned int data1, unsigned int data2, unsigned int sampleOffset) 
{
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    //REPrintf("RESynthMusicDevice::MidiEvent(%2.2x %2.2x %2.2x)\n", cmdByte, data1, data2);
#endif
    unsigned int index = 0;
    unsigned int nbEvents = _events.size();
    while(index < nbEvents && _events[index].sampleOffset <= sampleOffset) {
        ++index;
    }
    ScheduledMidiEvent evt;
    evt.sampleOffset = sampleOffset;
    evt.cmdByte = cmdByte;
    evt.data1 = data1;
    evt.data2 = data2;
    _events.insert(_events.begin()+index, evt);
}

// Called from Audio Rendering Thread
void RESynthMusicDevice::Process(unsigned int nbSamples, float* workBufferL, float* workBufferR)
{
    while(nbSamples != 0)
    {
        // Process MIDI events in front of Queue
        while(!_events.empty() && _events.front().sampleOffset == 0) 
        {
            const ScheduledMidiEvent& evt = _events.front();
            ProcessMidiEvent(evt.cmdByte, evt.data1, evt.data2);
            _events.pop_front();
        }
        
        // Process Samples until next MIDI event
        if(_events.empty()) 
        {
            unsigned int samplesToProcess = nbSamples;
            ProcessSamples(samplesToProcess, workBufferL, workBufferR);
            nbSamples -= samplesToProcess;
            workBufferL += samplesToProcess;
            workBufferR += samplesToProcess;
            AdvanceScheduledMidiEvents(samplesToProcess);
        }
        else
        {
            const ScheduledMidiEvent& nextEvent = _events.front();
            unsigned int samplesToProcess = std::min<unsigned int>(nbSamples, nextEvent.sampleOffset);
            
            // Process Samples
            ProcessSamples(samplesToProcess, workBufferL, workBufferR);
            nbSamples -= samplesToProcess;
            workBufferL += samplesToProcess;
            workBufferR += samplesToProcess;
            AdvanceScheduledMidiEvents(samplesToProcess);
        }
    }        
}

RESynthChannel* RESynthMusicDevice::Channel(int channel)
{
    if(channel >= 0 && channel < NumChannels)
    {
        RESynthChannel* ch = _channels[channel];
        if(ch == NULL)
        {
            ch = _channels[channel] = new RESynthChannel(this);
        }
        return ch;
    }
    return NULL;
}

void RESynthMusicDevice::ProcessNoteOnEvent(uint8_t channel, uint8_t pitch, uint8_t velocity)
{
    RESynthChannel* chan = Channel(channel);
    if(chan) chan->ProcessNoteOnEvent(pitch, velocity);
}

void RESynthMusicDevice::ProcessNoteOffEvent(uint8_t channel, uint8_t pitch)
{
    RESynthChannel* chan = Channel(channel);
    if(chan) chan->ProcessNoteOffEvent(pitch);
}

void RESynthMusicDevice::ProcessProgramChangeEvent(uint8_t channel, uint8_t program)
{
    RESynthChannel* chan = Channel(channel);
    if(chan) chan->ProcessProgramChangeEvent(program);
}

void RESynthMusicDevice::ProcessControllerEvent(uint8_t channel, uint8_t controllerType, uint8_t value)
{
    RESynthChannel* chan = Channel(channel);
    if(chan) chan->ProcessControllerEvent(controllerType, value);
}

void RESynthMusicDevice::ProcessPitchWheelEvent(uint8_t channel, int8_t lsb, int8_t msb)
{
    RESynthChannel* chan = Channel(channel);
    if(chan) chan->ProcessPitchWheelEvent(lsb, msb);
}

void RESynthMusicDevice::ProcessMidiEvent(unsigned int cmdByte, unsigned int data1, unsigned int data2) 
{
    uint8_t cmd = (cmdByte & 0x000000F0)>>4;
    uint8_t channel = cmdByte & 0x0000000F;
    
    switch(cmd)
    {
        case 0x8: {
            // Note Off
            ProcessNoteOffEvent(channel, data1);
            break;
        }
            
        case 0x9: {
            // Note On
            if(data2 == 0) {
                ProcessNoteOffEvent(channel, data1);
            }
            else {
                ProcessNoteOnEvent(channel, data1, data2);
            }
            break;
        }
            
        case 0xB: {
            // Controller
            ProcessControllerEvent(channel, data1, data2);
            break;
        }
            
        case 0xC: {
            // Program Change
            ProcessProgramChangeEvent(channel, data1);
            break;
        }
            
        case 0xE: {
            // Pitch Wheel
            ProcessPitchWheelEvent(channel, data1, data2);
            break;
        }
    }
}

void RESynthMusicDevice::ProcessSamples(unsigned int nbSamples, float* workBufferL, float* workBufferR)
{
    for(unsigned int i=0; i<_channels.size(); ++i)
    {
        RESynthChannel* channel = Channel(i);
        if(channel) channel->ProcessSamples(nbSamples, workBufferL, workBufferR);
    }
}

void RESynthMusicDevice::AdvanceScheduledMidiEvents(unsigned int nbSamples)
{
    unsigned int nbEvents = _events.size();
    for(unsigned int i=0; i<nbEvents; ++i) {
        _events[i].sampleOffset -= nbSamples;
    }
}

void RESynthMusicDevice::SetMidiProgramOfAllChannels(int program, int bank)
{
    for(int i=0; i<16; ++i)
    {
        RESynthChannel* channel = Channel(i);
        channel->ProcessControllerEvent(0, bank);
        channel->ProcessProgramChangeEvent(program);
    }
}




#pragma mark REMusicDevice
REMusicDevice* REMusicDevice::CreateMusicDevice(Reflow::MusicDeviceType type)
{
    REMusicDevice* device = NULL;
    switch(type)
    {       
        case Reflow::SynthMusicDevice:
            device = new RESynthMusicDevice;
            break;
            
        default:
            device = new RESynthMusicDevice;
            break;
    }
    
    device->Create();
    return device;
}

REMusicDevice::REMusicDevice()
: _sampleRate(44100)
{
    
}



#pragma mark RESynthChannel
RESynthChannel::RESynthChannel(RESynthMusicDevice* device)
: _program(0), _bank(0), _device(device), _monophonic(false)
{    
    Initialize();
}

RESynthChannel::~RESynthChannel()
{
    Shutdown();
}

void RESynthChannel::SetMonophonic(bool monophonic)
{
    _monophonic = monophonic;
}

bool RESynthChannel::IsMonophonic() const
{
    return _monophonic;
}

void RESynthChannel::Initialize()
{
    // Create Voices
    for(unsigned int i=0; i<NumVoices; ++i) {
        REMonophonicSynthVoice* voice = new REMonophonicSynthVoice();
        voice->Initialize(_device->SampleRate());
        _voices.push_back(voice);
    }
    
    ProcessProgramChangeEvent(0);
}

void RESynthChannel::Shutdown()
{
    BOOST_FOREACH(REMonophonicSynthVoice* voice, _voices) {
        delete voice;
    }
    _voices.clear();
}

void RESynthChannel::ProcessAllSoundOff()
{
    for(unsigned int i=0; i<_voices.size(); ++i)
    {
        REMonophonicSynthVoice* voice = _voices[i];
        if(voice->IsSounding()) {
            voice->StopNote();
            voice->BrutalStop();
        }
    }
}

void RESynthChannel::ProcessNoteOnEvent(uint8_t pitch, uint8_t velocity)
{
    //REPrintf("SynthMusicDevice::NoteOn %2.2x %2.2x\n", pitch, velocity);
    
    if(_patch == NULL) return;
    
    // Monophonic channels should brutally stop sounding before playing a new note
    if(IsMonophonic())
    {
        ProcessAllSoundOff();
    }
    
    for(unsigned int i=0; i<_patch->GeneratorCount(); ++i)
    {
        RESF2Generator* generator = _patch->Generator(i);
        if(generator->IsPitchInRange(pitch) && generator->IsVelocityInRange(velocity)) 
        {   
            REMonophonicSynthVoice* voiceUsed = NULL;
            
            // Brutal stop notes with the same exclusive class
            if(generator->ExclusiveClass()) 
            {
                for(unsigned int i=0; i<_voices.size(); ++i)
                {
                    REMonophonicSynthVoice* voice = _voices[i];
                    if(voice->ExclusiveClass() == generator->ExclusiveClass()) {
                        voice->BrutalStop();
                    }
                }
            }
            
            // Find a voice with same note
            for(unsigned int i=0; i<_voices.size(); ++i)
            {
                REMonophonicSynthVoice* voice = _voices[i];
                if(voice->Pitch() == pitch) {
                    voiceUsed = voice;
                    voice->BrutalStop();
                    break;
                }
            }
            
            // Find a free voice
            if(voiceUsed == NULL)
            {
                for(unsigned int i=0; i<_voices.size(); ++i)
                {
                    REMonophonicSynthVoice* voice = _voices[i];
                    if(!voice->IsSounding()) {
                        voiceUsed = voice;
                        break;
                    }
                }
            }
            
            if(voiceUsed == NULL){
                voiceUsed = _voices[0];
                voiceUsed->BrutalStop();
            }
            
            voiceUsed->PlayNote(generator, pitch, velocity);
            
            // TEMP HACK: use only first generator
            break;
        }
    }
}

void RESynthChannel::ProcessNoteOffEvent(uint8_t pitch)
{
    for(unsigned int i=0; i<_voices.size(); ++i)
    {
        REMonophonicSynthVoice* voice = _voices[i];
        if(voice->IsOn() && voice->Pitch() == pitch) {
            voice->StopNote();
        }
    } 
}

void RESynthChannel::ProcessProgramChangeEvent(uint8_t program)
{
    _program = program;
    
    RESoundFont* soundfont = _device->SoundFont();
    if(soundfont == NULL) return;
    
    _patch = soundfont->Patch(_program, _bank);
}

void RESynthChannel::ProcessControllerEvent(uint8_t controllerType, uint8_t value)
{
    switch(controllerType)
    {
        case 0: {
            // Bank Select
            _bank = value;
            break;
        }
            
        case 120: {
            // All sound off
            ProcessAllSoundOff();
            break;
        }
            
        default: {
            break;
        }
    }
}

void RESynthChannel::ProcessPitchWheelEvent(int8_t lsb, int8_t msb)
{
    double bend = (((msb & 0x7F) << 7) | (lsb & 0x7F)) / 16384.0;       
    bend = (bend - 0.5) * 48.0;                                     // [0 .. 1] <-> [-24 steps .. 24 steps]
    
    for(unsigned int i=0; i<_voices.size(); ++i)
    {
        REMonophonicSynthVoice* voice = _voices[i];
        voice->SetPitchWheel(bend);
    }
}

void RESynthChannel::ProcessSamples(unsigned int nbSamples, float* workBufferL, float* workBufferR)
{
    for(unsigned int i=0; i<_voices.size(); ++i)
    {
        REMonophonicSynthVoice* voice = _voices[i];
        if(voice->IsSounding()) {
            voice->Process(nbSamples, workBufferL, workBufferR, _device->Volume(), _device->Pan());
        }
    }
}

