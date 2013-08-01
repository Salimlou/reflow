//
//  RETypes.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 19/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RETypes.h"
#include "REPitchClass.h"
#include "REInputStream.h"
#include "REOutputStream.h"

#include <sstream>
#include <cmath>

const REColor REColor::White = REColor(1,1,1,1);
const REColor REColor::Black = REColor(0,0,0,1);
const REColor REColor::Red = REColor(1,0,0,1);
const REColor REColor::Green = REColor(0,1,0,1);
const REColor REColor::Blue = REColor(0,0,1,1);
const REColor REColor::DarkGray = REColor(0.25, 0.25, 0.25, 1.0);
const REColor REColor::Gray = REColor(0.50, 0.50, 0.50, 1.0);
const REColor REColor::LightGray = REColor(0.75, 0.75, 0.75, 1.0);
const REColor REColor::TransparentBlack = REColor(0,0,0,0);


bool RERect::PointInside(const REPoint& pt) const 
{
    return pt.x >= Left() && pt.x <= Right() && pt.y >= Top() && pt.y <= Bottom();
}

bool RERect::Contains(const RERect& rc) const
{
    return Left() <= rc.Left() && Right() >= rc.Right() && Top() <= rc.Top() && Bottom() <= rc.Bottom();
}

RERect RERect::FromPoints(const REPoint& a, const REPoint& b)
{
    float x = std::min<float>(a.x, b.x);
    float y = std::min<float>(a.y, b.y);
    float w = fabs(b.x - a.x);
    float h = fabs(b.y - a.y);
    return RERect(x, y, w, h);
}

RERect RERect::Union(const RERect& rc) const
{
    double minLeft = std::min<double>(Left(), rc.Left());
    double minTop = std::min<double>(Top(), rc.Top());
    double maxRight = std::max<double>(Right(), rc.Right());
    double maxBottom = std::max<double>(Bottom(), rc.Bottom());
    return RERect(minLeft, minTop, maxRight-minLeft, maxBottom-minTop);
}

RERect RERect::Inset(float top, float left, float bottom, float right) const
{
    float x = origin.x + left;
    float y = origin.y + top;
    float w = size.w - left - right;
    float h = size.h - top - bottom;
    return RERect(x, y, w, h);
}

bool RERect::Intersects(const RERect& a, const RERect& b)
{
    return  a.Left() < b.Right()  && a.Right()  > b.Left() &&
            a.Top()  < b.Bottom() && a.Bottom() > b.Top();
}

REColor REColor::Lerp(const REColor& a, const REColor& b, float t)
{
    return REColor((1.0f - t) * a.r + t * b.r,
                   (1.0f - t) * a.g + t * b.g,
                   (1.0f - t) * a.b + t * b.b,
                   (1.0f - t) * a.a + t * b.a);
}

REBeamingPattern::REBeamingPattern()
: _groupCount(0)
{
    Clear();
}

std::string REBeamingPattern::ToString() const
{
    std::ostringstream oss;
    for(int i=0; i<_groupCount; ++i) {
        if(i != 0) oss << '-';
        oss << (int)_groups[i];
    }
    return oss.str();
}

void REBeamingPattern::Clear()
{
    _groupCount = 0;
    memset(_groups, 0, sizeof(_groups));
}

bool REBeamingPattern::Parse(const char* str, int len)
{
    Clear();
    
    if(str == NULL) {
        
        return true;
    }
    
    if(len == -1) len = strlen(str);
    
    int i = 0;
    bool waitingDigit = true;
    while(i < len && _groupCount < REBeamingPattern::MaxGroupCount)
    {
        char c = str[i];
        
        if(waitingDigit)
        {
            // Digit ?
            if(!isdigit(c)) {
                Clear();
                return false;
            }
            
            // Add a group
            _groups[_groupCount++] = (c-'0');
            waitingDigit = false;
            ++i;
        }
        else
        {
            if(c == '-' || c == ' ')
            {
                waitingDigit = true;
                ++i;
            }
            else
            {
                Clear();
                return false;
            }
        }
    }
    
    return !waitingDigit || _groupCount == 0;
}

int REBeamingPattern::ToTickPattern(unsigned long* pattern) const
{
    if(_groupCount == 0){
        pattern[0] = REFLOW_PULSES_PER_QUARTER;
        return 1;
    }
    
    int count = std::min<int>(_groupCount, REBeamingPattern::MaxGroupCount);
    for(int i=0; i<count; ++i)
    {
        pattern[i] = (_groups[i] * (REFLOW_PULSES_PER_QUARTER / 2));
    }
    return count;
}

void REGlobalTimeDiv::EncodeTo(REOutputStream& coder) const
{
    coder.WriteInt32(bar);
    coder.WriteInt32(timeDiv.numerator());
    coder.WriteInt32(timeDiv.denominator());
}
void REGlobalTimeDiv::DecodeFrom(REInputStream& decoder)
{
    bar = decoder.ReadInt32();
    
    int32_t num = decoder.ReadInt32();
    int32_t den = decoder.ReadInt32();
    timeDiv = RETimeDiv(num, den);
}

RENotePitch RENotePitch::Transposed(const REPitchClass& interval) const
{
    REPitchClass pitchClass = REPitchClass(step, midi%12) - interval;
    
    RENotePitch pitch;
    pitch.step = pitchClass.ChromaticStep();
    pitch.alter = pitchClass.DiatonicAlteration();
    pitch.octave = midi/12;
    pitch.midi = midi - interval.ChromaticStep();
    return pitch;
}

#if defined(REFLOW_VERBOSE) && defined(REFLOW_QT)

#include "RELogger.h"
#include <stdarg.h>

static REFileLogger* _logger;
void REPrintf(const char* fmt, ...)
{
	if(_logger == NULL)
	{
		_logger = new REFileLogger("REConsole.txt");
	}

	char string[2048] = {0};
    va_list arg_ptr;
    
    va_start(arg_ptr, fmt);
    vsnprintf(string, 2048, fmt, arg_ptr);
    va_end(arg_ptr);
    
    _logger->Write(string);
}
#endif
