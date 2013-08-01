//
//  RESF2GeneratorPlayer.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 11/08/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include <iostream>

#ifdef _WIN32
#  define _USE_MATH_DEFINES 
#  include <math.h>
#else
#  include <cmath>
#endif

#include "RESF2GeneratorPlayer.h"
#include "RESF2Generator.h"
#include "RESF2Patch.h"
#include "RESoundFont.h"
#include "REFunctions.h"

RESF2GeneratorPlayer::RESF2GeneratorPlayer(float sampleRate)
: _generator(NULL), _playing(false), _time(0), _freqMod(1.0), _ampMod(1.0), _currentVolEnvPhase(Reflow::InitialPhase), _volEnvTime(0), _volEnvPhaseDuration(0), _noteOn(false), _releaseSustain(0), _sampleRate(sampleRate),
    _a0(1.0), _b1(0.0), _memSampleL(0.0), _memSampleR(0.0)
{
}

RESF2GeneratorPlayer::~RESF2GeneratorPlayer()
{
}

void RESF2GeneratorPlayer::SetGenerator(RESF2Generator* generator)
{
    _generator = generator;
    
    _delay = _generator->VolumeEnvelopeDelay();
    _attack = _generator->VolumeEnvelopeAttack();
    _hold = _generator->VolumeEnvelopeHold();
    _decay = _generator->VolumeEnvelopeDecay();
    _sustain = _generator->VolumeEnvelopeSustain();
    _release = _generator->VolumeEnvelopeRelease();
    
    _invDelay   = (_delay   != 0 ? 1.0 / _delay   : 0.0);
    _invAttack  = (_attack  != 0 ? 1.0 / _attack  : 0.0);
    _invHold    = (_hold    != 0 ? 1.0 / _hold    : 0.0);
    _invDecay   = (_decay   != 0 ? 1.0 / _decay   : 0.0);
    _invSustain = (_sustain != 0 ? 1.0 / _sustain : 0.0);
    _invRelease = (_release != 0 ? 1.0 / _release : 0.0); 
}

void RESF2GeneratorPlayer::SetFreqModifier(float freqModifier)
{
    _freqMod = freqModifier;
}
void RESF2GeneratorPlayer::SetAmpModifier(float ampModifier)
{
    _ampMod = ampModifier;
}

void RESF2GeneratorPlayer::Start()
{
    if(_generator == NULL) return;
    
    _noteOn = true;
    _playing = true;
    _time = 0.0;
}

void RESF2GeneratorPlayer::NoteOff()
{
    if(_generator == NULL) return;
    
    _noteOn = false;
}

void RESF2GeneratorPlayer::Stop()
{
    _playing = false;
}

bool RESF2GeneratorPlayer::IsSounding() const
{
    return _generator != NULL && _playing;
}

