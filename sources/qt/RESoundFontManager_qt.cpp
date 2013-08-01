#include "RESoundFontManager.h"

#include "REAudioEngine.h"
#include "REMusicRack.h"
#include "REMusicDevice.h"
#include "RESoundFont.h"
#include "RESF2Patch.h"
#include "RESF2Generator.h"


void RESoundFontManager::LoadDefaultSoundFont()
{
    if(_defaultSoundFontPath.empty())
    {
        _defaultSoundFont = LoadSoundFont("GeneralUser.sf2");
    }
    else {
        _defaultSoundFont = LoadSoundFont(_defaultSoundFontPath);
    }
}

RESoundFont* RESoundFontManager::LoadSoundFont(const std::string& sf2Filename)
{
    RESoundFont* soundfont = new RESoundFont;
    if(!soundfont->readSF2File(sf2Filename)) {
        REPrintf("Failed to load %s\n", sf2Filename.c_str());
    }
    return soundfont;
}
