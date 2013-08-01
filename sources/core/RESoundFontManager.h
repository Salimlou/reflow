//
//  RESoundFontManager.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESoundFontManager_h
#define Reflow_RESoundFontManager_h

#include "RETypes.h"


class RESoundFontManager
{
public:
    static RESoundFontManager& Instance();
    
    RESoundFont* DefaultSoundFont();

    void SetDefaultSoundFontPath(const std::string& sf2Path) {_defaultSoundFontPath = sf2Path;}    
    
private:
    RESoundFontManager();
    
    void LoadDefaultSoundFont();
    RESoundFont* LoadSoundFont(const std::string& sf2Filename);
    
private:
    RESoundFont* _defaultSoundFont;
    std::string _defaultSoundFontPath;
    
private:
    static RESoundFontManager* _instance;
};

#endif
