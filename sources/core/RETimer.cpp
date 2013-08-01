//
//  RETimer.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 03/03/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RETimer.h"

RETimer::RETimer()
: _startMicroseconds(0), _endMicroseconds(0), _stopped(false)
{
#ifdef WIN32
	QueryPerformanceFrequency(&_frequency);
	_startTime.QuadPart = 0;
	_endTime.QuadPart = 0;
#else
    _startTime.tv_sec = _startTime.tv_usec = 0;
    _endTime.tv_sec = _endTime.tv_usec = 0;
#endif
}

RETimer::~RETimer()
{
}

void RETimer::Start()
{
    _stopped = false;
#ifdef WIN32
	QueryPerformanceCounter(&_startTime);
#else
    gettimeofday(&_startTime, NULL);
#endif
}

void RETimer::Stop()
{
    _stopped = true;
#ifdef WIN32
	QueryPerformanceCounter(&_endTime);
#else
    gettimeofday(&_endTime, NULL);
#endif
}

double RETimer::DeltaTimeInMicroseconds()
{
#ifdef WIN32
	if(!_stopped) {
		QueryPerformanceCounter(&_endTime);
	}

	_startMicroseconds = _startTime.QuadPart * (1000000.0 / _frequency.QuadPart);
	_endMicroseconds = _endTime.QuadPart * (1000000.0 / _frequency.QuadPart);
#else
    if(!_stopped) {
        gettimeofday(&_endTime, NULL);
    }
    
    _startMicroseconds = (_startTime.tv_sec * 1000000.0) + _startTime.tv_usec;
    _endMicroseconds = (_endTime.tv_sec * 1000000.0) + _endTime.tv_usec;
#endif

    return _endMicroseconds - _startMicroseconds;
}

double RETimer::DeltaTimeInMilliseconds() 
{
    return 0.001 * DeltaTimeInMicroseconds();
}

double RETimer::DeltaTimeInSeconds() 
{
    return 0.000001 * DeltaTimeInMicroseconds();
}
