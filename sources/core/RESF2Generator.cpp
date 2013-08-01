//
//  RESF2Generator.cpp
//  Reflow
//
//  Created by Sebastien on 15/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESF2Patch.h"
#include "RESoundFont.h"
#include "RESF2Generator.h"
#include "RELogger.h"
#include "REFunctions.h"

#include <cmath>
#include <sstream>
#ifdef _WIN32
#  include <cstdio>
#  define snprintf _snprintf
#endif

RESF2Generator::RESF2Generator()
:   _keyRangeLow(0), _keyRangeHigh(127),
    _velRangeLow(0), _velRangeHigh(127),
    _rootKey(-1),
    _coarseTune(0),
    _fineTune(0),
    _patch(NULL),
    _sampleID(0),
    _pan(0.5),
    _loopType(NoLoop),
    _generatorFoundInInstrumentZoneFlag(0),
    _generatorFoundInGlobalInstrumentZoneFlag(0),
    _generatorFoundInPresetZoneFlag(0),
    _generatorFoundInGlobalPresetZoneFlag(0),
    _exclusiveClass(0),
    _lowpassFC(20000),
    _lowpassFilterEnabled(false)
{
    _InitializeDefaultGenerators();
}

bool RESF2Generator::IsPitchInRange(uint8_t pitch) const
{
    return pitch >= _keyRangeLow && pitch <= _keyRangeHigh;
}

bool RESF2Generator::IsVelocityInRange(uint8_t velocity) const
{
    return velocity >= _velRangeLow && velocity <= _velRangeHigh;
}

RESoundFont::SF2Generator _sfGenOperWithTypeLookup[] =
{
    33, 34, 35, 36, 37, 38, 8
};

void RESF2Generator::_InitializeDefaultGenerators()
{    
    for(int i=0; i<RESF2Generator::GeneratorEnumeratorType_Count; ++i)
    {
        RESoundFont::SF2GenList& gen = _generatorEnumerators[i];
        
        gen.sfGenOper = _sfGenOperWithTypeLookup[i];
        gen.genAmount.shAmount = RESoundFont::InfoOfSF2Gen(gen.sfGenOper).def;
    }
}

void RESF2Generator::_Finalize()
{
    const RESoundFont::SF2Sample* sample = _patch->SoundFont()->Sample(_sampleID);
    
    double sampleRate = (double)sample->dwSampleRate;
    
    int16_t delayTimecents = _generatorEnumerators[RESF2Generator::DelayVolEnv].genAmount.shAmount;
    int16_t attackTimecents = _generatorEnumerators[RESF2Generator::AttackVolEnv].genAmount.shAmount;
    int16_t holdTimecents = _generatorEnumerators[RESF2Generator::HoldVolEnv].genAmount.shAmount;
    int16_t decayTimecents = _generatorEnumerators[RESF2Generator::DecayVolEnv].genAmount.shAmount;
    int16_t sustainCentibels = _generatorEnumerators[RESF2Generator::SustainVolEnv].genAmount.shAmount;
    int16_t releaseTimecents = _generatorEnumerators[RESF2Generator::ReleaseVolEnv].genAmount.shAmount;
    int16_t filterFreqCutCents = _generatorEnumerators[RESF2Generator::InitialFilterFreqCut].genAmount.shAmount;
    
    _delay   = sampleRate * (delayTimecents == -32768 ? 0 : Reflow::AbsoluteTimecentsToSeconds((double)delayTimecents));
    _attack  = sampleRate * (attackTimecents == -32768 ? 0 : Reflow::AbsoluteTimecentsToSeconds((double)attackTimecents));
    _hold    = sampleRate * (holdTimecents == -32768 ? 0 : Reflow::AbsoluteTimecentsToSeconds((double)holdTimecents));
    _decay   = sampleRate * Reflow::AbsoluteTimecentsToSeconds((double)decayTimecents);
    if(sustainCentibels == 0) {
        _sustain = 1.0;
        _decay = 0;
    }
    else {
        _sustain = (sustainCentibels >= 1000 ? 0 : Reflow::DecibelsToPercent(-0.1 * (double)sustainCentibels));
    }
    _release = sampleRate * Reflow::AbsoluteTimecentsToSeconds((double)releaseTimecents);
    
    _lowpassFC = Reflow::MidiToFrequency(0.01 * (double)filterFreqCutCents);
    _lowpassFilterEnabled = (_lowpassFC < 20000);
}

