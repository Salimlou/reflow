//
//  RESF2Generator.h
//  Reflow
//
//  Created by Sebastien on 15/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESF2Generator_h
#define Reflow_RESF2Generator_h

#include "RETypes.h"
#include "RESoundFont.h"


class RESF2Generator
{
    friend class RESoundFont;
    friend class RESF2Patch;
    friend class RESF2GeneratorPlayer;
    friend class REMonophonicSynthVoice;
    
public:
    enum GeneratorEnumeratorType {
        DelayVolEnv,
        AttackVolEnv,
        HoldVolEnv,
        DecayVolEnv,
        SustainVolEnv,
        ReleaseVolEnv,
        InitialFilterFreqCut,
        
        GeneratorEnumeratorType_Count
    };
    
    enum SampleLoopType {
        NoLoop = 0,
        LoopForever = 1,
        LoopWhileOn = 2
    };
    
public:
    RESF2Patch* Patch() {return _patch;}
    const RESF2Patch* Patch() const {return _patch;}
    
    void SetLoopType(SampleLoopType loopType) {_loopType = loopType;}
    SampleLoopType LoopType() const {return _loopType;}
    
    bool IsPitchInRange(uint8_t pitch) const;
    bool IsVelocityInRange(uint8_t velocity) const;
    
    int ExclusiveClass() const {return _exclusiveClass;}
    
    void SetLowpassFilterEnabled(bool lowpass) {_lowpassFilterEnabled=lowpass;}
    bool IsLowpassFilterEnabled() const {return _lowpassFilterEnabled;}
    
    void DumpToLog(RELogger* log) const;
    
    inline float VolumeEnvelopeDelay() const {return _delay;}
    inline float VolumeEnvelopeAttack() const {return _attack;}
    inline float VolumeEnvelopeHold() const {return _hold;}
    inline float VolumeEnvelopeDecay() const {return _decay;}
    inline float VolumeEnvelopeSustain() const {return _sustain;}
    inline float VolumeEnvelopeRelease() const {return _release;}
    
private:
    RESF2Generator();
    
    void _InitializeDefaultGenerators();
    void _Finalize();
    void _ApplyValueGenerator(RESF2Generator::GeneratorEnumeratorType type, const RESoundFont::SF2GenList& gen, RESoundFont::SF2GenListLevel level);
    
private:
    uint8_t _keyRangeLow, _keyRangeHigh;
    uint8_t _velRangeLow, _velRangeHigh;
    int8_t _rootKey;                       // -1 means invalid
    int16_t _coarseTune;                    // semitones [-120 .. 120]
    int16_t _fineTune;                      // cents
    float _pan;                             // [0 .. 1] -> [left .. right]
    float _lowpassFC;                       // IIR low pass frequency cut
    RESF2Patch* _patch;
    unsigned long _sampleID;
    SampleLoopType _loopType;
    int8_t _exclusiveClass;
    bool _lowpassFilterEnabled;
    
    // Volume Envelope
    float _delay;           // in Sample Data Space Frames
    float _attack;          // in Sample Data Space Frames
    float _hold;            // in Sample Data Space Frames
    float _decay;           // in Sample Data Space Frames
    float _sustain;         // percent factor from dB (1 means 0dB, 0.5 for -3dB, ...)
    float _release;         // in Sample Data Space Frames
    
    // From the SF2
    RESoundFont::SF2GenList _generatorEnumerators[RESF2Generator::GeneratorEnumeratorType_Count];
    int8_t _generatorFoundInInstrumentZoneFlag;
    int8_t _generatorFoundInGlobalInstrumentZoneFlag;
    int8_t _generatorFoundInPresetZoneFlag;
    int8_t _generatorFoundInGlobalPresetZoneFlag;
    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    std::vector<RESoundFont::SF2InstGenList> _igens;
    std::vector<RESoundFont::SF2InstGenList> _igensGlobal;
    std::vector<RESoundFont::SF2GenList> _pgens;
    std::vector<RESoundFont::SF2GenList> _pgensGlobal;
#endif
    
    static std::string StringValueOfGenerator(RESoundFont::SF2GenList gen);
    
private:
    static std::string _StringValueOfAttenuation(int16_t cB, int16_t min, int16_t max);
    static std::string _StringValueOfSustainEnv(int16_t cB, int16_t min, int16_t max);
    static std::string _StringValueOfEnv(int16_t timecents, int16_t min, int16_t max);
    static std::string _StringValueOfDecayEnv(int16_t timecents, int16_t min, int16_t max);
    static std::string _StringValueOfSampleOffset(int16_t offset, bool coarse);
};


#endif
