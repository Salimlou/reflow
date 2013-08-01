//
//  REObject.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 05/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REObject.h"

REObject::REObject()
: _refcount(0)
{
    
}

REObject::~REObject()
{
}

REObject* REObject::Retain() 
{
    ++_refcount; 
    return this;
}

REObject* REObject::Release() 
{
    if(--_refcount <= 0) {
        delete this;
        return NULL;
    };
    return this;
}
