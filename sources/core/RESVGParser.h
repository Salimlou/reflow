//
//  RESVGParser.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef _RESVGPARSER_H_
#define _RESVGPARSER_H_

#include "RETypes.h"

class RESVGParserCommand;

class RESVGParser
{
public:
    RESVGParser();
    ~RESVGParser();
    
    REBezierPath* ParsePathData(const std::string& d, const RERect& rect);
    
private:
    REBezierPath* _Parse(const std::string& d);
    
    void _ProcessCommandMove(const RESVGParserCommand& command, REBezierPath& path);
    void _ProcessCommandLine(const RESVGParserCommand& command, REBezierPath& path);
    void _ProcessCommandCurve(const RESVGParserCommand& command, REBezierPath& path);
    void _ProcessCommandSmoothCurve(const RESVGParserCommand& command, REBezierPath& path);
    void _ProcessCommandClose(const RESVGParserCommand& command, REBezierPath& path);
    
private:
    RERect _viewBox;
    REPoint _lastPoint;
    REPoint _lastCurveControlPoint;
};


#endif
