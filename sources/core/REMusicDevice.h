//
//  REMusicDevice.h
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMusicDevice_h
#define Reflow_REMusicDevice_h

#include <deque>

#include "RETypes.h"

class REMusicDeviceImpl;

class REMusicDevice : public REAudioStream
{   
    friend class REMusicRack;
    
public:
    static REMusicDevice* CreateMusicDevice(Reflow::MusicDeviceType type);

    virtual ~REMusicDevice() {}
    
public:
    void SetUUID(int32_t uuid) {_uuid = uuid;}
    int32_t UUID() const {return _uuid;}
    
    double SampleRate() const {return _sampleRate;}
    
public:
    virtual Reflow::MusicDeviceType Type() const = 0;
    virtual void MidiEvent(unsigned int cmdByte, unsigned int data1, unsigned int data2, unsigned int sampleOffset) = 0;
    virtual void SetVolume(float vol) = 0;
    virtual void SetPan(float pan) = 0;
    
protected:
    REMusicDevice();
    
    virtual void Create() = 0;
    virtual void Initialize(double sampleRate) = 0;
    virtual void Shutdown() =0;
    virtual void Process(unsigned int nbSamples, float* workBufferL, float* workBufferR)=0;
    
protected:
    double _sampleRate;
    int32_t _uuid;
};


class RESynthMusicDevice;


class RESynthChannel
{
    friend class RESynthMusicDevice;
    
public:
    enum {
        NumVoices = 16
    };
    
public:
    RESynthChannel(RESynthMusicDevice* device);
    ~RESynthChannel();
    
    void SetMonophonic(bool monophonic);
    bool IsMonophonic() const;
    
protected:
    void ProcessNoteOnEvent(uint8_t pitch, uint8_t velocity);
    void ProcessNoteOffEvent(uint8_t pitch);
    void ProcessProgramChangeEvent(uint8_t program);
    void ProcessControllerEvent(uint8_t controllerType, uint8_t value);
    void ProcessPitchWheelEvent(int8_t lsb, int8_t msb);
    void ProcessSamples(unsigned int nbSamples, float* workBufferL, float* workBufferR);
    void ProcessAllSoundOff();
    
    void Initialize();
    void Shutdown();
    
private:
    int _program;
    int _bank;
    bool _monophonic;
    RESF2Patch* _patch;
    REMonophonicSynthVoiceVector _voices;
    RESynthMusicDevice* _device;
};




class RESynthMusicDevice : public REMusicDevice
{
    friend class RESequencer;
    
public:
    enum {
        NumChannels = 16
    };
    
    struct ScheduledMidiEvent {
        uint32_t sampleOffset;
        uint8_t cmdByte;
        uint8_t data1;
        uint8_t data2;
    };
    typedef std::deque<ScheduledMidiEvent> ScheduledMidiEventDeque;
    
    RESynthMusicDevice();
    RESynthMusicDevice(double sampleRate, RESoundFont* soundfont=NULL);
    virtual ~RESynthMusicDevice();
    
    
public:
    virtual Reflow::MusicDeviceType Type() const;    
    virtual void Create();
    virtual void Initialize(double sampleRate);
    virtual void Shutdown();
    
    // Called for Audio Rendering Thread
    virtual void MidiEvent(unsigned int cmdByte, unsigned int data1, unsigned int data2, unsigned int sampleOffset) ;
    
    // Called from Audio Rendering Thread
    virtual void Process(unsigned int nbSamples, float* workBufferL, float* workBufferR);
    
    virtual void SetVolume(float vol);
    virtual void SetPan(float pan);
    
    float Volume() const {return _volume;}
    float Pan() const {return _pan;}
    
    RESoundFont* SoundFont() {return _soundfont;}
    void SetSoundFont(RESoundFont* sf) {_soundfont = sf;}
    
    void SetMidiProgramOfAllChannels(int program, int bank);
    
protected:
    void ProcessNoteOnEvent(uint8_t channel, uint8_t pitch, uint8_t velocity);    
    void ProcessNoteOffEvent(uint8_t channel, uint8_t pitch);    
    void ProcessProgramChangeEvent(uint8_t channel, uint8_t program);
    void ProcessControllerEvent(uint8_t channel, uint8_t controllerType, uint8_t value);
    void ProcessPitchWheelEvent(uint8_t channel, int8_t lsb, int8_t msb);
    void ProcessMidiEvent(unsigned int cmdByte, unsigned int data1, unsigned int data2) ;    
    void ProcessSamples(unsigned int nbSamples, float* workBufferL, float* workBufferR);    
    void AdvanceScheduledMidiEvents(unsigned int nbSamples);    
    
    RESynthChannel* Channel(int channel);
    
protected:
    ScheduledMidiEventDeque _events;
    RESynthChannelVector _channels;
    RESoundFont* _soundfont;
    float _volume;
    float _pan;
};


#endif