static void _ClampGenerator(RESoundFont::SF2GenList& gen)
{
    const RESoundFont::SF2GeneratorInfo& info = RESoundFont::InfoOfSF2Gen(gen.sfGenOper);
    
    if(gen.sfGenOper >= 33 && gen.sfGenOper <= 35) {       
        if(gen.genAmount.shAmount == -32768) return;        // Conventionnaly means "Instantaneous" for delay, attack and hold phases.
    }
    else if(gen.sfGenOper == 37) {
        if(gen.genAmount.shAmount == 1000) return;          // Conventionnaly means "Full attenuation" for sustain phase.
    }
    
    if(gen.genAmount.shAmount < info.min) gen.genAmount.shAmount = info.min;
    if(gen.genAmount.shAmount > info.max) gen.genAmount.shAmount = info.max;
}

// This function will be called for levels in this order:
// 1. Global Instrument Zone
// 2. Instrument Zone
// 3. Preset Zone
// 4. Global Preset Zone
void RESF2Generator::_ApplyValueGenerator(RESF2Generator::GeneratorEnumeratorType type, const RESoundFont::SF2GenList& gen, RESoundFont::SF2GenListLevel level)
{
    bool presetLevel = (level == RESoundFont::PresetGenList || level == RESoundFont::PresetGlobalGenList);
    
    bool foundInInstrumentZone       = (0 != (_generatorFoundInInstrumentZoneFlag & (1 << type)));
    bool foundInGlobalInstrumentZone = (0 != (_generatorFoundInGlobalInstrumentZoneFlag & (1 << type)));
    bool foundInPresetZone           = (0 != (_generatorFoundInPresetZoneFlag & (1 << type)));
    bool foundInGlobalPresetZone     = (0 != (_generatorFoundInGlobalPresetZoneFlag & (1 << type)));
    bool found = (foundInInstrumentZone || foundInGlobalInstrumentZone || foundInPresetZone || foundInGlobalPresetZone);
    
    if(level == RESoundFont::InstrumentGlobalGenList)
    {
        // SF2.01 Specs, page 65, item 3 //
        //  > A generator in a global instrument zone that is identical to a default generator supersedes or replaces the default generator.
    
        _generatorEnumerators[type] = gen;
        foundInGlobalInstrumentZone |= (1 << type);
    }
    else if(level == RESoundFont::InstrumentGenList)
    {
        // SF2.01 Specs, page 65, item 4 //
        //  > A generator in a local instrument zone that is identical to a default generator or to a generator in a global instrument zone supersedes or replaces that generator.
        
        _generatorEnumerators[type] = gen;
        foundInInstrumentZone |= (1 << type);
    }
    else if(level == RESoundFont::PresetGenList)
    {
        _generatorEnumerators[type].genAmount.shAmount += gen.genAmount.shAmount;
        foundInPresetZone |= (1 << type);
    }
    else if(level == RESoundFont::PresetGlobalGenList)
    {
        if(foundInPresetZone)
        {
            // SF2.01 Specs, page 65, item 9 //
            //  > A generator in a local preset zone that is identical to a generator in a global preset zone supersedes or replaces that generator in the global preset zone. 
            //  > That generator then has its effects added to the destination-summing node of all zones in the given instrument.
            //
            // Implementation Notes: As we have found a preset zone generator first, we can simply ignore this global zone generator to meets SF2 Specs requirements
            //
            
            // NOTHING TO DO HERE
        }
        else 
        {
            // Sum with what we already have
            _generatorEnumerators[type].genAmount.shAmount += gen.genAmount.shAmount;
        }
        foundInGlobalPresetZone |= (1 << type);
    }
    
    // Clamp our generator
    _ClampGenerator(_generatorEnumerators[type]);
}

