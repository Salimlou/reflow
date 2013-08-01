//
//  REWriteChunkToFile.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REWriteChunkToFile.h"

REWriteChunkToFile::REWriteChunkToFile(FILE* file, const char* chunkId)
: _file(file)
{
    uint32_t size = 0;
    _pos = ftell(_file);
    
    fwrite(chunkId, 4, 1, _file);
    fwrite((void*)&size, 4, 1, _file);
}

REWriteChunkToFile::~REWriteChunkToFile()
{
    uint32_t jumpPos = ftell(_file);
    uint32_t size = (jumpPos - _pos - 8);
    
    fseek(_file, _pos+4, SEEK_SET);
    fwrite((void*)&size, 4, 1, _file);
    
    fseek(_file, jumpPos, SEEK_SET);
}
