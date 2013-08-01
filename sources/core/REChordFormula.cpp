//
//  REChordFormula.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 16/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REChordFormula.h"
#include "REChordName.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using std::vector;
using std::string;

using namespace boost;

std::map<std::string, REPitchClass> REChordFormula::_intervalsByName;

REChordFormula::REChordFormula()
: _intervalCount(0), _optionalDegrees(0)
{    
}

REChordFormula::REChordFormula(const std::string& rawFormula)
: _intervalCount(0), _optionalDegrees(0)
{
    Parse(rawFormula);
}

REChordFormula::REChordFormula(const std::string& rawFormula, const std::string& symbol)
: _intervalCount(0), _optionalDegrees(0)
{
    Parse(rawFormula);
    _symbol = symbol;
}

bool REChordFormula::IsValid() const
{
    return _intervalCount != 0;
}

bool REChordFormula::Parse(const std::string& rawFormula_)
{
    _intervalCount = 0;
    std::string rawFormula = rawFormula_;
    _rawFormula = rawFormula;
    
    // Remove Head
    if(!starts_with(rawFormula, "1,")) {
        return false;
    }
    erase_head(rawFormula, 2);
    
    // Split components
    vector<string> components;
    boost::split(components, rawFormula, boost::is_any_of(","));

    _intervalCount = components.size();
    if(_intervalCount < 1 || _intervalCount > 6) {
        _intervalCount = 0;
        return false;
    }
    
    for(int i=0; i<_intervalCount; ++i)
    {
        bool optional_degree = false;
        std::string s = components[i];
        trim(s);
        
        // Optional degree
        if(boost::starts_with(s, "(") && boost::ends_with(s, ")")) {
            optional_degree = true;
            _optionalDegrees |= (1 << i);
            erase_head(s, 1);
            erase_tail(s, 1);
        }
        
        bool error = false;
        REPitchClass interval = IntervalByName(s, &error);
        if(error) {
            _intervalCount = 0;
            return false;
        }
        
        _intervals[i] = interval;
    }
    
    return true;
}

REPitchClass REChordFormula::IntervalByName(const std::string& interval, bool* error)
{
    if(_intervalsByName.empty()) {
        InitializeIntervalsByNameLookupTable();
    }
    
    std::map<std::string, REPitchClass>::const_iterator it = _intervalsByName.find(interval);
    if(it == _intervalsByName.end())
    {
        *error = true;
        return REPitchClass(0,0);
    }
    else 
    {
        *error = false;
        return it->second;
    }
}

int REChordFormula::DegreeCount() const
{
    if(_intervalCount == 0) return 0;
    return _intervalCount + 1;
}

const REPitchClass& REChordFormula::Degree(int degree) const
{
    if(degree == 0) {
        return REPitchClass::Tonic;
    }
    degree = degree-1;
    if(degree >= 0 && degree < _intervalCount) {
        return _intervals[degree];
    }
    
    return REPitchClass::Tonic;
}

bool REChordFormula::IsDegreeOptional(int degree) const
{
    if(degree == 0) {
        return false;
    }
    degree = degree-1;
    if(degree >= 0 && degree < _intervalCount) {
        return 0 != (_optionalDegrees & (1 << degree));
    }
    
    return false;
}

void REChordFormula::InitializeIntervalsByNameLookupTable()
{
    _intervalsByName.clear();
    
    _intervalsByName[ "2"] = REPitchClass::MajorSecond;
    
    _intervalsByName["b3"] = REPitchClass::MinorThird;
    _intervalsByName[ "3"] = REPitchClass::MajorThird;
    
    _intervalsByName[ "4"] = REPitchClass::PerfectFourth;

    _intervalsByName["b5"] = REPitchClass::PerfectFifth.Diminished(1);
    _intervalsByName[ "5"] = REPitchClass::PerfectFifth;
    _intervalsByName["#5"] = REPitchClass::PerfectFifth.Augmented(1);

    _intervalsByName["6"] = REPitchClass::MajorSixth;
    
    _intervalsByName["bb7"] = REPitchClass::MajorSeventh.Diminished(2);
    _intervalsByName["b7"] = REPitchClass::MajorSeventh.Diminished(1);    
    _intervalsByName["7"] = REPitchClass::MajorSeventh;    
    
    _intervalsByName["b9"] = REPitchClass::Ninth.Diminished(1);
    _intervalsByName["9"] = REPitchClass::Ninth;    
    _intervalsByName["#9"] = REPitchClass::Ninth.Augmented(1);
    
    _intervalsByName["b11"] = REPitchClass::Eleventh.Diminished(1);
    _intervalsByName["11"] = REPitchClass::Eleventh;    
    _intervalsByName["#11"] = REPitchClass::Eleventh.Augmented(1);
    
    _intervalsByName["b13"] = REPitchClass::Thirteenth.Diminished(1);
    _intervalsByName["13"] = REPitchClass::Thirteenth;    
    _intervalsByName["#13"] = REPitchClass::Thirteenth.Augmented(1);
}

REChordFormula::REChordFormula(const REChordFormula& rhs)
{
    *this = rhs;
}

REChordFormula& REChordFormula::operator=(const REChordFormula& rhs)
{
    _intervalCount = rhs._intervalCount;
    memcpy(_intervals, rhs._intervals, sizeof(rhs._intervals));
    
    _rawFormula = rhs._rawFormula;
    _symbol = rhs._symbol;
    
    return *this;
}

void REChordFormula::EncodeTo(REOutputStream& coder) const
{
    coder.WriteString(_rawFormula);
    coder.WriteString(_symbol);
    coder.WriteInt8(_intervalCount);
    coder.WriteInt8(_optionalDegrees);
    for(int i=0; i<_intervalCount; ++i) {
        _intervals[i].EncodeTo(coder);
    }
}
void REChordFormula::DecodeFrom(REInputStream& decoder)
{
    _rawFormula = decoder.ReadString();
    _symbol = decoder.ReadString();
    _intervalCount = decoder.ReadInt8();
    _optionalDegrees = decoder.ReadInt8();
    for(int i=0; i<_intervalCount; ++i) {
        _intervals[i].DecodeFrom(decoder);
    }
}

void REChordFormula::CalculatePitchesWithRootNote(const REPitchClass& rootNote, REPitchClassVector& pitches) const
{
    if(!IsValid()) return;
    
    pitches.push_back(rootNote);
    for(int i=0; i<_intervalCount; ++i) {
        pitches.push_back(rootNote + _intervals[i]);
    }
}

void REChordFormula::SetSymbol(const std::string& symbol)
{
    _symbol = symbol;
}
const std::string& REChordFormula::Symbol() const
{
    return _symbol;
}