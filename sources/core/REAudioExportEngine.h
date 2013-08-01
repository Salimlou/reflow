//
//  REAudioExportEngine.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 20/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REAudioExportEngine_h
#define Reflow_REAudioExportEngine_h

#include "REAudioEngine.h"

class REAudioExportEngine : public REAudioEngine
{
public:
    REAudioExportEngine(const std::string& filename);
    virtual ~REAudioExportEngine();
    
    bool ExportSong(const RESong* song);
    void CancelExport();
    
public:
    virtual void Initialize();
    virtual void Shutdown();
    
    virtual void StartRendering();
    virtual void StopRendering();
    
private:
    std::string _filename;
    FILE* _file;
    uint16_t _nbChannels;
    uint16_t _bitsPerSample;
    bool _clip;
    bool _cancelRequested;
};


#endif
