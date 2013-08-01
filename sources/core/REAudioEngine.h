//
//  REAudioEngine.h
//  Reflow
//
//  Created by Sebastien on 11/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REAudioEngine_h
#define Reflow_REAudioEngine_h

#include "RETypes.h"
#include "REMidiClip.h"
#include "REAudioSettings.h"

#include <mutex>


class REAudioEngine : public REMidiInputListener
{
public:
    typedef std::recursive_mutex MutexType;
    typedef std::lock_guard<MutexType> MutexLocker;
    
public:
    static REAudioEngine* Instance();

public:
    virtual void Initialize(const REAudioSettings& settings);
    virtual void Shutdown();
    
    virtual void StartRendering();
    virtual void StopRendering();
    
    double SampleRate() const {return _sampleRate;}

    const REAudioSettings& AudioSettings() const;
    void SetAudioSettings(const REAudioSettings& settings);
    
    void AddRack(REMusicRack* rack);
    void RemoveRack(REMusicRack* rack);
    
    RESoundFont* SoundFont() {return _soundfont;}
    
    void PlayInstantClipOnMonitoringDevice(const REMidiClip& clip, double bpm, int dpitch, bool appendSoundOffEvent=false);
    
protected:
    REAudioEngine();
    virtual ~REAudioEngine();
    
    void LoadDefaultSoundFont();
    void _RouteInstantMidiClipToDevice(REMusicDevice* device, const REInstantMidiClip* clip);
    
protected:
    static REAudioEngine* _instance;
    double _sampleRate;
    REMidiInstantPacketRingBuffer _instantMidiPacketBuffer;
    REInstantMidiClipRingBuffer _instantMidiClipBuffer;
    REMusicRackVector _racks;
    REMusicRack* _monitorRack;
    REMusicDevice* _monitorDevice;
    RESoundFont* _soundfont;
    REAudioSettings _settings;
    mutable MutexType _mtx;
    
public:
    virtual void OnMidiPacketReceived(const REMidiInstantPacket* inPacket);
};

#endif
