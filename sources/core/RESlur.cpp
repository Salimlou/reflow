//
//  RESlur.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/04/13.
//
//

#include "RESlur.h"
#include "REInputStream.h"
#include "REOutputStream.h"
#include "REPainter.h"
#include "REBezierPath.h"


RESlur::RESlur()
{
    _startBeat = REGlobalTimeDiv(0, 0);
    _endBeat = REGlobalTimeDiv(2, 2);
    
    _startOffset = REPoint(0, -15.0);
    _endOffset = REPoint(0, -15.0);
    
    _startControlPointOffset = REPoint(15, -15);
    _endControlPointOffset = REPoint(-15, -15);
    
    _thickness = 4.0f;
}

RESlur::RESlur(const RESlur& s)
{
    *this = s;
}

RESlur& RESlur::operator=(const RESlur& s)
{
    _startBeat = s._startBeat;
    _endBeat = s._endBeat;
    _startOffset = s._startOffset;
    _endOffset = s._endOffset;
    _startControlPointOffset = s._startControlPointOffset;
    _endControlPointOffset = s._endControlPointOffset;
    _thickness = s._thickness;
    return *this;
}

RERange RESlur::BarRange() const
{
    return RERange(_startBeat.bar, (_endBeat.bar-_startBeat.bar)+1);
}

float RESlur::MinY() const
{
    float minY = 0.0f;
    float yStartCP = _startOffset.y + _startControlPointOffset.y;
    float yEndCP = _endOffset.y + _endControlPointOffset.y;
    
    if(_startOffset.y < minY) minY = _startOffset.y;
    if(_endOffset.y < minY) minY = _endOffset.y;
    if(yStartCP < minY) minY = yStartCP;
    if(yEndCP < minY) minY = yEndCP;
    
    return minY;
}

float RESlur::MaxY() const
{
    float maxY = 0.0;
    float yStartCP = _startOffset.y + _startControlPointOffset.y;
    float yEndCP = _endOffset.y + _endControlPointOffset.y;
    
    if(_startOffset.y > maxY) maxY = _startOffset.y;
    if(_endOffset.y > maxY) maxY = _endOffset.y;
    if(yStartCP > maxY) maxY = yStartCP;
    if(yEndCP > maxY) maxY = yEndCP;
    
    return maxY;
}

void RESlur::Draw(REPainter& painter, float x0, float x1, float y, const REColor& color) const
{
    REPoint p_start = REPoint(x0, y) + StartOffset();
    REPoint p_start_cp = p_start + StartControlPointOffset();
    REPoint p_end = REPoint(x1, y) + EndOffset();
    REPoint p_end_cp = p_end + EndControlPointOffset();
    REPoint deltaThickness = REPoint(0, _thickness);
    
    REBezierPath slurPath;
    slurPath.MoveToPoint(p_start);
    slurPath.CurveToPoint(p_end, p_start_cp, p_end_cp);
    slurPath.CurveToPoint(p_start, p_end_cp + deltaThickness, p_start_cp + deltaThickness);
    slurPath.Close();
    
    painter.SetFillColor(color);
    painter.FillPath(slurPath);
}

void RESlur::EncodeTo(REOutputStream& coder) const
{
    _startBeat.EncodeTo(coder);
    _endBeat.EncodeTo(coder);
    coder.WriteFloat(_startOffset.x);
    coder.WriteFloat(_startOffset.y);
    coder.WriteFloat(_endOffset.x);
    coder.WriteFloat(_endOffset.y);
    coder.WriteFloat(_startControlPointOffset.x);
    coder.WriteFloat(_startControlPointOffset.y);
    coder.WriteFloat(_endControlPointOffset.x);
    coder.WriteFloat(_endControlPointOffset.y);
    coder.WriteFloat(_thickness);
}

void RESlur::DecodeFrom(REInputStream& decoder)
{
    _startBeat.DecodeFrom(decoder);
    _endBeat.DecodeFrom(decoder);
    _startOffset.x = decoder.ReadFloat();
    _startOffset.y = decoder.ReadFloat();
    _endOffset.x = decoder.ReadFloat();
    _endOffset.y = decoder.ReadFloat();
    _startControlPointOffset.x = decoder.ReadFloat();
    _startControlPointOffset.y = decoder.ReadFloat();
    _endControlPointOffset.x = decoder.ReadFloat();
    _endControlPointOffset.y = decoder.ReadFloat();
    _thickness = decoder.ReadFloat();
}

