//
//  RESoundFont.cpp
//  Reflow
//
//  Created by Sebastien on 12/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESoundFont.h"
#include "REInputStream.h"
#include "REFunctions.h"
#include "RESF2Patch.h"
#include "RESF2Generator.h"
#include "RELogger.h"

#include <cmath>

#define DUMP_SF2_LOADING

static float timecentsToSec(float tc) {
    return powf(2.0, tc/1200.0);
}

RESoundFont::RESoundFont()
: _sfInsts(NULL), _sfPresetHeaders(NULL), _sfSamples(NULL),
_sfInstCount(0), _sfPresetHeaderCount(0), _sfSampleCount(0),
_sampleData(NULL), _sampleCount(0), _logger(NULL)
{
    /*_drumPatch = NULL;
    for(int i=0; i<128; ++i) {
        _gmPatches[i] = NULL;
    }*/
}

RESoundFont::~RESoundFont()
{
    delete _sampleData;
    delete _sfPresetHeaders;
    delete _sfSamples;
    delete _sfInsts;
    
    /*delete _drumPatch;
    for(int i=0; i<128; ++i) {
        delete _gmPatches[i];
    }*/
}

bool RESoundFont::readSF2File(const std::string& filename)
{
    REFileInputStream file;
    if(!file.Open(filename)) {
        return false;
    }
        
    char headerMagic[4];
    file.Read(headerMagic, 4);
    uint32_t headerSize = file.ReadUInt32();
    
    file.Read(headerMagic, 4);
    //headerSize = file.ReadUInt32();
    if(strncmp(headerMagic, "sfbk", 4)) {
        std::cout << "SoundFont is corrupted" << std::endl;
        return false;
    }
    //std::cout << "SFBK" << std::endl;
    
    // LIST <INFO-list> chunk
    char fourcc[4];
    file.Read(fourcc, 4);
    uint32_t chunkSize = file.ReadUInt32();
    assert(!strncmp(fourcc, "LIST", 4));
    
    // INFO chunk
    {
        std::string infoBytes = file.ReadBytes(chunkSize);
        REBufferInputStream infoChunk (infoBytes);
        readChunkINFO(&infoChunk, chunkSize);
    }
    
    // LIST <sdta> chunk
    file.Read(fourcc, 4);
    chunkSize = file.ReadUInt32();
    readChunkSDTA (&file, chunkSize);
    
    // LIST <pdta> chunk
    file.Read(fourcc, 4);
    chunkSize = file.ReadUInt32();
    readChunkPDTA (&file, chunkSize);
    
    file.Close();
    
    // Load General Midi Patches
    /*for(unsigned int i = 0; i < 128; ++i) 
    {
        int idx = IndexOfPresetHeaderForGMInstrument(i, 0);
        RESF2Patch* patch = LoadPatch(idx);
        _gmPatches[i] = patch;
    }
    
    // Load General Midi Drum Patch
    _drumPatch = LoadPatch(IndexOfPresetHeaderForGMInstrument(0, 128));*/
    
    for(int presetIndex = 0; presetIndex < _sfPresetHeaderCount; ++presetIndex) {
        _patches.push_back(LoadPatch(presetIndex));
    }
    
    return true;
}

RESF2Patch* RESoundFont::Patch(int program, int bank)
{
    int idx = IndexOfPresetHeaderForGMInstrument(program, bank);
    if(idx >= 0 && idx < _patches.size()) {
        return _patches[idx];
    }
    return NULL;
}

