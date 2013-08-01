//
//  REWriteChunkToFile.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REWriteChunkToFile_h
#define Reflow_REWriteChunkToFile_h

#include "RETypes.h"

class REWriteChunkToFile
{
public:
    REWriteChunkToFile(FILE* file, const char* chunkId);
    ~REWriteChunkToFile();
    
private:
    FILE* _file;
    unsigned long _pos;
};


#endif
