//
//  REAudioEngine.cpp
//  Reflow
//
//  Created by Sebastien on 11/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REAudioEngine.h"
#include "REMusicRack.h"
#include "REMusicDevice.h"
#include "RESoundFont.h"
#include "RESoundFontManager.h"

REAudioEngine* REAudioEngine::_instance = nullptr;

REAudioEngine* REAudioEngine::Instance()
{
    return _instance;
}

REAudioEngine::REAudioEngine()
    : _monitorRack(NULL), _soundfont(NULL), _sampleRate(44100)
{
}
REAudioEngine::~REAudioEngine()
{
    
}


void REAudioEngine::OnMidiPacketReceived(const REMidiInstantPacket* inPacket)
{
    _instantMidiPacketBuffer.Push(*inPacket);
}


void REAudioEngine::AddRack(REMusicRack* rack)
{
    rack->SetSampleRate(_sampleRate);
    
    // CRITICAL: Add music rack to vector (Called from Main Thread)
    {
        MutexLocker lock(_mtx);
        _racks.push_back(rack);
        rack->_audioEngine = this;
    }
}

void REAudioEngine::RemoveRack(REMusicRack* rack)
{
    // CRITICAL: Remove music rack from vector (Called from Main Thread)
    {
        MutexLocker lock(_mtx);
        _racks.erase(std::find(_racks.begin(), _racks.end(), rack), _racks.end());
        rack->_audioEngine = NULL;
    }
}

void REAudioEngine::LoadDefaultSoundFont()
{
    _soundfont = RESoundFontManager::Instance().DefaultSoundFont();
}

void REAudioEngine::Initialize(const REAudioSettings& settings)
{
    _settings = settings;
    
    // Open SoundFont
    LoadDefaultSoundFont();
}

void REAudioEngine::Shutdown()
{
}

void REAudioEngine::StartRendering()
{
}

void REAudioEngine::StopRendering()
{
}

void REAudioEngine::PlayInstantClipOnMonitoringDevice(const REMidiClip& clip, double bpm, int dpitch, bool appendSoundOffEvent)
{
    REInstantMidiClip instantClip(clip, bpm, dpitch);
    
    if(appendSoundOffEvent) {
        int tick = instantClip.MaxTick() + REFLOW_PULSES_PER_QUARTER;
        for(int ch=0; ch<16; ++ch)
        {
            REMidiEvent soundOff;
            soundOff.tick = tick;
            soundOff.data[0] = 0xB0 | ch;
            soundOff.data[1] = 120;       // all sound off
            soundOff.data[2] = 0;
            instantClip.AddEvent(soundOff);
        }
    }
    
    _instantMidiClipBuffer.Push(instantClip);
}

void REAudioEngine::_RouteInstantMidiClipToDevice(REMusicDevice* device, const REInstantMidiClip* clip)
{
    double bpm = clip->BPM();
    double sampleRate = device->SampleRate();
    int dpitch = clip->DeltaPitch();
    
    // Events
    unsigned int nbEvents = clip->EventCount();
    for(unsigned int i=0; i<nbEvents; ++i)
    {
        const REMidiEvent& evt = clip->Event(i);
        double t = (double)evt.tick / (double)REFLOW_PULSES_PER_QUARTER;
        uint32_t delay = (uint32_t)((sampleRate * t * 60.0) / bpm);
        device->MidiEvent(evt.data[0],
                          evt.data[1],
                          evt.data[2],
                          delay);
    }
    
    // Note Events
    unsigned int nbNoteEvents = clip->NoteEventCount();
    for(unsigned int i=0; i<nbNoteEvents; ++i)
    {
        const REMidiNoteEvent& evt = clip->NoteEvent(i);
        double t = (double)evt.tick / (double)REFLOW_PULSES_PER_QUARTER;
        double tlast = (double)(evt.tick + evt.duration) / (double)REFLOW_PULSES_PER_QUARTER;
        
        // Note On
        uint32_t delay = (uint32_t)((sampleRate * t * 60.0) / bpm);
        device->MidiEvent(0x90 | evt.channel,
                          evt.pitch + dpitch, 
                          evt.velocity, 
                          delay);
        
        // Note Off
        delay = delay = (uint32_t)((sampleRate * tlast * 60.0) / bpm);
        device->MidiEvent(0x80 | evt.channel, 
                          evt.pitch + dpitch, 
                          0, 
                          delay);
    }
}

const REAudioSettings& REAudioEngine::AudioSettings() const
{
    return _settings;
}

void REAudioEngine::SetAudioSettings(const REAudioSettings& settings)
{
    //CRITICAL: We can't modify audio settings during the audio callback
    REAudioEngine::MutexLocker lock_(_mtx);
    
    _settings = settings;
}