void RESoundFont::ApplyGenListToGenerator(RESF2Generator* generator, const SF2GenList& genList, SF2GenListLevel level)
{
    SF2Generator sfGenOper = genList.sfGenOper;
    SF2GenAmountType genAmount = genList.genAmount;
    
    switch(sfGenOper)
    {
        case 0: {
            if(Verbose()) _logger->printf("unused startOffset: %d\n", (int)genAmount.shAmount);
            break;
        }
        case 1: {
            if(Verbose()) _logger->printf("unused startLoopOffset: %d\n", (int)genAmount.shAmount);
            break;
        }
        case 2: {
            if(Verbose()) _logger->printf("unused endLoopOffset: %d\n",(int) genAmount.shAmount);
            break;
        }
        case 3: {
            if(Verbose()) _logger->printf("unused endOffset: %d\n", (int)genAmount.shAmount);
            break;
        }
        case 4: {
            if(Verbose()) _logger->printf("unused startCoarseOffset: %d\n", (int)genAmount.shAmount);
            break;
        }
        case 8: {
            generator->_ApplyValueGenerator(RESF2Generator::InitialFilterFreqCut, genList, level); break;
        }
        case 17: {
            generator->_pan = 0.5 + (float)genAmount.shAmount / 1000.0f;
            if(Verbose()) _logger->printf("pan %1.2f\n", generator->_pan);
            break;
        }
            
        case 33:        generator->_ApplyValueGenerator(RESF2Generator::DelayVolEnv, genList, level); break;
        case 34:        generator->_ApplyValueGenerator(RESF2Generator::AttackVolEnv, genList, level); break;
        case 35:        generator->_ApplyValueGenerator(RESF2Generator::HoldVolEnv, genList, level); break;
        case 36:        generator->_ApplyValueGenerator(RESF2Generator::DecayVolEnv, genList, level); break;
        case 37:        generator->_ApplyValueGenerator(RESF2Generator::SustainVolEnv, genList, level); break;
        case 38:        generator->_ApplyValueGenerator(RESF2Generator::ReleaseVolEnv, genList, level); break;
            
            
        case 41: {
            int idx = genAmount.wAmount;
            if(Verbose()) _logger->printf("instrument %d (-> %s)\n", idx, _sfInsts[idx].achInstName);
            break;
        }
        case 43: {
            if(Verbose()) _logger->printf("keyRange [%d..%d]\n", genAmount.ranges.byLo, genAmount.ranges.byHi);
            if(genAmount.ranges.byLo > generator->_keyRangeLow) {generator->_keyRangeLow = genAmount.ranges.byLo;}
            if(genAmount.ranges.byHi < generator->_keyRangeHigh) {generator->_keyRangeHigh = genAmount.ranges.byHi;}
            break;
        }
        case 44: {
            if(Verbose()) _logger->printf("velRange [%d..%d]\n", genAmount.ranges.byLo, genAmount.ranges.byHi);
            if(genAmount.ranges.byLo > generator->_velRangeLow) {generator->_velRangeLow = genAmount.ranges.byLo;}
            if(genAmount.ranges.byHi < generator->_velRangeHigh) {generator->_velRangeHigh = genAmount.ranges.byHi;}
            break;
        }
        case 45: {
            if(Verbose()) _logger->printf("unused startloopAddrsCoarseOffset (%d)\n", (int)genAmount.shAmount);
            break;
        }
        case 48: {
            float attenuation = 0.1 * (float)genAmount.shAmount;    // attenuation in dB
            float factor = Reflow::DecibelsToPercent(attenuation);
            if(Verbose()) _logger->printf("initialAttenuation %1.2fdB (factor %1.3f)\n", attenuation, factor);
            break;
        }
        case 50: {
            if(Verbose()) _logger->printf("unused endloopAddrsCoarseOffset (%d)\n", (int)genAmount.shAmount);
            break;
        }
        case 51: {
            generator->_coarseTune += genAmount.shAmount;
            if(Verbose()) _logger->printf("coarseTune (%d semitones)\n", (int)genAmount.shAmount);
            break;
        }
        case 52: {
            generator->_fineTune += genAmount.shAmount;
            if(Verbose()) _logger->printf("fineTune (%d cents)\n", (int)genAmount.shAmount);
            break;
        }
        case 53: {
            generator->_sampleID = genAmount.wAmount;
            if(Verbose()) _logger->printf("sampleID %lu (-> %s with data from %d to %d)\n", 
                                generator->_sampleID, _sfSamples[generator->_sampleID].achSampleName, _sfSamples[generator->_sampleID].dwStart, _sfSamples[generator->_sampleID].dwEnd);
            break;
        }
        case 54: {
            if(Verbose()) _logger->printf("sampleModes: %d\n", genAmount.wAmount);
            if(genAmount.wAmount == 0) {
                generator->_loopType = RESF2Generator::NoLoop;
            }
            else if(genAmount.wAmount == 1) {
                generator->_loopType = RESF2Generator::LoopForever;
            }
            else if(genAmount.wAmount == 2) {
                generator->_loopType = RESF2Generator::LoopWhileOn;
            }
            break;
        }
        case 57: {
            if(Verbose()) _logger->printf("exclusiveClass %d\n", genAmount.shAmount);
            if(genAmount.shAmount != 0) generator->_exclusiveClass = genAmount.shAmount;
            break;
        }
        case 58: {
            if(Verbose()) _logger->printf("overridingRootKey %d\n", genAmount.shAmount);
            generator->_rootKey = genAmount.shAmount;
            break;
        }
        default: {
            if(Verbose()) _logger->printf("unknown generator %d\n", sfGenOper);
        }
    }
}


