//
//  RESoundFontManager.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RESoundFontManager.h"
#include "RESoundFont.h"

RESoundFontManager* RESoundFontManager::_instance = NULL;

RESoundFontManager::RESoundFontManager()
: _defaultSoundFont(NULL), _defaultSoundFontPath("")
{
}

RESoundFontManager& RESoundFontManager::Instance()
{
    if(_instance == NULL) {
        _instance = new RESoundFontManager;
    }
    return *_instance;
}

RESoundFont* RESoundFontManager::DefaultSoundFont()
{
    if(_defaultSoundFont == NULL) {
        LoadDefaultSoundFont();
    }
    return _defaultSoundFont;
}
