//
//  RELogger.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RELogger_h
#define Reflow_RELogger_h

#include "RETypes.h"

class RELogger
{
public:
    virtual void Write(const std::string& str) = 0;
    
    void printf(const char* fmt, ...);
};

class REFileLogger : public RELogger
{
public:
    REFileLogger();
    REFileLogger(const std::string& filename, bool append=false);
    virtual ~REFileLogger();
    
    virtual void Write(const std::string& str);
    
    void Open(const std::string& filename, bool append=false);
    void Close();
    
private:
    FILE* _file;
};

#endif
