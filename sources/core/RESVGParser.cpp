//
//  RESVGParser.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 09/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RESVGParser.h"
#include "REPainter.h"
#include "REBezierPath.h"
#include "REException.h"
#include "REFunctions.h"

#include <boost/algorithm/string/erase.hpp>

#include <sstream>

class RESVGParserCommand
{
public:    
    RESVGParserCommand() : _command(' ') {}
    
    RESVGParserCommand(char command) : _command(command) {}
    
    RESVGParserCommand(const RESVGParserCommand& token) {*this = token;}
    
    RESVGParserCommand& operator=(const RESVGParserCommand& token) {
        _command = token._command;
        _values = token._values;
        return *this;
    }
    
    void AddValue(double val) {_values.push_back(val);}
    
    int ValueCount() const {return _values.size();}
    
    double Value(int idx) const {
        if(idx >= 0 && idx < _values.size()) {
            return _values[idx];
        }
        else {
            std::ostringstream oss; oss << "Value " << idx << " is out of bounds for command [" << _command << "]";
            REThrow(oss.str());
        }
    }
    
    char Command() const {return _command;}
    
    bool IsRelative() const {return islower(_command);}
    
public:
    char _command;
    std::vector<double> _values;
};



static const char * const _separatorChars  = "MmLlHhVvCcSsZzQqAa,-";
static const char* const _commandChars     = "MmLlHhVvCcSsZzQqAa";
static const int _separatorCharsLen = 20;
static const int _commandCharsLen = 18;

RESVGParser::RESVGParser()
{
    
}

RESVGParser::~RESVGParser()
{
    
}

REBezierPath* RESVGParser::ParsePathData(const std::string& d, const RERect& rect)
{
    _viewBox = rect;
    std::string d_ = boost::erase_all_copy(d, " ");
    return _Parse(d_);
}

static bool _CharacterIsCommand(char c) {
    for(int i=0; i<_commandCharsLen; ++i) {
        if(c == _commandChars[i]) return true;
    }
    return false;
}

static bool _CharacterIsSeparator(char c) {
    for(int i=0; i<_separatorCharsLen; ++i) {
        if(c == _separatorChars[i]) return true;
    }
    return false;
}

void RESVGParser::_ProcessCommandMove(const RESVGParserCommand& command, REBezierPath& path)
{
    int idx = 0;
    bool moveTo = true;
    
    while(idx < command.ValueCount())
    {
        double x = command.Value(idx++);
        double y = command.Value(idx++);
        if(command.IsRelative()) {
            x += _lastPoint.x;
            y += _lastPoint.y;
        }
        _lastPoint = REPoint(x,y);
        
        // Move To followed by implicit (and optional) Line-Tos
        if(moveTo) {
            path.MoveToPoint(x, y);
        }
        else {
            path.LineToPoint(x, y);
            moveTo = false;
        }
    }
}
void RESVGParser::_ProcessCommandLine(const RESVGParserCommand& command, REBezierPath& path)
{
    int idx = 0;
    while(idx < command.ValueCount())
    {
        double x = 0;
        double y = 0;
        
		switch(command.Command())
        {
			case 'l':
				x = _lastPoint.x;
				y = _lastPoint.y;
			case 'L':
				x += command.Value(idx++);
				y += command.Value(idx++);
				break;
                
			case 'h' :
				x = _lastPoint.x;				
			case 'H' :
				x += command.Value(idx++);
				y = _lastPoint.y;
				break;
                
			case 'v' :
				y = _lastPoint.y;
			case 'V' :
				y += command.Value(idx++);
				x = _lastPoint.x;
				break;
		}
		_lastPoint = REPoint(x, y);
        path.LineToPoint(x, y);
	}
}
void RESVGParser::_ProcessCommandCurve(const RESVGParserCommand& command, REBezierPath& path)
{
    int idx = 0;
    while(idx < command.ValueCount())
    {
        double x1 = command.Value(idx++);
        double y1 = command.Value(idx++);
        double x2 = command.Value(idx++);
        double y2 = command.Value(idx++);
        double x  = command.Value(idx++);
        double y  = command.Value(idx++);
        if(command.IsRelative()) {
            x1 += _lastPoint.x;
            y1 += _lastPoint.y;
            x2 += _lastPoint.x;
            y2 += _lastPoint.y;
            x  += _lastPoint.x;
            y  += _lastPoint.y;
        }
        
        _lastPoint = REPoint(x,y);
        _lastCurveControlPoint = REPoint(x2,y2);
        path.CurveToPoint(REPoint(x,y), REPoint(x1,y1), REPoint(x2,y2));
    }
}
void RESVGParser::_ProcessCommandSmoothCurve(const RESVGParserCommand& command, REBezierPath& path)
{
    int idx = 0;
    while(idx < command.ValueCount())
    {
        double x1 = _lastCurveControlPoint.x;
        double y1 = _lastCurveControlPoint.y;
        double x2 = command.Value(idx++);
        double y2 = command.Value(idx++);
        double x  = command.Value(idx++);
        double y  = command.Value(idx++);
        if(command.IsRelative()) {
            x2 += _lastPoint.x;
            y2 += _lastPoint.y;
            x  += _lastPoint.x;
            y  += _lastPoint.y;
        }
        
        _lastPoint = REPoint(x,y);
        _lastCurveControlPoint = REPoint(x2,y2);
        path.CurveToPoint(REPoint(x,y), REPoint(x1,y1), REPoint(x2,y2));
    }
}
void RESVGParser::_ProcessCommandClose(const RESVGParserCommand& /*command*/, REBezierPath& path)
{
    path.Close();
}

