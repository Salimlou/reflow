//
//  REMusicRack.h
//  Reflow
//
//  Created by Sebastien on 11/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMusicRack_h
#define Reflow_REMusicRack_h

#include "RETypes.h"

#include <mutex>

class RESynthMusicDevice;
class REAudioEngine;

class REMusicRack
{
    friend class REAudioEngine;
    friend class RESequencer;
    
public:
    typedef std::recursive_mutex MutexType;
    typedef std::lock_guard<MutexType> MutexLocker;
    
public:
    REMusicRack ();
    ~REMusicRack();
    
    void SetRenderingEnabled(bool rendering);
    bool RenderingEnabled() const;
    void Render(unsigned int nbSamples, float* workBufferL, float* workBufferR);
    
    void SetDelegate(REMusicRackDelegate* delegate);
    REMusicRackDelegate* Delegate() {return _delegate;}
    const REMusicRackDelegate* Delegate() const {return _delegate;}
    
    void SetSampleRate(double sampleRate);
    double SampleRate() const;
    
    const REMusicDevice* Device(int idx) const;
    REMusicDevice* Device(int idx);
    unsigned int DeviceCount() const;
    int IndexOfDevice(const REMusicDevice* device) const;
    
    REMusicDevice* CreateMusicDevice();
    void DestroyMusicDevice(REMusicDevice*);
    void DestroyAllMusicDevices();
    int IndexOfMusicDeviceWithUUID(int32_t uuid) const;
    
    RESynthMusicDevice* MetronomeDevice();
    RESynthMusicDevice* CreateMetronomeDevice();
    void DestroyMetronomeDevice();
    
    MutexType& Mutex() {return _mtx;}
    
public:
    void _RenderFrames(unsigned int nbFrames);
    void _RenderFramesInAutoreleasePool(unsigned int nbFrames);
    void _RenderTrack(unsigned int trackIndex, double t0, double t1);
    
protected:
    REMusicDevice* _NewMusicDevice(int32_t uuid);
    
private:
    double _sampleRate;
    bool _renderingEnabled;
    MutexType _mtx;
    REMusicDeviceVector _devices;
    RESynthMusicDevice* _metronomeDevice;
    REMusicRackDelegate* _delegate;
    REAudioEngine* _audioEngine;
};

#endif