void RESoundFont::CalculateGeneratorsForInstrument(unsigned int instrumentIndex, RESF2GeneratorVector& generators)
{
    SF2Inst* inst = &_sfInsts[instrumentIndex];
    RERange rg = InstrumentBagRangeOfInstrument(instrumentIndex);
    if(Verbose()) _logger->printf("    ~~[Instru %d - %s] Bags in range [%d..%d]~~\n", instrumentIndex, inst->achInstName, rg.FirstIndex(), rg.LastIndex());
    
    SF2InstBag* ibagGlobal = NULL;
    RERange ibagGlobalRange;
    
    for(unsigned int ibagIdx = rg.FirstIndex(); ibagIdx <= rg.LastIndex(); ++ibagIdx)
    {
        SF2InstBag* ibag = &_sfInstBags[ibagIdx];
        RERange igenRg = InstrumentGeneratorRangeOfInstrumentBag(ibagIdx);
        if(Verbose()) _logger->printf("      [IBag %d] Generators in range [%d..%d]\n", ibagIdx, igenRg.FirstIndex(), igenRg.LastIndex());
        
        // Are we in global zone? 
        SF2InstGenList* lastIGen = &_sfIGens[igenRg.LastIndex()];
        if(ibagIdx == rg.FirstIndex() && lastIGen->sfGenOper != 53) // SampleID
        {
            ibagGlobal = ibag;
            ibagGlobalRange = igenRg;
        }
        else 
        {   
            // Create Generator
            RESF2Generator* generator = new RESF2Generator;
            
            // Apply Gen List of Instrument Global Zone
            if(ibagGlobal)
            {
                for(unsigned int igenIdx = ibagGlobalRange.FirstIndex(); igenIdx <= ibagGlobalRange.LastIndex(); ++igenIdx)
                {
                    SF2InstGenList* igen = &_sfIGens[igenIdx];
                    if(Verbose()) _logger->printf("        [GLOBAL IGEN - Oper: %d] ", igen->sfGenOper);
                    ApplyGenListToGenerator(generator, *igen, RESoundFont::InstrumentGlobalGenList);
                    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
                    generator->_igensGlobal.push_back(*igen);
#endif
                }
            }
            
            // Apply Instrument Gen List
            for(unsigned int igenIdx = igenRg.FirstIndex(); igenIdx <= igenRg.LastIndex(); ++igenIdx)
            {
                SF2InstGenList* igen = &_sfIGens[igenIdx];
                if(Verbose()) _logger->printf("        [IGEN - Oper: %d] ", igen->sfGenOper);
                ApplyGenListToGenerator(generator, *igen, RESoundFont::InstrumentGenList);
                
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
                generator->_igens.push_back(*igen);
#endif
            }
            
            // Push generator
            if(generator->_sampleID != 0) {
                generators.push_back(generator);
            }
            else {
                delete generator;
            }
        }
    }
    if(Verbose()) _logger->printf("    ~~[/Instru]\n");
}

RESF2Patch* RESoundFont::LoadPatch(unsigned int presetIndex)
{
    SF2PresetHeader* pheader = &_sfPresetHeaders[presetIndex];
    if(Verbose()) _logger->printf("~~~~ Loading Patch %s (preset %d) ~~~~\n", pheader->achPresetName, presetIndex);
    
    RESF2Patch* patch = new RESF2Patch(this);
    patch->SetName(pheader->achPresetName);
    
    RERange rg = PresetBagRangeOfPresetHeader(presetIndex);
    if(Verbose()) _logger->printf("[Preset %d - %s] Bags in range [%d..%d]\n", presetIndex, pheader->achPresetName, rg.FirstIndex(), rg.LastIndex());
    
    SF2PresetBag* pbagGlobal = NULL;
    RERange pbagGlobalRange;
    
    // Load Generators
    for(unsigned int pbagIdx = rg.FirstIndex(); pbagIdx <= rg.LastIndex(); ++pbagIdx)
    {
        SF2PresetBag* pbag = &_sfPresetBags[pbagIdx];
        RERange pgenRg = PresetGeneratorRangeOfPresetBag(pbagIdx);
        if(Verbose()) _logger->printf("  [Bag %d] Generators in range [%d..%d]\n", pbagIdx, pgenRg.FirstIndex(), pgenRg.LastIndex());
        
        // Are we in global zone
        SF2GenList* lastPGen = &_sfPGens[pgenRg.LastIndex()];
        if(pbagIdx == rg.FirstIndex() && lastPGen->sfGenOper != 41)
        {
            // Global Zone
            pbagGlobal = pbag;
            pbagGlobalRange = pgenRg;
        }
        else
        {
            // Retrieve Instrument
            unsigned int instrumentIndex = lastPGen->genAmount.wAmount;
            RESF2GeneratorVector generators;
            CalculateGeneratorsForInstrument(instrumentIndex, generators);
            
            // Apply Preset Operators (except the last one as it is the Instrument Generator
            for(unsigned int pgenIdx = pgenRg.FirstIndex(); pgenIdx <= pgenRg.LastIndex()-1; ++pgenIdx)
            {
                SF2GenList* pgen = &_sfPGens[pgenIdx];
                for(RESF2Generator* gen : generators)
                {
                    if(Verbose()) _logger->printf("    [PGEN - Oper: %d] ", pgen->sfGenOper);
                    ApplyGenListToGenerator(gen, *pgen, RESoundFont::PresetGenList);
                    
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
                    gen->_pgens.push_back(*pgen);
#endif
                }
            }   
            
            // Apply Global Zone Preset Operators if found
            if(pbagGlobal)
            {
                for(unsigned int pgenIdx = pbagGlobalRange.FirstIndex(); pgenIdx <= pbagGlobalRange.LastIndex(); ++pgenIdx)
                {
                    SF2GenList* pgen = &_sfPGens[pgenIdx];
                    for(RESF2Generator* gen : generators)
                    {
                        if(Verbose()) _logger->printf("    [GLOBAL PGEN - Oper: %d] ", pgen->sfGenOper);
                        ApplyGenListToGenerator(gen, *pgen, RESoundFont::PresetGlobalGenList);
                        
#ifdef REFLOW_TRACE_SOUNDFONT_GENERATORS
                        gen->_pgensGlobal.push_back(*pgen);
#endif
                    }
                }
            }
            
            // Finalize & Append Generators
            for(unsigned int i=0; i<generators.size(); ++i)
            {
                RESF2Generator* generator = generators[i];
                patch->AddGenerator(generator);
            }
        }
    }
    
    if(Verbose()) _logger->printf("~~~~ Patch Loading OK ~~~~\n");
    return patch;
}