void RESF2Generator::DumpToLog(RELogger* log) const
{
    const RESoundFont::SF2Sample* sample = _patch->SoundFont()->Sample(_sampleID);
    
    uint32_t sampleFramesCount = (sample->dwEnd - sample->dwStart);
    uint32_t sampleLoopFramesCount = (sample->dwEndloop - sample->dwStartloop);
    
    log->printf("+ Volume Envelope   [  _  ] [  A  ] [  H  ] [  D  ] [  S  ] [  R  ]\n");
    log->printf("+                    %5.0f   %5.0f   %5.0f   %5.0f   %5.3f   %5.0f \n", _delay, _attack, _hold, _decay, _sustain, _release);
    log->printf("+ Sample [%s] (@%d Hz)\n", sample->achSampleName, sample->dwSampleRate);
    log->printf("+   Loop Type: %d\n", _loopType);
    log->printf("+   Duration : %d frames - %1.3f seconds [start: %d - end: %d]\n", sampleFramesCount, (float)sampleFramesCount / (float)sample->dwSampleRate, sample->dwStart, sample->dwEnd);
    log->printf("+   Loop     : %d frames - %1.3f seconds [start: %d - end: %d]\n", sampleLoopFramesCount, (float)sampleLoopFramesCount / (float)sample->dwSampleRate, sample->dwStartloop, sample->dwEndloop);
    log->printf("+ Low Pass Cut Off Frequency: %1.3fHz\n", _lowpassFC);
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
    log->printf("+-----------------------------------+----------------+----------------+----------------+----------------+----------------+\n");
    for(int i=0; i<60; ++i) 
    {
        RESoundFont::SF2InstGenList igen;
        RESoundFont::SF2InstGenList igenGlobal;
        RESoundFont::SF2GenList pgen;
        RESoundFont::SF2GenList pgenGlobal;
        bool igenFound = false;
        bool igenGlobalFound = false;
        bool pgenFound = false;
        bool pgenGlobalFound = false;
        std::string igenStr = "              ";
        std::string igenGlobalStr = "              ";
        std::string pgenStr = "              ";
        std::string pgenGlobalStr = "              ";
        
        for(std::vector<RESoundFont::SF2InstGenList>::const_iterator it = _igens.begin(); it != _igens.end(); ++it) {
            if(it->sfGenOper == i) {
                igen = *it;
                igenFound = true;
                igenStr = StringValueOfGenerator(*reinterpret_cast<RESoundFont::SF2GenList*>(&igen));
                break;
            }
        }
        
        for(std::vector<RESoundFont::SF2InstGenList>::const_iterator it = _igensGlobal.begin(); it != _igensGlobal.end(); ++it) {
            if(it->sfGenOper == i) {
                igenGlobal = *it;
                igenGlobalFound = true;
                igenGlobalStr = StringValueOfGenerator(*reinterpret_cast<RESoundFont::SF2GenList*>(&igenGlobal));
                break;
            }
        }
        
        for(std::vector<RESoundFont::SF2GenList>::const_iterator it = _pgens.begin(); it != _pgens.end(); ++it) {
            if(it->sfGenOper == i) {
                pgen = *it;
                pgenFound = true;
                pgenStr = StringValueOfGenerator(pgen);
                break;
            }
        }
        
        for(std::vector<RESoundFont::SF2GenList>::const_iterator it = _pgensGlobal.begin(); it != _pgensGlobal.end(); ++it) {
            if(it->sfGenOper == i) {
                pgenGlobal = *it;
                pgenGlobalFound = true;
                pgenGlobalStr = StringValueOfGenerator(pgenGlobal);
                break;
            }
        }

        bool found = igenFound || igenGlobalFound || pgenFound || pgenGlobalFound;
        log->printf("|  [%s] %2.2d %-25.25s| %14.14s | %14.14s | %14.14s | %14.14s | []\n", 
                    found ? "OK" : "  ", i, RESoundFont::NameOfSF2Gen(i), igenStr.c_str(), igenGlobalStr.c_str(), pgenStr.c_str(), pgenGlobalStr.c_str());
    }
    log->printf("+-----------------------------------+----------------+----------------+----------------+----------------+----------------+\n");
#endif
}

