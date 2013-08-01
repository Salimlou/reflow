//
//  RESongError.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 29/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESongError_h
#define Reflow_RESongError_h

#include "RETypes.h"

class RESongError
{
public:
    enum Code {
        NoError = 0,
        
    };
    
public:
    RESongError() : _errorCode(NoError) {}
    RESongError(RESongError::Code code) : _errorCode(code) {}
    
    Code ErrorCode() const {return _errorCode;}
    
    std::string ToString() const;
    
private:
    Code _errorCode;
};

typedef std::vector<RESongError> RESongErrorVector;

#endif