int RESoundFont::IndexOfPresetHeaderForGMInstrument(int program, int bank) const
{
    for(int i=0; i<_sfPresetHeaderCount; ++i) {
        const SF2PresetHeader* ph = &_sfPresetHeaders[i];
        if(ph->wPreset == program && ph->wBank == bank) {
            return i;
        }
    }
    return -1;
}
RESoundFont::SF2PresetHeader* RESoundFont::PresetHeader(int idx)
{
    if(idx >= 0 && idx < _sfPresetHeaderCount) {
        return &_sfPresetHeaders[idx];
    }
    return NULL;
}

const RESoundFont::SF2PresetHeader* RESoundFont::PresetHeader(int idx) const
{
    if(idx >= 0 && idx < _sfPresetHeaderCount) {
        return &_sfPresetHeaders[idx];
    }
    return NULL;
}

const RESoundFont::SF2Inst* RESoundFont::InstrumentHeader(int idx) const
{
    if(idx >= 0 && idx < _sfInstCount) {
        return &_sfInsts[idx];
    }
    return NULL;
}

RERange RESoundFont::PresetBagRangeOfPresetHeader(int idx) const
{
    int pbagIdx = _sfPresetHeaders[idx].wPresetBagNdx;
    int pbagNextIdx = (idx+1 < _sfPresetHeaderCount ? _sfPresetHeaders[idx+1].wPresetBagNdx : _sfPresetBagCount);
    
    return RERange(pbagIdx, pbagNextIdx-pbagIdx);
}

RERange RESoundFont::InstrumentBagRangeOfInstrument(int idx) const
{
    int ibagIdx = _sfInsts[idx].wInstBagNdx;
    int ibagNextIdx = (idx+1 < _sfInstCount ? _sfInsts[idx+1].wInstBagNdx : _sfInstCount);
    
    return RERange(ibagIdx, ibagNextIdx-ibagIdx);
}

RERange RESoundFont::PresetGeneratorRangeOfPresetBag(int pbagIdx) const
{
    int pgenIdx = _sfPresetBags[pbagIdx].wGenNdx;
    int pgenNextIdx = (pbagIdx+1 < _sfPresetBagCount ? _sfPresetBags[pbagIdx+1].wGenNdx : _sfPGenCount);
    
    return RERange(pgenIdx, pgenNextIdx-pgenIdx);
}

RERange RESoundFont::PresetModulatorRangeOfPresetBag(int pbagIdx) const
{
    int pmodIdx = _sfPresetBags[pbagIdx].wModNdx;
    int pmodNextIdx = (pbagIdx+1 < _sfPresetBagCount ? _sfPresetBags[pbagIdx+1].wModNdx : _sfPGenCount);
    
    return RERange(pmodIdx, pmodNextIdx-pmodIdx);
}

RERange RESoundFont::InstrumentGeneratorRangeOfInstrumentBag(int ibagIdx) const
{
    int igenIdx = _sfInstBags[ibagIdx].wInstGenNdx;
    int igenNextIdx = (ibagIdx+1 < _sfInstBagCount ? _sfInstBags[ibagIdx+1].wInstGenNdx : _sfIGenCount);
    
    return RERange(igenIdx, igenNextIdx-igenIdx);
}

RERange RESoundFont::InstrumentModulatorRangeOfInstrumentBag(int ibagIdx) const
{
    int imodIdx = _sfInstBags[ibagIdx].wInstModNdx;
    int imodNextIdx = (ibagIdx+1 < _sfInstBagCount ? _sfInstBags[ibagIdx+1].wInstModNdx : _sfIGenCount);
    
    return RERange(imodIdx, imodNextIdx-imodIdx);
}

const RESoundFont::SF2Sample* RESoundFont::Sample(unsigned int sampleId) const
{
    if(sampleId < _sfSampleCount) {
        return &_sfSamples[sampleId];
    }
    return NULL;
}

