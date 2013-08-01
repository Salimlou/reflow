//
//  REMonophonicSynthVoice.h
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMonophonicSynthVoice_h
#define Reflow_REMonophonicSynthVoice_h

#include "RETypes.h"

class REMonophonicSynthVoice
{
public:
    REMonophonicSynthVoice();
    ~REMonophonicSynthVoice();
    
    void Initialize(float sampleRate);
    
    bool IsSounding() const;
    bool IsOn() const;
    bool IsCrossFading() const;
    unsigned int Pitch() const;
    
    void PlayNote(REMonoSample* sample, uint8_t pitch, uint8_t velocity);
    void PlayNote(RESF2Generator* sf2Gen, uint8_t pitch, uint8_t velocity);
    void StopNote();
    void BrutalStop();
    void Process(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan);
    
    void SetPitchWheel(double pitchWheel);
    
    void SetExclusiveClass(int ec) {_exclusiveClass = ec;}
    int ExclusiveClass() const {return _exclusiveClass;}
    
private:
    void _StartXFade();
    void _ProcessWithXFade(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan);
    void _ProcessWithoutXFade(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan);
    
private:
    bool _on;
    uint8_t _pitch;
    int _exclusiveClass;
    float _sampleRate;
    double _pitchWheel;                 // in steps (12.0 = 1 octave pitch bend)
    RESampleGenerator* _player;
    RESampleGenerator* _xfadePlayer;
    int _xfadePos;
};

#endif