std::string RESF2Generator::StringValueOfGenerator(RESoundFont::SF2GenList gen)
{
    std::ostringstream oss; oss << gen.genAmount.shAmount;
    std::string defaultStr = oss.str();
    return defaultStr;
    
    switch(gen.sfGenOper)
    {
        case 0  /*"startAddrsOffset"*/              : return _StringValueOfSampleOffset(gen.genAmount.shAmount, false);
        case 1  /*"endAddrsOffset"*/                : return _StringValueOfSampleOffset(gen.genAmount.shAmount, false);
        case 2  /*"startloopAddrsOffset"*/          : return _StringValueOfSampleOffset(gen.genAmount.shAmount, false);
        case 3  /*"endloopAddrsOffset"*/            : return _StringValueOfSampleOffset(gen.genAmount.shAmount, false);
        case 4  /*"startAddrsCoarseOffset"*/        : return _StringValueOfSampleOffset(gen.genAmount.shAmount, true);
        case 5  /*"modLfoToPitch"*/                 : return defaultStr;
        case 6  /*"vibLfoToPitch"*/                 : return defaultStr;
        case 7  /*"modEnvToPitch"*/                 : return defaultStr;
        case 8  /*"initialFilterFc"*/               : return defaultStr;
        case 9  /*"initialFilterQ"*/                : return defaultStr;
        case 10 /*"modLfoToFilterFc"*/              : return defaultStr;
        case 11 /*"modEnvToFilterFc"*/              : return defaultStr;
        case 12 /* "endAddrsCoarseOffset"*/         : return _StringValueOfSampleOffset(gen.genAmount.shAmount, true);
        case 13 /* "modLfoToVolume"*/               : return defaultStr;
        case 14 /* "unused1"*/                      : return defaultStr;
        case 15 /*"chorusEffectsSend"*/             : return defaultStr;
        case 16 /*"reverbEffectsSend"*/             : return defaultStr;
        case 17 /*"pan"*/                           : return defaultStr;
        case 18 /*"unused2"*/                       : return defaultStr;
        case 19 /*"unused3"*/                       : return defaultStr;
        case 20 /*"unused4"*/                       : return defaultStr;
        case 21 /*"delayModLFO"*/                   : return defaultStr;
        case 22 /*"freqModLFO"*/                    : return defaultStr;
        case 23 /*"delayVibLFO"*/                   : return defaultStr;
        case 24 /*"freqVibLFO"*/                    : return defaultStr;
        case 25 /*"delayModEnv"*/                   : return defaultStr;
        case 26 /*"attackModEnv"*/                  : return defaultStr;
        case 27 /*"holdModEnv"*/                    : return defaultStr;
        case 28 /*"decayModEnv"*/                   : return defaultStr;
        case 29 /*"sustainModEnv"*/                 : return defaultStr;
        case 30 /*"releaseModEnv"*/                 : return defaultStr;
        case 31 /*"keynumToModEnvHold",*/           : return defaultStr;
        case 32 /*"keynumToModEnvDecay",*/          : return defaultStr;
        case 33 /*"delayVolEnv",*/                  : return _StringValueOfEnv(gen.genAmount.shAmount, -12000, 5000);
        case 34 /*"attackVolEnv",*/                 : return _StringValueOfEnv(gen.genAmount.shAmount, -12000, 8000);
        case 35 /*"holdVolEnv",*/                   : return _StringValueOfEnv(gen.genAmount.shAmount, -12000, 5000);
        case 36 /*"decayVolEnv",*/                  : return _StringValueOfDecayEnv(gen.genAmount.shAmount, -12000, 8000);
        case 37 /*"sustainVolEnv",*/                : return _StringValueOfSustainEnv(gen.genAmount.shAmount, 0, 1440);
        case 38 /*"releaseVolEnv",*/                : return defaultStr;
        case 39 /*"keynumToVolEnvHold",*/           : return defaultStr;
        case 40 /*"keynumToVolEnvDecay",*/          : return defaultStr;
        case 41 /*"instrument",*/                   : return defaultStr;
        case 42 /*"reserved1",*/                    : return defaultStr;
        case 43 /*"keyRange",*/                     : return defaultStr;
        case 44 /*"velRange",*/                     : return defaultStr;
        case 45 /*"startloopAddrsCoarseOffset",*/   : return _StringValueOfSampleOffset(gen.genAmount.shAmount, true);
        case 46 /*"keynum",*/                       : return defaultStr;
        case 47 /*"velocity",*/                     : return defaultStr;
        case 48 /*"initialAttenuation",*/           : return _StringValueOfAttenuation(gen.genAmount.shAmount, 0, 1440);
        case 49 /*"reserved2",*/                    : return defaultStr;
        case 50 /*"endloopAddrsCoarseOffset",*/     : return _StringValueOfSampleOffset(gen.genAmount.shAmount, true);
        case 51 /*"coarseTune",*/                   : return defaultStr;
        case 52 /*"fineTune",*/                     : return defaultStr;
        case 53 /*"sampleID",*/                     : return defaultStr;
        case 54 /*"sampleModes",*/                  : return defaultStr;
        case 55 /*"reserved3",*/                    : return defaultStr;
        case 56 /*"scaleTuning",*/                  : return defaultStr;
        case 57 /*"exclusiveClass",*/               : return defaultStr;
        case 58 /*"overridingRootKey",*/            : return defaultStr;
        case 59 /*"unused5",*/                      : return defaultStr;
        default                                     : return defaultStr;
    }
}