void RESoundFont::readChunkINFO (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "    INFO (" << chunkSize << "bytes)" << std::endl;
    
    char fourcc[4];
    file->Read(fourcc, 4);
    assert(!strncmp(fourcc, "INFO", 4));
    
    // Sub chunks
    while(!file->AtEnd())
    {
        char fourcc[4];
        file->Read(fourcc, 4);
        uint32_t chunkSize = file->ReadUInt32();
        
        if(!strncmp(fourcc, "ifil", 4))
        {
            // ifil chunk
            readChunkIFIL (file, chunkSize);
        }
        else if (!strncmp(fourcc, "isng", 4))
        {
            // isng chunk
            readChunkISNG (file, chunkSize);
        }
        else if (!strncmp(fourcc, "INAM", 4))
        {
            // inam chunk
            readChunkINAM (file, chunkSize);
        }
        /*else if (!strncmp(fourcc, "irom", 4))
         {
         // irom chunk
         readChunkIROM (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "iver", 4))
         {
         // iver chunk
         readChunkIVER (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "ICRD", 4))
         {
         // ICRD chunk
         readChunkICRD (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "IENG", 4))
         {
         // IENG chunk
         readChunkIENG (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "IPRD", 4))
         {
         // IPRD chunk
         readChunkIPRD (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "ICOP", 4))
         {
         // ICOP chunk
         readChunkICOP (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "ICMT", 4))
         {
         // ICMT chunk
         readChunkICMT (&file, chunkSize);
         }
         else if (!strncmp(fourcc, "ISFT", 4))
         {
         // ISFT chunk
         readChunkISFT (&file, chunkSize);
         }*/
        else {
            file->SeekTo (file->Pos() + chunkSize);
        }
    }
}

void RESoundFont::readChunkSDTA (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "    SDTA (" << chunkSize << "bytes)" << std::endl;
    
    char fourcc[4];
    file->Read(fourcc, 4);
    assert(!strncmp(fourcc, "sdta", 4));
    
    // Sub chunks
    while(!file->AtEnd())
    {
        char fourcc[4];
        file->Read(fourcc, 4);
        uint32_t chunkSize = file->ReadUInt32();
        
        if(!strncmp(fourcc, "LIST", 4)) {
            file->SeekTo(file->Pos() - 8);
            return;
        }
        else if(!strncmp(fourcc, "smpl", 4))
        {
            // smpl chunk
            readChunkSMPL (file, chunkSize);
        }
        /*else if (!strncmp(fourcc, "sm24", 4))
         {
         // sm24 chunk
         readChunkSM24 (&file, chunkSize);
         }*/
        else {
            file->SeekTo (file->Pos() + chunkSize);
        }
    }
}

void RESoundFont::readChunkPDTA (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "    PDTA (" << chunkSize << "bytes)" << std::endl;
    
    char fourcc[4];
    file->Read(fourcc, 4);
    assert(!strncmp(fourcc, "pdta", 4));
    
    // Sub chunks
    while(!file->AtEnd())
    {
        char fourcc[4];
        file->Read(fourcc, 4);
        uint32_t chunkSize = file->ReadUInt32();
        
        if(!strncmp(fourcc, "phdr", 4))
        {
            // phdr chunk
            readChunkPHDR (file, chunkSize);
        }
        else if (!strncmp(fourcc, "pbag", 4))
        {
            // pbag chunk
            readChunkPBAG (file, chunkSize);
        }
        else if (!strncmp(fourcc, "pmod", 4))
        {
            // pmod chunk
            readChunkPMOD (file, chunkSize);
        }
        else if (!strncmp(fourcc, "pgen", 4))
        {
            // pgen chunk
            readChunkPGEN (file, chunkSize);
        }
        else if (!strncmp(fourcc, "inst", 4))
        {
            // inst chunk
            readChunkINST (file, chunkSize);
        }
        else if (!strncmp(fourcc, "ibag", 4))
        {
            // ibag chunk
            readChunkIBAG (file, chunkSize);
        }
        else if (!strncmp(fourcc, "imod", 4))
        {
            // imod chunk
            readChunkIMOD (file, chunkSize);
        }
        else if (!strncmp(fourcc, "igen", 4))
        {
            // igen chunk
            readChunkIGEN (file, chunkSize);
        }
        else if (!strncmp(fourcc, "shdr", 4))
        {
            // shdr chunk
            readChunkSHDR (file, chunkSize);
        }
        else {
            file->SeekTo (file->Pos() + chunkSize);
        }
    }
}

void RESoundFont::readChunkIFIL (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      IFIL (" << chunkSize << "bytes)" << std::endl;
    file->SeekTo (file->Pos() + chunkSize);
}

void RESoundFont::readChunkISNG (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      ISNG (" << chunkSize << "bytes)" << std::endl;
    file->SeekTo (file->Pos() + chunkSize);
}

