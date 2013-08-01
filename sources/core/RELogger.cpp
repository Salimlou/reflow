//
//  RELogger.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RELogger.h"

#include <stdarg.h>

void RELogger::printf(const char* fmt, ...)
{
    char string[1024] = {0};
    va_list arg_ptr;
    
    va_start(arg_ptr, fmt);
    vsnprintf(string, 1024, fmt, arg_ptr);
    va_end(arg_ptr);
    
    Write(string);
}

REFileLogger::REFileLogger()
: _file(NULL)
{
}

REFileLogger::REFileLogger(const std::string& filename, bool append)
: _file(NULL)
{
    Open(filename, append);
}

REFileLogger::~REFileLogger()
{
    Close();
}

void REFileLogger::Write(const std::string& str)
{
    if(_file) {
        fputs(str.c_str(), _file);
    }
}

void REFileLogger::Open(const std::string& filename, bool append)
{
    if(_file != NULL) {
        Close();
    }

    _file = fopen(filename.c_str(), append ? "a" : "w");
}

void REFileLogger::Close()
{
    if(_file != NULL) {
        fclose(_file);
        _file = NULL;
    }
}
