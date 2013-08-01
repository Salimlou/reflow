//
//  REGuitarProWriter.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REGuitarProWriter_h
#define Reflow_REGuitarProWriter_h

#include "RETypes.h"
#include "REOutputStream.h"

class REGuitarProWriter
{
public:
    REGuitarProWriter();
    virtual ~REGuitarProWriter();
    
public:
    bool ExportSongToFile(const RESong* song, const std::string& filename);
    
private:
    void WriteSong();
    void WriteInformation();
    void WritePageSettings();
    void WriteMeasure(int barIndex);
    void WriteTrack(int trackIndex);
    void WriteMeasureOfTrack(int barIndex, int trackIndex);
    void WriteBeat(const REChord* chord);
    
    void SkipBytes(int nbBytes);
    void WriteGPString(const std::string& str);
    void WriteGPStringFixed(const std::string& str, unsigned long fixedLength);
    void WriteInt32AndGPString(const std::string& str);
    void WriteGPStringAlt(const std::string& str);
    
private:
    REBufferOutputStream _data;
    const RESong* _song;
    std::string _filename;
    
    int _currentBarIndex;
    int _currentTrackIndex;
};

#endif