void RESoundFont::readChunkINAM (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      INAM (" << chunkSize << "bytes)" << std::endl;
    
    _name = file->ReadBytes(chunkSize);
}

void RESoundFont::readChunkSMPL (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      SMPL (" << chunkSize << "bytes)" << std::endl;
    
    _sampleCount = chunkSize/2;
    _sampleData  = new int16_t[_sampleCount];
    
    /*for(int i=0; i<_sampleCount; ++i)
    {
        int16_t s;
        file->Read((char*)&s, 2);
        _sampleData[i] = float(s) / 32766.0f ;
    }*/
    
    file->Read((char*)_sampleData, 2*_sampleCount);
}

void RESoundFont::readChunkPHDR (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      PHDR (" << chunkSize << "bytes)" << std::endl;
    
    int nbPresets = chunkSize / 38;
    _sfPresetHeaderCount = nbPresets - 1;
    _sfPresetHeaders = new SF2PresetHeader[_sfPresetHeaderCount];
    
    for(int i=0; i<_sfPresetHeaderCount; ++i)
    {
        SF2PresetHeader* ph = &_sfPresetHeaders[i];
        file->Read((char*)ph, 38);
        ////std::cout << "          " << ph->wPreset << " - " << ph->wBank << " - pbag:" << ph->wPresetBagNdx << " " << ph->achPresetName << std::endl;
    }
    
    file->SeekTo(file->Pos() + 38);
}

void RESoundFont::readChunkPBAG (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      PBAG (" << chunkSize << "bytes)" << std::endl;
    
    int nbBags = chunkSize / 4;
    _sfPresetBagCount = nbBags - 1;
    _sfPresetBags = new SF2PresetBag [_sfPresetBagCount];
    
    for(int i=0; i<_sfPresetBagCount; ++i)
    {
        SF2PresetBag* ph = &_sfPresetBags[i];
        file->Read((char*)ph, 4);
        
        ////std::cout << "          Preset Bag #"<< i << "- PGEN:" << ph->wGenNdx << " PMOD:" << ph->wModNdx << std::endl;
    }
    
    file->SeekTo (file->Pos() + 4);
}

void RESoundFont::readChunkPMOD (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      PMOD (" << chunkSize << "bytes)" << std::endl;
    file->SeekTo (file->Pos() + chunkSize);
}

void RESoundFont::readChunkPGEN (REInputStream* file, uint32_t chunkSize)
{
    ////std::cout << "      PGEN (" << chunkSize << "bytes)" << std::endl;
    
    int nbGens = chunkSize / 4;
    _sfPGenCount = nbGens - 1;
    _sfPGens = new SF2GenList [_sfPGenCount];
    
    for(int i=0; i<_sfPGenCount; ++i)
    {
        SF2GenList* ph = &_sfPGens[i];
        file->Read((char*)ph, 4);
        
        ////std::cout << "          PGEN #" << i << "- Oper:" << ph->sfGenOper << std::endl;
    }
    
    file->SeekTo(file->Pos() + 4);
}

void RESoundFont::readChunkINST (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      INST (" << chunkSize << "bytes)" << std::endl;
    
    int nbInstruments = chunkSize / 22;
    _sfInstCount = nbInstruments - 1;
    _sfInsts = new SF2Inst[_sfInstCount];
    
    for(int i=0; i<_sfInstCount; ++i)
    {
        SF2Inst *inst = &_sfInsts[i];
        file->Read((char*)inst, 22);
        ////std::cout << "          INST #" << i << ":" << inst->achInstName << " (Bag:" << inst->wInstBagNdx << ")" << std::endl;
    }
    
    file->SeekTo(file->Pos() + 22);
}

void RESoundFont::readChunkIBAG (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      IBAG (" << chunkSize << "bytes)" << std::endl;
    
    int nbBags = chunkSize / 4;
    _sfInstBagCount = nbBags - 1;
    _sfInstBags = new SF2InstBag [_sfInstBagCount];
    
    for(int i=0; i<_sfInstBagCount; ++i)
    {
        SF2InstBag* ph = &_sfInstBags[i];
        file->Read((char*)ph, 4);
        
        ////std::cout << "          IBAG #"<< i << "- IGEN:" << ph->wInstGenNdx << " IMOD:" << ph->wInstModNdx << std::endl;
    }
    
    file->SeekTo(file->Pos() + 4);
}

void RESoundFont::readChunkIMOD (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      IMOD (" << chunkSize << "bytes)" << std::endl;
    file->SeekTo (file->Pos() + chunkSize);
}

void RESoundFont::readChunkIGEN (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      IGEN (" << chunkSize << "bytes)" << std::endl;
    
    int nbGens = chunkSize / 4;
    _sfIGenCount = nbGens - 1;
    _sfIGens = new SF2InstGenList [_sfIGenCount];
    
    for(int i=0; i<_sfIGenCount; ++i)
    {
        SF2InstGenList* ph = &_sfIGens[i];
        file->Read((char*)ph, 4);
        
        ////std::cout << "          IGEN #" << i << "- Oper:" << ph->sfGenOper << std::endl;
    }
    
    file->SeekTo(file->Pos() + 4);
}

