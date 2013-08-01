//
//  RESoundFont.h
//  Reflow
//
//  Created by Sebastien on 12/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESoundFont_h
#define Reflow_RESoundFont_h

#include "RETypes.h"


class RESoundFont
{
public:
    enum SF2GenListLevel {
        PresetGlobalGenList,
        PresetGenList,
        InstrumentGlobalGenList,
        InstrumentGenList
    };
    
    struct SF2Sample
    {
        char achSampleName[20];
        uint32_t dwStart;
        uint32_t dwEnd;
        uint32_t dwStartloop;
        uint32_t dwEndloop;
        uint32_t dwSampleRate;
        uint8_t  byOriginalPitch;
        char    chPitchCorrection;
        uint16_t wSampleLink;
        uint32_t sfSampleType;
    };
    
    struct SF2PresetHeader
    {
        char achPresetName[20];
        uint16_t wPreset;
        uint16_t wBank;
        uint16_t wPresetBagNdx;
        uint32_t dwLibrary;
        uint32_t dwGenre;
        uint32_t dwMorphology;
    };
    
    struct SF2PresetBag {
        uint16_t wGenNdx;
        uint16_t wModNdx;
    };
    
    struct SF2Inst {
        char achInstName[20];
        uint16_t wInstBagNdx;
    };
    
    typedef uint16_t SF2Generator;
    
    typedef struct {
        uint8_t byLo;
        uint8_t byHi;
    } SF2RangesType;
    
    typedef union {
        SF2RangesType ranges;
        int16_t shAmount;
        uint16_t wAmount;
    } SF2GenAmountType;
    
    struct SF2GenList {
        SF2Generator sfGenOper;
        SF2GenAmountType genAmount;
    };
    
    typedef SF2GenList SF2InstGenList;
    
    struct SF2InstBag {
        uint16_t wInstGenNdx;
        uint16_t wInstModNdx;
    };
    
public:
    struct SF2GeneratorInfo {
        const char* name;
        int16_t min;
        int16_t max;
        int16_t def;
    };
    
public:
    RESoundFont();
    ~RESoundFont();
    
public:
    bool readSF2File (const std::string& filename);
    
    const std::string& Name() const {return _name;}
    
    RESF2Patch* GMPatchForProgram(int program);
    RESF2Patch* GMPatchForDrumkit();
    
    RESF2Patch* Patch(int program, int bank);
    
    int IndexOfPresetHeaderForGMInstrument(int program, int bank) const;
    unsigned int PresetHeaderCount() const {return _sfPresetHeaderCount;}
    SF2PresetHeader* PresetHeader(int idx);
    const SF2PresetHeader* PresetHeader(int idx) const;
    
    const RESoundFont::SF2Inst* InstrumentHeader(int idx) const;
    
    RERange PresetBagRangeOfPresetHeader(int idx) const;
    RERange PresetGeneratorRangeOfPresetBag(int pbagIdx) const;
    RERange PresetModulatorRangeOfPresetBag(int pbagIdx) const;
    RERange InstrumentBagRangeOfInstrument(int idx) const;
    RERange InstrumentGeneratorRangeOfInstrumentBag(int ibagIdx) const;
    RERange InstrumentModulatorRangeOfInstrumentBag(int ibagIdx) const;
    
    //void DumpGenList(SF2Generator sfGenOper, SF2GenAmountType genAmount);
    
    const SF2Sample* Sample(unsigned int sampleId) const;
    const int16_t* SampleData() const {return _sampleData;}
    
    //void SetVerbose(bool verbose) {_verbose=verbose;}
    void SetLogger(RELogger* logger) {_logger = logger;}
    bool Verbose() const {return _logger != NULL;}
    
public:
    static const char* NameOfSF2Gen(int gen);
    static const SF2GeneratorInfo& InfoOfSF2Gen(int gen);
    
public:
    void DumpPreset(unsigned int presetIndex);
    void DumpInstrument(unsigned int instrumentIndex);
    
private:
    void readChunkINFO (REInputStream* file, uint32_t chunkSize);
    void readChunkSDTA (REInputStream* file, uint32_t chunkSize);
    void readChunkPDTA (REInputStream* file, uint32_t chunkSize);
    
    void readChunkIFIL (REInputStream* file, uint32_t chunkSize);
    void readChunkISNG (REInputStream* file, uint32_t chunkSize);
    void readChunkINAM (REInputStream* file, uint32_t chunkSize);
    
    void readChunkSMPL (REInputStream* file, uint32_t chunkSize);
    
    void readChunkPHDR (REInputStream* file, uint32_t chunkSize);
    void readChunkPBAG (REInputStream* file, uint32_t chunkSize);
    void readChunkPMOD (REInputStream* file, uint32_t chunkSize);
    void readChunkPGEN (REInputStream* file, uint32_t chunkSize);
    void readChunkINST (REInputStream* file, uint32_t chunkSize);
    void readChunkIBAG (REInputStream* file, uint32_t chunkSize);
    void readChunkIMOD (REInputStream* file, uint32_t chunkSize);
    void readChunkIGEN (REInputStream* file, uint32_t chunkSize);
    void readChunkSHDR (REInputStream* file, uint32_t chunkSize);
    
    RESF2Patch* LoadPatch(unsigned int presetIndex);
    
private:   
    void CalculateGeneratorsForInstrument(unsigned int instrumentIndex, RESF2GeneratorVector& generators);

    void ApplyGenListToGenerator(RESF2Generator* generator, const SF2GenList& genList, SF2GenListLevel level);
    
private:
    std::string _name;
    
    unsigned int _sfSampleCount;
    unsigned int _sfPresetHeaderCount;
    unsigned int _sfPresetBagCount;
    unsigned int _sfPGenCount;
    unsigned int _sfInstCount;
    unsigned int _sfInstBagCount;
    unsigned int _sfIGenCount;
    
    SF2Sample*       _sfSamples;
    SF2PresetHeader* _sfPresetHeaders;
    SF2PresetBag*    _sfPresetBags;
    SF2GenList*      _sfPGens;
    SF2Inst*         _sfInsts;
    SF2InstBag*      _sfInstBags;
    SF2InstGenList*  _sfIGens;
    
    /*RESF2Patch* _gmPatches[128];
    RESF2Patch* _drumPatch;*/
    std::vector<RESF2Patch*> _patches;
    
    unsigned int _sampleCount;
    int16_t*    _sampleData;
    RELogger*   _logger;
};



#endif
