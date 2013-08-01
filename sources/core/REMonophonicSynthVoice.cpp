//
//  REMonophonicSynthVoice.cpp
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REMonophonicSynthVoice.h"
#include "REMonoSample.h"
#include "RESamplePlayer.h"
#include "RESF2GeneratorPlayer.h"
#include "RESF2Generator.h"
#include "RESF2Patch.h"
#include "RESoundFont.h"
#include "REFunctions.h"

#define REFLOW_XFADE_SIZE  (64)

REMonophonicSynthVoice::REMonophonicSynthVoice()
: _on(false), _pitch(0), _sampleRate(44100.0), _player(NULL), _xfadePlayer(NULL), _xfadePos(REFLOW_XFADE_SIZE), _pitchWheel(0.0), _exclusiveClass(0)
{    
}

REMonophonicSynthVoice::~REMonophonicSynthVoice()
{
    if(_player) {delete _player; _player = NULL;}
    if(_xfadePlayer) {delete _xfadePlayer; _xfadePlayer = NULL;}
}

void REMonophonicSynthVoice::Initialize(float sampleRate)
{
    _sampleRate = sampleRate;
}

bool REMonophonicSynthVoice::IsCrossFading() const
{
    return _xfadePos < REFLOW_XFADE_SIZE;
}

bool REMonophonicSynthVoice::IsSounding() const
{
    if(IsCrossFading()) {
        return _player != NULL || _xfadePlayer != NULL;
    }
    else return (_player != NULL ? _player->IsSounding() : false);
}

bool REMonophonicSynthVoice::IsOn() const
{
    return _on;
}

unsigned int REMonophonicSynthVoice::Pitch() const
{
    return _pitch;
}

void REMonophonicSynthVoice::_StartXFade()
{
    if(_xfadePlayer) {delete _xfadePlayer; _xfadePlayer = NULL;}
    _xfadePlayer = _player;
    _player = NULL;
    _xfadePos = 0;
}

void REMonophonicSynthVoice::StopNote()
{
    _on = false;
    
    if(_player) _player->NoteOff();
}
void REMonophonicSynthVoice::PlayNote(REMonoSample* sample, uint8_t pitch, uint8_t velocity)
{
    assert(false && "Using REMonoSample is deprecated");
    _StartXFade();
    
    // Calculate freq modifier with pitch
    _pitch = pitch;
    float pitchFreq = Reflow::MidiToFrequency(_pitch);
    
    float freqMod = pitchFreq / sample->SourceFreq();
    float ampMod = (float)velocity / 127.0;
    
    _player = new RESamplePlayer;
    _on = true;
    static_cast<RESamplePlayer*>(_player)->SetSample(sample);
    _player->SetFreqModifier(freqMod);
    _player->SetAmpModifier(ampMod);
    _player->Start();
}

void REMonophonicSynthVoice::SetPitchWheel(double pitchWheel)
{
    _pitchWheel = pitchWheel;
    
    if(_player == NULL) return;
    
    RESF2GeneratorPlayer* sf2Player = static_cast<RESF2GeneratorPlayer*>(_player);
    RESF2Generator* sf2Gen = sf2Player->_generator;
    
    // Apply coarseTune to pitch we have to play
    int pitch = _pitch;
    int pitch2 = pitch;
    pitch2 += sf2Gen->_coarseTune;
    if(pitch2 > 127) {pitch = 127;}
    if(pitch2 < 0) pitch2 = 0;
    pitch = pitch2;
    
    double midi = (double)pitch + _pitchWheel;
    float pitchFreq = Reflow::MidiToFrequency(midi);
    float freqMod = 1.0;
    
    RESF2Patch* patch = sf2Gen->Patch();
    RESoundFont* soundfont = patch->SoundFont();
    const RESoundFont::SF2Sample* sample = soundfont->Sample(sf2Gen->_sampleID);
    
    float sampleOriginFrequency = Reflow::MidiToFrequency(sample->byOriginalPitch);
    if(sf2Gen->_rootKey != -1) {
        sampleOriginFrequency = Reflow::MidiToFrequency(sf2Gen->_rootKey);
    }
    
    freqMod = pitchFreq / sampleOriginFrequency;
    freqMod *= ((float)sample->dwSampleRate / _sampleRate);

    _player->SetFreqModifier(freqMod);
}

void REMonophonicSynthVoice::BrutalStop()
{
    if(_player == NULL) return;
    
    _StartXFade();
    _on = false;
}