void RESoundFont::readChunkSHDR (REInputStream* file, uint32_t chunkSize)
{
    //std::cout << "      SHDR (" << chunkSize << "bytes)" << std::endl;
    
    int nbSamples = chunkSize / 46;
    
    _sfSampleCount = nbSamples-1;
    _sfSamples = new SF2Sample[_sfSampleCount];
    
    for(int i=0; i<_sfSampleCount; ++i)
    {
        SF2Sample *sample = &_sfSamples[i];
        file->Read((char*)sample, 46);
        ////std::cout << "          " << sample->achSampleName << "(pitch:" << (int)sample->byOriginalPitch << " rate:" << sample->dwSampleRate << " start: " << sample->dwStart << " [loop from " << sample->dwStartloop << " to " << sample->dwEndloop << "] end: " << sample->dwEnd << ")" << std::endl;
    }
    
    file->SeekTo(file->Pos() + 46);
}


RESF2Patch* RESoundFont::GMPatchForProgram(int program)
{
    int index = IndexOfPresetHeaderForGMInstrument(program, 0);
    if(index >= 0 && index < _patches.size()) {
        return _patches[index];
    }
    return NULL;
}

RESF2Patch* RESoundFont::GMPatchForDrumkit()
{
    int index = IndexOfPresetHeaderForGMInstrument(0, 128);
    if(index >= 0 && index < _patches.size()) {
        return _patches[index];
    }
    return NULL;
}

void RESoundFont::DumpPreset(unsigned int presetIndex)
{
    if(!Verbose()) return;
    
    SF2PresetHeader* pheader = &_sfPresetHeaders[presetIndex];
    RERange rg = PresetBagRangeOfPresetHeader(presetIndex);
    _logger->printf("[Preset %d - %s] Bags in range [%d..%d]\n", presetIndex, pheader->achPresetName, rg.FirstIndex(), rg.LastIndex());
    
    for(unsigned int pbagIdx = rg.FirstIndex(); pbagIdx <= rg.LastIndex(); ++pbagIdx)
    {
        SF2PresetBag* pbag = &_sfPresetBags[pbagIdx];
        
        // Modulators
        RERange pmodRg = PresetModulatorRangeOfPresetBag(pbagIdx);
        _logger->printf("  [Bag %d] Modulators in range [%d..%d]\n", pbagIdx, pmodRg.FirstIndex(), pmodRg.LastIndex());
        
        // Generators
        RERange pgenRg = PresetGeneratorRangeOfPresetBag(pbagIdx);
        _logger->printf("  [Bag %d] Generators in range [%d..%d]\n", pbagIdx, pgenRg.FirstIndex(), pgenRg.LastIndex());
        
        
        
        /*for(unsigned int pgenIdx = pgenRg.FirstIndex(); pgenIdx <= pgenRg.LastIndex(); ++pgenIdx)
         {
         SF2GenList* pgen = &_sfPGens[pgenIdx];
         _logger->printf("    [PGEN - Oper: %d] ", pgen->sfGenOper);
         //DumpGenList(pgen->sfGenOper, pgen->genAmount);
         }*/
    }
}


void RESoundFont::DumpInstrument(unsigned int instrumentIndex)
{
    if(!Verbose()) return;
    
    SF2Inst* inst = &_sfInsts[instrumentIndex];
    RERange rg = InstrumentBagRangeOfInstrument(instrumentIndex);
    _logger->printf("[Instru %d - %s] Bags in range [%d..%d]\n", instrumentIndex, inst->achInstName, rg.FirstIndex(), rg.LastIndex());
    
    for(unsigned int ibagIdx = rg.FirstIndex(); ibagIdx <= rg.LastIndex(); ++ibagIdx)
    {
        SF2InstBag* ibag = &_sfInstBags[ibagIdx];

        // Modulators
        RERange imodRg = InstrumentModulatorRangeOfInstrumentBag(ibagIdx);
        _logger->printf("  [IBag %d] Modulators in range [%d..%d]\n", ibagIdx, imodRg.FirstIndex(), imodRg.LastIndex());
        
        // Generators
        RERange igenRg = InstrumentGeneratorRangeOfInstrumentBag(ibagIdx);
        _logger->printf("  [IBag %d] Generators in range [%d..%d]\n", ibagIdx, igenRg.FirstIndex(), igenRg.LastIndex());
        
        /*for(unsigned int igenIdx = igenRg.FirstIndex(); igenIdx <= igenRg.LastIndex(); ++igenIdx)
        {
            SF2InstGenList* igen = &_sfIGens[igenIdx];
            printf("    [IGEN - Oper: %d] ", igen->sfGenOper);
            DumpGenList(igen->sfGenOper, igen->genAmount);
        }*/
    }
}


