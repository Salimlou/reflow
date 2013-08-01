//
//  REMusicRack.cpp
//  Reflow
//
//  Created by Sebastien on 11/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REMusicRack.h"
#include "REMusicDevice.h"
#include "REAudioEngine.h"

#include <boost/foreach.hpp>

REMusicRack::REMusicRack()
: _renderingEnabled(false), _delegate(NULL), _metronomeDevice(NULL), _audioEngine(NULL)
{
    
}

REMusicRack::~REMusicRack()
{
    DestroyMetronomeDevice();
    DestroyAllMusicDevices();
}

void REMusicRack::SetDelegate(REMusicRackDelegate* delegate)
{
    // CRITICAL: Prevent rack from being rendered while we modify some parameters
    MutexLocker lock_(_mtx);
    
    _delegate = delegate;
}

void REMusicRack::SetRenderingEnabled(bool rendering)
{
    // CRITICAL: Prevent rack from being rendered while we modify some parameters
    MutexLocker lock_my_rack_please(_mtx);
    
    _renderingEnabled = rendering;
}
bool REMusicRack::RenderingEnabled() const
{
    return _renderingEnabled;
}

void REMusicRack::Render(unsigned int nbSamples, float* workBufferL, float* workBufferR)
{
    // CRITICAL: Do not modify a music rack while it is being processed
    MutexLocker lock_my_rack_please(_mtx);
 
    if(!_renderingEnabled) return;

    const REAudioSettings& audioSettings = (_audioEngine ? _audioEngine->AudioSettings() : REAudioSettings::DefaultAudioSettings());
    
    // Will Render Rack
    if(_delegate) _delegate->WillRenderRack(this, nbSamples, workBufferL, workBufferR);
    
    for(unsigned int i=0; i<_devices.size(); ++i)
    {
        REMusicDevice* device = _devices[i];
        
        // Will Render Device
        if(_delegate) _delegate->WillRenderDevice(this, device, nbSamples, workBufferL, workBufferR);
        
        // Render Device
        device->Process(nbSamples, workBufferL, workBufferR);

        // Did Render Device
        if(_delegate) _delegate->DidRenderDevice(this, device, nbSamples, workBufferL, workBufferR);
    }
    
    // Render Metronome
    if(_metronomeDevice && audioSettings.MetronomeEnabled())
    {
        _metronomeDevice->SetVolume(audioSettings.MetronomeGain());
        _metronomeDevice->Process(nbSamples, workBufferL, workBufferR);
    }
    
    // Did Render Rack
    if(_delegate) _delegate->DidRenderRack(this, nbSamples, workBufferL, workBufferR);
}

RESynthMusicDevice* REMusicRack::MetronomeDevice()
{
    return _metronomeDevice;
}

RESynthMusicDevice* REMusicRack::CreateMetronomeDevice()
{    
    REMusicDevice* device = REMusicDevice::CreateMusicDevice(Reflow::SynthMusicDevice);
    device->Initialize(_sampleRate);
    RESynthMusicDevice* synth = static_cast<RESynthMusicDevice*>(device);
    synth->SetMidiProgramOfAllChannels(0, 128);
    
    // CRITICAL: Do not process rack while creating device
    {
        MutexLocker lock_my_rack_please(_mtx);
        _metronomeDevice = synth;
    }
    
    return synth;
}

void REMusicRack::DestroyMetronomeDevice()
{
    if(_metronomeDevice == NULL) return;
    
    // CRITICAL: Do not process rack while creating device
    {
        MutexLocker lock_my_rack_please(_mtx);
        delete _metronomeDevice;
        _metronomeDevice = NULL;
    }
}

const REMusicDevice* REMusicRack::Device(int idx) const
{
    if(idx >= 0 && idx < DeviceCount()) {
        return _devices[idx];
    }
    return NULL;
}

REMusicDevice* REMusicRack::Device(int idx)
{
    if(idx >= 0 && idx < DeviceCount()) {
        return _devices[idx];
    }
    return NULL;    
}

int REMusicRack::IndexOfDevice(const REMusicDevice* device) const
{
    for(int i=0; i<_devices.size(); ++i) {
        if(_devices[i] == device) {
            return i;
        }
    }
    return -1;
}

unsigned int REMusicRack::DeviceCount() const 
{
    return (unsigned int)_devices.size();
}

void REMusicRack::SetSampleRate(double sampleRate)
{
    _sampleRate = sampleRate;
}

double REMusicRack::SampleRate() const
{
    return _sampleRate;
}

int REMusicRack::IndexOfMusicDeviceWithUUID(int32_t uuid) const
{
    for(int i=0; i<_devices.size(); ++i) {
        if(_devices[i]->UUID() == uuid) {
            return i;
        }
    }
    return -1;
}

REMusicDevice* REMusicRack::_NewMusicDevice(int32_t uuid)
{
    REMusicDevice* device = REMusicDevice::CreateMusicDevice(Reflow::SynthMusicDevice);
    device->SetUUID(uuid);
    device->Initialize(_sampleRate);
    RESynthMusicDevice* synth = static_cast<RESynthMusicDevice*>(device);
    return synth;
}

REMusicDevice* REMusicRack::CreateMusicDevice()
{
    REMusicDevice* device = REMusicDevice::CreateMusicDevice(Reflow::SynthMusicDevice);
    device->Initialize(_sampleRate);
    RESynthMusicDevice* synth = static_cast<RESynthMusicDevice*>(device);
    
    // CRITICAL: Do not process rack while creating device
    {
        MutexLocker lock_my_rack_please(_mtx);
        _devices.push_back(device);
    }
    
    return device;
}

void REMusicRack::DestroyMusicDevice(REMusicDevice* device)
{
    // CRITICAL: Do not process rack while destroying a device
    {
        MutexLocker lock_my_rack_please(_mtx);
        _devices.erase(std::find(_devices.begin(), _devices.end(), device), _devices.end());
    }
    delete device;
}

void REMusicRack::DestroyAllMusicDevices()
{
    // CRITICAL: Do not process rack while destroying a device
    {
        MutexLocker lock_my_rack_please(_mtx);
        
        BOOST_FOREACH(REMusicDevice* dev, _devices) {
            delete dev;
        }
        _devices.clear();
    }
}
