//
//  RESF2Patch.cpp
//  Reflow
//
//  Created by Sebastien on 15/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESF2Patch.h"
#include "RESoundFont.h"
#include "RESF2Generator.h"

#include <boost/foreach.hpp>

RESF2Patch::RESF2Patch(RESoundFont* sf2)
: _sf2(sf2)
{
}

RESF2Patch::~RESF2Patch()
{
    BOOST_FOREACH(RESF2Generator* gen, _generators) {
        delete gen;
    }
    _generators.clear();
}

void RESF2Patch::AddGenerator(RESF2Generator* gen)
{
    gen->_patch = this;
    gen->_Finalize();
    _generators.push_back(gen);
}

const RESF2Generator* RESF2Patch::Generator(int idx) const
{
    if(idx >= 0 && idx < GeneratorCount()) {
        return _generators[idx];
    }
    return NULL;
}

RESF2Generator* RESF2Patch::Generator(int idx)
{
    if(idx >= 0 && idx < GeneratorCount()) {
        return _generators[idx];
    }
    return NULL;    
}

int RESF2Patch::FindGenerators(int pitch, int velocity, RESF2GeneratorVector* foundGenerators)
{
    int nbGeneratorsFound = 0;
    for(unsigned int i=0; i<GeneratorCount(); ++i) 
    {
        RESF2Generator* generator = Generator(i);
        if(generator->IsPitchInRange(pitch) && generator->IsVelocityInRange(velocity)) {
            foundGenerators->push_back(generator);
            ++nbGeneratorsFound;
        }
    }
    return nbGeneratorsFound;
}
