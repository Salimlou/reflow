//
//  REChordDiagram.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 18/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REChordDiagram.h"
#include "REPainter.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REFunctions.h"

#include <cmath>
#include <sstream>

REChordDiagram::REChordDiagram()
{
}

REChordDiagram::REChordDiagram(const REChordDiagram& rhs)
{
    *this = rhs;
}

REChordDiagram& REChordDiagram::operator=(const REChordDiagram& rhs)
{
    _chord = rhs.ChordName();
    _grip = rhs.Grip();
    return *this;
}

void REChordDiagram::EncodeTo(REOutputStream& coder) const
{
    _chord.EncodeTo(coder);
    _grip.EncodeTo(coder);
}

void REChordDiagram::DecodeFrom(REInputStream& decoder)
{
    _chord.DecodeFrom(decoder);
    _grip.DecodeFrom(decoder);
}

void REChordDiagram::SetGrip(const REGrip& grip)
{
    _grip = grip;
}

void REChordDiagram::SetChordName(const REChordName& name)
{
    _chord = name;
}

RERect REChordDiagram::BoundingBoxForSize(float size) const
{
    if(!_grip.IsValid()) return RERect(0,0,0,0);
    
    const int maxFret = 6;
    int stringCount = _grip.StringCount();
    float dx = size;
	float dy = roundf(1.25 * size);
    
    float x = 0;
    float y = dx + 1.0;
    float x0 = roundf(x - (float(stringCount) * dx) / 2.0);
    float x1 = roundf(x0 + (stringCount-1) * dx);
    
    float y0 = y;
    float y1 = y + (maxFret) * dy + 0.25 * dy;
    
    RERect bbox(x0, 0, (x1-x0), y1);
    
    // Delta Fret
    RERange fretRange = _grip.FretRange();
    int deltaFret = 0;
    if(fretRange.LastIndex() > 4) {
        deltaFret = 1 - fretRange.FirstIndex();
    }
    
    if(deltaFret) {
        int df = 1 - deltaFret;
        std::ostringstream oss; oss << df;
        REPoint pt(x0 - dx, y0);
        if(df >= 10) {
            pt.x -= dx/2;
        }
        
        bbox.origin.x = pt.x;
        bbox.size.w = (x1 - bbox.origin.x);
    }
    return bbox;
}

float REChordDiagram::HeightForSize(float size)
{
    const int maxFret = 6;
    float dx = size;
    float dy = roundf(1.25 * size);
    
    float y = 0 + dx + 1.0;
    
    float y0 = y;
    float y1 = y + (maxFret) * dy + 0.25 * dy;
    return y1;
}

void REChordDiagram::Draw(REPainter& painter, const RERect& rect, float size) const
{
    if(!_grip.IsValid()) return;
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);

    float dx = size;

    float x = roundf(rect.MiddleX());
    float y = rect.Top() + dx + 1.0;
    Draw(painter, REPoint(x,y), size);
}


void REChordDiagram::Draw(REPainter& painter, const REPoint& point, float size) const
{
    if(!_grip.IsValid()) return;
    
    painter.SetFillColor(REColor::Black);
    painter.SetStrokeColor(REColor::Black);
    
    const int maxFret = 6;
    int stringCount = _grip.StringCount();
    float dx = size;
    float dy = roundf(1.25 * size);
    
    float x = point.x;
    float y = point.y + dx + 1.0;
    float x0 = roundf(x - (float(stringCount) * dx) / 2.0);
    float x1 = roundf(x0 + (stringCount-1) * dx);
    
    float y0 = y;
    float y1 = y + (maxFret-1) * dy /*+ 0.25 * dy*/;
    
    // Delta Fret
    RERange fretRange = _grip.FretRange();
    int deltaFret = 0;
    if(fretRange.LastIndex() > 4) {
        deltaFret = 1 - fretRange.FirstIndex();
    }
    
    // Draw the Delta Fret
    if(deltaFret) {
        int df = 1 - deltaFret;
        std::ostringstream oss; oss << df;
        REPoint pt(x0 - dx - 0.50*size, y0);
        if(df >= 10) {
            pt.x -= dx/2;
        }
        painter.DrawText(oss.str(), pt, "Arial", REPainter::Bold, roundf(1.25*size), REColor::Black);
    }
    
    // Horizontal Lines
    for(int lineIndex=0; lineIndex < maxFret; ++lineIndex)
    {
        float lineY = y + (lineIndex * dy) + 0.5;
        if(lineIndex == 0 && deltaFret == 0) {
            float h = roundf(dy/8);
            painter.FillRect(RERect(x0, y0, (x1-x0+1), h));
        }
        else {
            painter.StrokeLine(REPoint(x0, lineY), REPoint(x1, lineY));
        }
    }
    
    // Vertical Lines
    for(int i=0; i<stringCount; ++i)
    {
        float lineX = x0 + i * dx + 0.5;
        painter.StrokeLine(REPoint(lineX, y0), REPoint(lineX, y1));
    }
    
    // X, O and fret numbers
    for(int i=0; i<stringCount; ++i)
    {
        float stringX = x0 + dx * (float)i;
        
        if(_grip.IsStringPlayed(i)) {
            int fret = _grip.Fret(i);
            if(fret == 0) {
                float w = roundf(0.60 * dx);
                RERect rc(stringX - w/2, y - dx, w, w);
                painter.PathBegin();
                painter.PathAddEllipseInRect(rc);
                painter.PathStroke();
            }
            else {
                fret += deltaFret;
                if(fret <= 5) {
                    float fretY = y + (fret * dy) + 0.5;
                    float w = roundf(0.60 * dx);
                    RERect rc(stringX - w/2, fretY - 0.75 * dy, w, w);
                    painter.PathBegin();
                    painter.PathAddEllipseInRect(rc);
                    painter.PathFill();
                }
            }
        }
        else {
            float w = roundf(0.60 * dx);
            float x0 = stringX - w/2;
            float x1 = x0 + w;
            float y0 = y - dx;
            float y1 = y0 + w;
            painter.StrokeLine(REPoint(x0,y0), REPoint(x1, y1));
            painter.StrokeLine(REPoint(x0,y1), REPoint(x1, y0));
        }
    }
}