void REMonophonicSynthVoice::PlayNote(RESF2Generator* sf2Gen, uint8_t pitch, uint8_t velocity)
{
    if(_player) _StartXFade();
    
    _pitch = pitch;
    _exclusiveClass = sf2Gen->ExclusiveClass();
    
    // Apply coarseTune to pitch we have to play
    int pitch2 = pitch;
    pitch2 += sf2Gen->_coarseTune;
    if(pitch2 > 127) {pitch = 127;}
    if(pitch2 < 0) pitch2 = 0;
    pitch = pitch2;
    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    //REPrintf("[[[ coarseTune = %d]]]\n", sf2Gen->_coarseTune);
#endif
    
    double fineTune = 0.01 * (double)sf2Gen->_fineTune;
    
    double midi = (double)pitch + _pitchWheel + fineTune;
    float pitchFreq = Reflow::MidiToFrequency(midi);
    float freqMod = 1.0;
    float ampMod = (float)velocity / 127.0f;
    
    RESF2Patch* patch = sf2Gen->Patch();
    RESoundFont* soundfont = patch->SoundFont();
    const RESoundFont::SF2Sample* sample = soundfont->Sample(sf2Gen->_sampleID);
    RESF2GeneratorPlayer* sf2Player = new RESF2GeneratorPlayer(_sampleRate);
    _player = sf2Player;
    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    /*REPrintf("> PlayNote (%d) Sample: %s (@%d Hz)\n", (int)pitch, sample->achSampleName, sample->dwSampleRate);
    REPrintf("    originalPitch: %d\n", (int)sample->byOriginalPitch);
    REPrintf("    dwStart: %d\n", (int)sample->dwStart);
    REPrintf("      dwStartLoop: %d\n", (int)sample->dwStartloop);
    REPrintf("      dwEndLoop: %d\n", (int)sample->dwEndloop);
    REPrintf("    dwEnd: %d\n", (int)sample->dwEnd);*/
#endif
    
    float sampleOriginFrequency = Reflow::MidiToFrequency(sample->byOriginalPitch);
    if(sf2Gen->_rootKey != -1) {
        sampleOriginFrequency = Reflow::MidiToFrequency(sf2Gen->_rootKey);
    }
    
    freqMod = pitchFreq / sampleOriginFrequency;
    freqMod *= ((float)sample->dwSampleRate / _sampleRate);
    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    //REPrintf("    >>> Freq Mod: %1.2f\n", freqMod);
#endif
    
    _on = true;
    sf2Player->SetGenerator(sf2Gen);
    _player->SetFreqModifier(freqMod);
    _player->SetAmpModifier(ampMod);
    _player->Start();
}

void REMonophonicSynthVoice::_ProcessWithXFade(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan)
{
    static float fadeInWorkL[REFLOW_XFADE_SIZE];
    static float fadeInWorkR[REFLOW_XFADE_SIZE];
    static float fadeOutWorkL[REFLOW_XFADE_SIZE];
    static float fadeOutWorkR[REFLOW_XFADE_SIZE];
    
    memset(fadeInWorkL, 0, sizeof(float) * REFLOW_XFADE_SIZE);
    memset(fadeInWorkR, 0, sizeof(float) * REFLOW_XFADE_SIZE);
    memset(fadeOutWorkL, 0, sizeof(float) * REFLOW_XFADE_SIZE);
    memset(fadeOutWorkR, 0, sizeof(float) * REFLOW_XFADE_SIZE);
    
    if(_player) {
        _player->Process(nbSamples, fadeInWorkL, fadeInWorkR, volume, pan);
    }
    if(_xfadePlayer) {
        _xfadePlayer->Process(nbSamples, fadeOutWorkL, fadeOutWorkR, volume, pan);
    }
    
    for(int i=0; i<nbSamples; ++i)
    {
        float t = _xfadePos / (float)REFLOW_XFADE_SIZE;
        workBufferL[i] += (1.0 - t) * fadeOutWorkL[i] + (t * fadeInWorkL[i]);
        workBufferR[i] += (1.0 - t) * fadeOutWorkR[i] + (t * fadeInWorkR[i]);
        ++_xfadePos;
    }
    
    //_xfadePos += nbSamples;
}

void REMonophonicSynthVoice::_ProcessWithoutXFade(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan)
{
    if(_player) {
        _player->Process(nbSamples, workBufferL, workBufferR, volume, pan);
    }
}

void REMonophonicSynthVoice::Process(unsigned int nbSamples, float* workBufferL, float* workBufferR, float volume, float pan)
{   
    int xfadeRemaining = REFLOW_XFADE_SIZE - _xfadePos;
    
    if(xfadeRemaining > 0)
    {
        int nbSamplesToProcess = std::min<int>(nbSamples, xfadeRemaining);
        _ProcessWithXFade(nbSamplesToProcess, workBufferL, workBufferR, volume, pan);
        
        int nbRemaining = (nbSamples - nbSamplesToProcess);
        if(nbRemaining) _ProcessWithoutXFade(nbRemaining, workBufferL + nbSamplesToProcess, workBufferR + nbSamplesToProcess, volume, pan);        
    }
    else {
        _ProcessWithoutXFade(nbSamples, workBufferL, workBufferR, volume, pan);
    }
}
