//
//  RETimer.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 03/03/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RETimer_h
#define Reflow_RETimer_h

#include "RETypes.h"

#ifdef WIN32
#  include <Windows.h>
#else
#  include <sys/time.h>
#endif

class RETimer
{
public:
    RETimer();
    virtual ~RETimer();
    
    void Start();
    void Stop();
    
    double DeltaTimeInSeconds();
    double DeltaTimeInMilliseconds();
    double DeltaTimeInMicroseconds();    
    
private:
    double _startMicroseconds;
    double _endMicroseconds;
    bool _stopped;
    
#ifdef WIN32
	LARGE_INTEGER _frequency;
	LARGE_INTEGER _startTime;
	LARGE_INTEGER _endTime;
#else
    timeval _startTime;
    timeval _endTime;
#endif
};

#endif
