//
//  REGrip.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 19/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REGrip.h"
#include "REFunctions.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using std::vector;
using std::string;
using namespace boost;

REGrip::REGrip()
: _stringCount(0)
{
    Clear();
}

REGrip::REGrip(const char* grip)
: _stringCount(0)
{
    Clear();
    BuildWithString(grip);
}

void REGrip::Clear()
{
    ::memset(_frets, 0, sizeof(int8_t) * REFLOW_MAX_STRINGS);
    _playStringFlags = 0;
}

bool REGrip::IsValid() const
{
    return _stringCount != 0;
}

int REGrip::StringCount() const
{
    return _stringCount;
}

bool REGrip::IsOpenChord() const
{
    for(int i=0; i<_stringCount; ++i)
    {
        if( _frets[i] == 0 && (_playStringFlags & (1 << i)) != 0) {
            return true;
        }
    }
    return false;
}

bool REGrip::IsTransposable() const
{
    return !IsOpenChord();
}

bool REGrip::IsStringPlayed(int str) const
{
    if(str >= 0 && str < _stringCount) {
        return 0 != (_playStringFlags & (1 << str));
    }
    return false;
}

bool REGrip::BuildWithString(const char* grip_)
{
    std::string grip(grip_);
    trim(grip);
    
    vector<string> components;
    split(components, grip, is_any_of(" "));
    _stringCount = components.size();
    if(_stringCount < 4) {
        _stringCount = 0; return false;
    }
    
    for(int i=0; i<_stringCount; ++i)
    {
        std::string fret = trim_copy(components[i]);
        if(fret == "x" || fret == "X") {
            _frets[i] = 0;
        }
        else if(fret == "o" || fret == "O") {
            _playStringFlags |= (1 << i);
            _frets[i] = 0;
        }
        else {
            try {
                _frets[i] = lexical_cast<int>(fret);
                _playStringFlags |= (1 << i);
            }
            catch (std::exception& ex) {
                _stringCount = 0;
                Clear();
                return false;
            }
        }
    }
    
    return true;
}

RERange REGrip::FretRange() const
{
    bool found = false;
    int minFret = 0;
    int maxFret = 0;
    for(int i=0; i<_stringCount; ++i)
    {
        if(_frets[i] != 0 && (_playStringFlags & (1 << i)) != 0) 
        {
            if(found) {
                if(_frets[i] < minFret) minFret = _frets[i];
                if(_frets[i] > maxFret) maxFret = _frets[i];
            }
            else {
                minFret = maxFret = _frets[i];
                found = true;
            }
        }
    }
    if(found) {
        return RERange(minFret, maxFret-minFret+1);
    }
    else {
        return RERange();
    }
}

std::string REGrip::ToString() const
{
    std::ostringstream oss;
    for(int i=0; i<_stringCount; ++i)
    {
        if(i > 0) oss << ' ';
        
        if(IsStringPlayed(i)) {
            oss << (int)(_frets[i]);
        }
        else {
            oss << 'x';
        }
    }
    return oss.str();
}

int REGrip::Fret(int idx) const
{
    if(idx >= 0 && idx < _stringCount) {
        return _frets[idx];
    }
    else return 0;
}

REGrip REGrip::Transposed(int dfret) const
{
    REGrip grip = *this;
    grip.Transpose(dfret);
    return grip;
}

void REGrip::Transpose(int dfret)
{
    dfret = Reflow::Wrap12(dfret);
    RERange rg = FretRange();
    int minFret = dfret + rg.FirstIndex();
    for(int i=0; i<_stringCount; ++i)
    {
        if(_frets[i] != 0 && (_playStringFlags & (1 << i)) != 0) 
        {
            if(minFret > 12) {
                _frets[i] = (_frets[i] + dfret)%12;
            }
            else {
                _frets[i] += dfret;
            }
        }
    }
}

REGrip::REGrip(const REGrip& rhs)
{
    *this = rhs;
}

REGrip& REGrip::operator=(const REGrip& rhs)
{
    ::memcpy(_frets, rhs._frets, sizeof(_frets));
    _playStringFlags = rhs._playStringFlags;
    _stringCount = rhs._stringCount;
    return *this;
}

void REGrip::EncodeTo(REOutputStream& coder) const
{
    coder.WriteInt8(_stringCount);
    coder.WriteInt16(_playStringFlags);
    for(int i=0; i<_stringCount; ++i) {
        coder.WriteInt8(_frets[i]);
    }
}

void REGrip::DecodeFrom(REInputStream& decoder)
{
    _stringCount = decoder.ReadInt8();
    _playStringFlags = decoder.ReadInt16();
    for(int i=0; i<_stringCount; ++i) {
        _frets[i] = decoder.ReadInt8();
    }
}