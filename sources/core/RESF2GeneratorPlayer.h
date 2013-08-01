//
//  RESF2GeneratorPlayer.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 11/08/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESF2GeneratorPlayer_h
#define Reflow_RESF2GeneratorPlayer_h

#include "RETypes.h"
#include "RESampleGenerator.h"

class RESF2GeneratorPlayer : public RESampleGenerator
{
    friend class REMonophonicSynthVoice;
    
public:
    RESF2GeneratorPlayer(float sampleRate);
    virtual ~RESF2GeneratorPlayer();
    
    void SetFreqModifier(float freqModifier);
    void SetAmpModifier(float ampModifier);
    
    void Start();
    void NoteOff();
    void Stop();
    bool IsSounding() const;
    
    void Process(unsigned int nbFrames, float* workBufferL, float* workBufferR, float volume, float pan);
    
public:
    void SetGenerator(RESF2Generator* generator);
    
private:
    float _sampleRate;
    float _time, _freqMod, _ampMod;
    RESF2Generator* _generator;
    bool _playing;
    bool _noteOn;
    
    // AHDSR Envelope
    Reflow::EnvelopePhase _currentVolEnvPhase;
    float _volEnvTime, _volEnvPhaseDuration, _inverseVolEnvPhaseDuration;
    float _releaseSustain;
    
    float _delay, _attack, _hold, _decay, _sustain, _release;
    float _invDelay, _invAttack, _invHold, _invDecay, _invSustain, _invRelease;
    
    float _a0, _b1, _memSampleL, _memSampleR;       // These values are for Low Pass Filter
};


#endif
