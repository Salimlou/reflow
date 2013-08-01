//
//  RESongError.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 29/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RESongError.h"

#include <boost/format.hpp>

using namespace boost;

std::string RESongError::ToString() const
{
    return str (format("Error (Code %1%)") % (int)_errorCode);
}
