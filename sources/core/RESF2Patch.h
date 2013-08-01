//
//  RESF2Patch.h
//  Reflow
//
//  Created by Sebastien on 15/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESF2Patch_h
#define Reflow_RESF2Patch_h

#include "RETypes.h"


/** RESF2Patch class.
 */
class RESF2Patch
{
    friend class RESoundFont;
    
public:
    const RESoundFont* SoundFont() const {return _sf2;}
    RESoundFont* SoundFont() {return _sf2;}
    
    unsigned int GeneratorCount() const {return _generators.size();}
    const RESF2Generator* Generator(int idx) const;
    RESF2Generator* Generator(int idx);
    
    void SetName(const std::string& name) {_name = name;}
    const std::string& Name() const {return _name;}
    
    int FindGenerators(int pitch, int velocity, RESF2GeneratorVector* foundGenerators);
    
private:
    RESF2Patch(RESoundFont* sf2);
    ~RESF2Patch();
    
    void AddGenerator(RESF2Generator* gen);
    
private:
    RESoundFont* _sf2;
    RESF2GeneratorVector _generators;
    std::string _name;
};

#endif