static RESoundFont::SF2GeneratorInfo _SF2GeneratorInfoArray[] = {
    { /* 0*/  "startAddrsOffset", 0, 0, 0 },
    { /* 1*/  "endAddrsOffset", 0, 0, 0 },
    { /* 2*/  "startloopAddrsOffset", 0, 0, 0 },
    { /* 3*/  "endloopAddrsOffset", 0, 0, 0 },
    { /* 4*/  "startAddrsCoarseOffset", 0, 0, 0 },
    { /* 5*/  "modLfoToPitch", 0, 0, 0 },
    { /* 6*/  "vibLfoToPitch", 0, 0, 0 },
    { /* 7*/  "modEnvToPitch", 0, 0, 0 },
    { /* 8*/  "initialFilterFc",            1500, 13500, 13500 },
    { /* 9*/  "initialFilterQ", 0, 0, 0 },
    { /*10*/  "modLfoToFilterFc", 0, 0, 0 },
    { /*11*/  "modEnvToFilterFc", 0, 0, 0 },
    { /*12*/  "endAddrsCoarseOffset", 0, 0, 0 },
    { /*13*/  "modLfoToVolume", 0, 0, 0 },
    { /*14*/  "unused1", 0, 0, 0 },
    { /*15*/  "chorusEffectsSend", 0, 0, 0 },
    { /*16*/  "reverbEffectsSend", 0, 0, 0 },
    { /*17*/  "pan", 0, 0, 0 },
    { /*18*/  "unused2", 0, 0, 0 },
    { /*19*/  "unused3", 0, 0, 0 },
    { /*20*/  "unused4", 0, 0, 0 },
    { /*21*/  "delayModLFO", 0, 0, 0 },
    { /*22*/  "freqModLFO", 0, 0, 0 },
    { /*23*/  "delayVibLFO", 0, 0, 0 },
    { /*24*/  "freqVibLFO", 0, 0, 0 },
    { /*25*/  "delayModEnv", 0, 0, 0 },
    { /*26*/  "attackModEnv", 0, 0, 0 },
    { /*27*/  "holdModEnv", 0, 0, 0 },
    { /*28*/  "decayModEnv", 0, 0, 0 },
    { /*29*/  "sustainModEnv", 0, 0, 0 },
    { /*30*/  "releaseModEnv", 0, 0, 0 },
    { /*31*/  "keynumToModEnvHold", 0, 0, 0 },
    { /*32*/  "keynumToModEnvDecay", 0, 0, 0 },
    { /*33*/  "delayVolEnv",                -12000, 5000, -12000 },
    { /*34*/  "attackVolEnv",               -12000, 8000, -12000 },
    { /*35*/  "holdVolEnv",                 -12000, 5000, -12000 },
    { /*36*/  "decayVolEnv",                -12000, 8000, -12000 },
    { /*37*/  "sustainVolEnv",                   0, 1440,      0 },
    { /*38*/  "releaseVolEnv",              -12000, 8000, -12000 },
    { /*39*/  "keynumToVolEnvHold", 0, 0, 0 },
    { /*40*/  "keynumToVolEnvDecay", 0, 0, 0 },
    { /*41*/  "instrument", 0, 0, 0 },
    { /*42*/  "reserved1", 0, 0, 0 },
    { /*43*/  "keyRange", 0, 0, 0 },
    { /*44*/  "velRange", 0, 0, 0 },
    { /*45*/  "startloopAddrsCoarseOffset", 0, 0, 0 },
    { /*46*/  "keynum", 0, 0, 0 },
    { /*47*/  "velocity", 0, 0, 0 },
    { /*48*/  "initialAttenuation", 0, 0, 0 },
    { /*49*/  "reserved2", 0, 0, 0 },
    { /*50*/  "endloopAddrsCoarseOffset", 0, 0, 0 },
    { /*51*/  "coarseTune", 0, 0, 0 },
    { /*52*/  "fineTune", 0, 0, 0 },
    { /*53*/  "sampleID", 0, 0, 0 },
    { /*54*/  "sampleModes", 0, 0, 0 },
    { /*55*/  "reserved3", 0, 0, 0 },
    { /*56*/  "scaleTuning", 0, 0, 0 },
    { /*57*/  "exclusiveClass", 0, 0, 0 },
    { /*58*/  "overridingRootKey", 0, 0, 0 },
    { /*59*/  "unused5", 0, 0, 0 },
    { /*60*/  "endOper", 0, 0, 0 }
};

const char* RESoundFont::NameOfSF2Gen(int gen)
{
    if(gen >= 0 && gen < 60) {
        return _SF2GeneratorInfoArray[gen].name;
    }
    return "";
}

const RESoundFont::SF2GeneratorInfo& RESoundFont::InfoOfSF2Gen(int gen) 
{
    return _SF2GeneratorInfoArray[gen];
}

