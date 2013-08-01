//
//  REBend.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 27/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REBend.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REPainter.h"

REBend::REBend()
: _type(Reflow::NoBend), _bentPitch(0), _releasePitch(0)
{
}

REBend::REBend(Reflow::BendType type)
: _type(type), _bentPitch(0), _releasePitch(0)
{
    
}

REBend::REBend(Reflow::BendType type, int bentPitch)
: _type(type), _bentPitch(bentPitch), _releasePitch(0)
{
}

REBend::REBend(Reflow::BendType type, int bentPitch, int releasePitch)
: _type(type), _bentPitch(bentPitch), _releasePitch(releasePitch)
{
}

void REBend::_DrawAux(REPainter& painter, const REPoint& point, const RESize& size, float initialPitch, float middlePitch, float finalPitch)
{
    bool forceDrawBlack = painter.IsForcedToBlack();
    
    float maxPitch = initialPitch;
	if(middlePitch > maxPitch) maxPitch = middlePitch;
	if(finalPitch > maxPitch) maxPitch = finalPitch;
	
	if(maxPitch == 0.0) return;
	float invMaxPitch = 1.0 / maxPitch;
	float x0 = point.x;
	float x1 = point.x + size.w;
	float y0 = point.y + 0.5;
	float h = size.h;
	REPoint p0, p1;
	
	if(initialPitch == middlePitch && middlePitch == finalPitch)		// Prebend
	{
		p0 = REPoint(x0+0.5, y0);
		p1 = REPoint(x0+0.5, y0 - h * finalPitch * invMaxPitch);
		
		if(!forceDrawBlack) { 
            painter.SetStrokeColor(REColor(0.20, 0.20, 0.20, 1.0));
            painter.SetFillColor(REColor(0.20, 0.20, 0.20, 1.0));
        }
        painter.StrokeLine(p0, p1);
		
		REPoint ap0, ap1, ap2;
        
		// Arrow goes up
		ap0 = REPoint(p1.x, p1.y);
		ap1 = REPoint(ap0.x + 2.5, p1.y + 5.5);
		ap2 = REPoint(ap0.x - 2.5, p1.y + 5.5);
		
        painter.PathBegin();
        painter.PathMoveToPoint(ap0);
        painter.PathLineToPoint(ap1);
        painter.PathLineToPoint(ap2);
        painter.PathLineToPoint(ap0);        
        painter.PathClose();
        painter.PathFill();
		return;
	}
	
	if(middlePitch == finalPitch) 
	{
		// Two point bend
		p0 = REPoint(x0, y0 - h * initialPitch * invMaxPitch);
		p1 = REPoint(x1, y0 - h * finalPitch * invMaxPitch);
	}
	else if(initialPitch == middlePitch)
	{
		// Two point bend
		p0 = REPoint(x0, y0 - h * initialPitch * invMaxPitch);
		p1 = REPoint(x1, y0 - h * finalPitch * invMaxPitch);
	}
	else {
        _DrawAux(painter, REPoint(point.x, point.y), RESize(size.w/2, size.h), initialPitch, middlePitch, middlePitch);
        _DrawAux(painter, REPoint(point.x+size.w/2, point.y), RESize(size.w/2, size.h), middlePitch, finalPitch, finalPitch);
		return;
	}
	
	{
		REPoint pmid = REPoint(0.5*(p0.x+p1.x), 0.5*(p0.y+p1.y));
		REPoint pright = REPoint(x1, p0.y);
		REPoint control1 = REPoint(x0 + 0.55*size.w, p0.y);
		REPoint control2 = REPoint(x1 - 0.15*size.w, 0.5*(pmid.y+pright.y));
		
        painter.PathBegin();
        painter.PathMoveToPoint(p0);
        painter.PathCurveToPoint(p1, control1, control2);
        
		if(!forceDrawBlack) { 
            painter.SetStrokeColor(REColor(0.20, 0.20, 0.20, 1.0));
            painter.SetFillColor(REColor(0.20, 0.20, 0.20, 1.0));
        }
		
        painter.PathStroke();
		
		// Arrow
		REPoint ap0, ap1, ap2;
		if(p0.y > p1.y) {
			// Arrow goes up
			ap0 = REPoint(p1.x, p1.y);
			ap1 = REPoint(ap0.x + 2.0, p1.y + 6.5);
			ap2 = REPoint(ap0.x - 3.5, p1.y + 5.0);
		}
		else {
			ap0 = REPoint(p1.x, p1.y);
			ap1 = REPoint(ap0.x + 2.0, p1.y - 6.5);
			ap2 = REPoint(ap0.x - 3.5, p1.y - 5.0);
		}
        painter.PathBegin();
        painter.PathMoveToPoint(ap0);
        painter.PathLineToPoint(ap1);
        painter.PathLineToPoint(ap2);
        painter.PathLineToPoint(ap0);        
        painter.PathClose();
        painter.PathFill();
	}

}

void REBend::Draw(REPainter& painter, const REPoint& point, const RESize& size) const
{
    float initialPitch = 0;
    float middlePitch = 0;
    float finalPitch = 0;
    
    // Retrieve Pitch factors
    PitchFactors(&initialPitch, &middlePitch, &finalPitch);
    
    // Draw Bend
    _DrawAux(painter, point, size, initialPitch, middlePitch, finalPitch);
}

void REBend::PitchFactors(float* initialPitch, float* middlePitch, float* finalPitch) const
{
    switch(_type)
    {
        case Reflow::Bend: {
            *initialPitch = 0;
            *middlePitch = *finalPitch = BentPitchInTones();
            break;
        }
            
        case Reflow::BendAndRelease: {
            *initialPitch = 0;
            *middlePitch = BentPitchInTones();
            *finalPitch = ReleasePitchInTones();
            break;
        }
            
        case Reflow::PreBend: {
            *initialPitch = *middlePitch = *finalPitch = BentPitchInTones();
            break;
        }
            
        case Reflow::PreBendAndRelease: {
            *initialPitch = *middlePitch = BentPitchInTones();
            *finalPitch = ReleasePitchInTones();
            break;
        }
            
        default: {
            return;
        }
    }
}

void REBend::EncodeTo(REOutputStream& coder) const
{
    coder.WriteInt8(_type);
    coder.WriteInt8(_bentPitch);
    coder.WriteInt8(_releasePitch);
}

void REBend::DecodeFrom(REInputStream& decoder)
{
    _type = decoder.ReadInt8();
    _bentPitch = decoder.ReadInt8();
    _releasePitch = decoder.ReadInt8();
}