REBezierPath* RESVGParser::_Parse(const std::string& d)
{
    REBezierPath* path = NULL;
    try {
        // Parse tokens
        std::vector<std::string> tokens;
        int length = d.size();
        int idx = 0;
        while(idx < length)
        {
            std::string token = "";
            char currentChar = d[idx];
            
            if(currentChar != ',') {
                token += currentChar;
            }
            
            if(!_CharacterIsCommand(currentChar) && currentChar != ',') {
                while ( (++idx < length) && !_CharacterIsSeparator(currentChar = d[idx]))
                {
                    token += currentChar;
                }
            }
            else {
                ++idx;
            }
            
            if(!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        // No token found
        if(tokens.empty()) REThrow("Path Data is empty");
        
        // Commands
        std::vector<RESVGParserCommand> commands;
        int nbTokens = tokens.size();
        idx = 0;
        const std::string* token = &tokens[idx];
        char command = token->at(0);
        while (idx < nbTokens) 
        {
            if(!_CharacterIsCommand(command)) {
                REThrow("SVG Parse Error");
            }
            
            RESVGParserCommand svgCommand(command);
            while(++idx < nbTokens)
            {
                token = &tokens[idx];
                command = token->at(0);
                if(!_CharacterIsCommand(command))
                {
                    bool minus = ('-' == token->at(0));
                    double value = (minus ? -Reflow::StringToFloat(token->substr(1)) : Reflow::StringToFloat(*token));
                    svgCommand.AddValue(value);
                }
                else break;
            }
            
            commands.push_back(svgCommand);
        }
        
        // Generate Bezier
        path = new REBezierPath;
        _lastPoint = REPoint(0,0);
        _lastCurveControlPoint = REPoint(0,0);
        std::vector<RESVGParserCommand>::const_iterator it = commands.begin();
        for(; it != commands.end(); ++it)
        {
            const RESVGParserCommand& svgCommand = *it;
            switch(svgCommand.Command())
            {
                case 'M': case 'm':
                    _ProcessCommandMove(svgCommand, *path);
                    break;
                case 'L': case 'l':
                case 'H': case 'h':
                case 'V': case 'v':
                    _ProcessCommandLine(svgCommand, *path);
                    break;
                case 'C': case 'c':
                    _ProcessCommandCurve(svgCommand, *path);
                    break;
                case 'S': case 's':
                    _ProcessCommandSmoothCurve(svgCommand, *path);
                    break;
                case 'Z': case 'z':
                    _ProcessCommandClose(svgCommand, *path);
                    break;
                default: {
                    std::ostringstream oss; oss << "Unknown Command [" << svgCommand.Command() << "]";
                    REThrow(oss.str());
                }
            }
        }
    }
    catch (std::exception& e) {
        REPrintf("Exception Caught: %s\n", e.what());
        if(path) delete path;
        return NULL;
    }
    return path;
}