std::string RESF2Generator::_StringValueOfSampleOffset(int16_t offset, bool coarse)
{
    std::ostringstream oss;
    if(coarse) {
        oss << offset * 32768 << " samples";
    }
    else {
        oss << offset << " samples";
    }
    return oss.str();
}

std::string RESF2Generator::_StringValueOfAttenuation(int16_t cB, int16_t min, int16_t max)
{
    if(cB < min) cB = min;
    if(cB > max) cB = max;
    
    double dB = 0.1 * (double)cB;
    float percent = Reflow::DecibelsToPercent(-dB);
    char s[15];
    snprintf(s, 15, "%1.2fdB (x%1.2f)", dB, percent);
    s[14] = 0;
    return std::string(s);
}

std::string RESF2Generator::_StringValueOfSustainEnv(int16_t cB, int16_t min, int16_t max)
{
    if(cB >= 1000) {
        return "FULL x0.00";
    }
    
    if(cB < min) cB = min;
    if(cB > max) cB = max;
    
    double dB = 0.1 * (double)cB;
    float percent = Reflow::DecibelsToPercent(-dB);
    char s[15];
    snprintf(s, 15, "%1.2fdB (x%1.2f)", dB, percent);
    s[14] = 0;
    return std::string(s);
}

std::string RESF2Generator::_StringValueOfEnv(int16_t timecents, int16_t min, int16_t max)
{
    double val = 0.0;
    if (timecents == -32768) {return "0.000 s";}
    else if (timecents < min) {timecents = min;}
    else if (timecents > max) {timecents = max;}
    
    val = (double) pow(2.0, (double) timecents / 1200.0);
    
    char s[15];
    snprintf(s, 15, "%1.3f s", val);
    s[14] = 0;
    return std::string(s);
}

std::string RESF2Generator::_StringValueOfDecayEnv(int16_t timecents, int16_t min, int16_t max)
{
    double val = 0.0;
    if (timecents < min) {timecents = min;}
    else if (timecents > max) {timecents = max;}
    
    val = (double) pow(2.0, (double) timecents / 1200.0);
    
    char s[15];
    snprintf(s, 15, "%1.3f s", val);
    s[14] = 0;
    return std::string(s);
}