void RESF2GeneratorPlayer::Process(unsigned int nbFrames, float* workBufferL, float* workBufferR, float volume, float pan)
{
    if(!_playing || _generator == NULL) return;
    
    RESF2Patch* patch = _generator->Patch();
    if(!patch) return;
    
    RESoundFont* soundfont = patch->SoundFont();
    if(!soundfont) return;
    
    const RESoundFont::SF2Sample* sample = soundfont->Sample(_generator->_sampleID);
    
    const int16_t* data = soundfont->SampleData() + sample->dwStart;
    bool loops = false; 
    if(_generator->LoopType() == RESF2Generator::LoopForever) {
        loops = true;
    }
    else if(_generator->LoopType() == RESF2Generator::LoopWhileOn) {
        loops = _noteOn;
    }
    
    float startLoop = sample->dwStartloop - sample->dwStart;
    float endLoop = (loops ? sample->dwEndloop - sample->dwStart : sample->dwEnd - sample->dwStart);
    float speed = _freqMod;
    float panL = (1.0f - _generator->_pan);
    float ppanL = (1.0f - pan);
    float panR = _generator->_pan;
    float ppanR = pan;
    
    float sustain = _generator->VolumeEnvelopeSustain();
    
    float finalAmpModL = panL * ppanL * volume * _ampMod;
    float finalAmpModR = panR * ppanR * volume * _ampMod;
    
    int nbEmptyL = 0;
    int nbEmptyR = 0;
    
    // Low pass filtering recursion coefficients
    if(_generator->IsLowpassFilterEnabled()) 
    {
        float fc = _generator->_lowpassFC / _sampleRate;
        float x = expf(-2.0 * M_PI * fc);
        x = Reflow::Clamp<float>(x, 0.0, 1.0);
        _a0 = 1.0 - x;
        _b1 = x;
    }
    
    float inv32000 = 1.0 / 32000.0f;
    for(unsigned int i=0; i<nbFrames; ++i)
    {
        unsigned long pos = (unsigned long)_time;
        float sample = (float)data[pos] * inv32000;
        
        float ampEnvelope = 0;
        
        // Volume Envelope
        switch(_currentVolEnvPhase)
        {
            case Reflow::InitialPhase: 
            {
                // Continue to delay phase
                _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                _volEnvTime = 0;
                _volEnvPhaseDuration = _delay;
                _inverseVolEnvPhaseDuration = _invDelay;
            }
                
            case Reflow::DelayPhase: 
            {
                if(_volEnvTime >= _volEnvPhaseDuration) 
                {
                    // Continue to Attack Phase
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    _volEnvTime -= _volEnvPhaseDuration;
                    _volEnvPhaseDuration = _attack;
                    _inverseVolEnvPhaseDuration = _invAttack;
                }
                else break;
            }
                   
            case Reflow::AttackPhase: 
            {
                if(_volEnvTime >= _volEnvPhaseDuration) 
                {
                    // Continue to Hold Phase
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    _volEnvTime -= _volEnvPhaseDuration;
                    _volEnvPhaseDuration = _hold;
                    _inverseVolEnvPhaseDuration = _invHold;
                }
                else {
                    ampEnvelope = _volEnvTime * _inverseVolEnvPhaseDuration;        // [0 .. 1]
                    if(!_noteOn) {
                        // Abort and directly goto release phase with current enveloppe as sustain level
                        _releaseSustain = ampEnvelope;
                        _currentVolEnvPhase = Reflow::ReleasePhase;
                        _volEnvTime = 0.0;
                        _volEnvPhaseDuration = _release;
                        _inverseVolEnvPhaseDuration = _invRelease;
                    }
                    break;
                }
            }
                
            case Reflow::HoldPhase: 
            {
                if(_volEnvTime >= _volEnvPhaseDuration) 
                {
                    // Continue to Decay Phase
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    _volEnvTime -= _volEnvPhaseDuration;
                    _volEnvPhaseDuration = _decay;
                    _inverseVolEnvPhaseDuration = _invDecay;
                }
                else {
                    ampEnvelope = 1.0;
                    if(!_noteOn) {
                        // Abort and directly goto release phase with current enveloppe as sustain level
                        _releaseSustain = ampEnvelope;
                        _currentVolEnvPhase = Reflow::ReleasePhase;
                        _volEnvTime = 0.0;
                        _volEnvPhaseDuration = _release;
                        _inverseVolEnvPhaseDuration = _invRelease;
                    }
                    break;
                }
            }
                
            case Reflow::DecayPhase: 
            {
                if(_volEnvTime >= _volEnvPhaseDuration) 
                {
                    // Continue to Sustain Phase
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    _volEnvTime -= _volEnvPhaseDuration;
                    _volEnvPhaseDuration = 0;
                    _inverseVolEnvPhaseDuration = 0;
                }
                else {
                    float t = _volEnvTime * _inverseVolEnvPhaseDuration;
                    ampEnvelope = (1.0 - t) + t * sustain;      // [1 .. sustain]
                    if(!_noteOn) {
                        // Abort and directly goto release phase with current enveloppe as sustain level
                        _releaseSustain = ampEnvelope;
                        _currentVolEnvPhase = Reflow::ReleasePhase;
                        _volEnvTime = 0.0;
                        _volEnvPhaseDuration = _release;
                        _inverseVolEnvPhaseDuration = _invRelease;
                    }
                    break;
                }
            }
                
            case Reflow::SustainPhase: 
            {
                if(sustain < 0.001) {
                    _currentVolEnvPhase = Reflow::FinalPhase;
                    break;
                }
                
                if(_noteOn) {
                    ampEnvelope = sustain;
                    break;
                }
                else {
                    // Continue to Release Phase
                    _releaseSustain = sustain;
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    _volEnvTime -= _volEnvPhaseDuration;
                    _volEnvPhaseDuration = _release;
                    _inverseVolEnvPhaseDuration = _invRelease;
                }
            }
                
            case Reflow::ReleasePhase: 
            {
                if(_volEnvTime >= _volEnvPhaseDuration) 
                {
                    // Finished
                    _currentVolEnvPhase = static_cast<Reflow::EnvelopePhase>((int)_currentVolEnvPhase + 1);
                    break;
                }
                else {
                    float t = _volEnvTime * _inverseVolEnvPhaseDuration;
                    ampEnvelope = (1.0 - t) * _releaseSustain;      // [sustain .. 0]
                    break;
                }    
            }
                
            default: break;
        }
         
        // Final Phase
        if(_currentVolEnvPhase == Reflow::FinalPhase)
        {
            Stop();
            break;
        }
        
        // Modulate sample
        float sampleL = sample * finalAmpModL * ampEnvelope;
        float sampleR = sample * finalAmpModR * ampEnvelope;

        if(_generator->IsLowpassFilterEnabled())
        {
            sampleL = _a0 * sampleL + _b1 * _memSampleL;
            _memSampleL = sampleL;
            
            sampleR = _a0 * sampleR + _b1 * _memSampleR;
            _memSampleR = sampleR;
        }
        
        *workBufferL += sampleL;
        ++workBufferL;
        
        *workBufferR += sampleR;
        ++workBufferR;
                
        _volEnvTime += speed;
        _time += speed;
        
        // Looping
        if(loops) {
            if(_time >= endLoop) {
                _time -= (endLoop - startLoop);
            }
        }
        else {
            if(_time >= endLoop) {
                Stop();
                break;
            }
        }
    }
}
