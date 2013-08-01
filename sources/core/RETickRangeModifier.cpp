//
//  RETickRangeModifier.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RETickRangeModifier.h"
#include "REFunctions.h"

#include <sstream>

std::string RETickRangeModifierElement::ToString() const {
    std::ostringstream oss;
    oss << ValueAsString() << "[" << t0 << "-" << t1 << "]";
    return oss.str();
}

std::string REOttaviaRangeModifierElement::ValueAsString() const {
    return std::string(Reflow::NameOfOttavia(value));
}