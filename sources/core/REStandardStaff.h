//
//  REStandardStaff.h
//  ReflowIphone
//
//  Created by Sebastien on 11/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESTANDARDSTAFF_H_
#define _RESTANDARDSTAFF_H_

#include "RETypes.h"
#include "REStaff.h"

class REStandardStaff : public REStaff
{
    friend class RESystem;
    friend class RELayout;
    friend class REScore;
    
public:
    enum GrandStaffHand {
        RightHand = 0,
        LeftHand = 1
    };
    
public:
    REStandardStaff();
    virtual ~REStandardStaff();
	
public:
    virtual unsigned int LineCount() const;
    virtual float UnitSpacing() const;
    
    virtual int FirstVoiceIndex() const;
    virtual int LastVoiceIndex() const;
    
    virtual void ListNoteGizmos(REGizmoVector& gizmos) const;
    
    GrandStaffHand Hand() const {return _hand;}
    
    void IterateSymbolsOfSlice(int sliceIndex, const REConstSymbolAnchorOperation& op) const;
    
protected:
    virtual void DrawSlice(REPainter& painter, int sliceIndex) const;
    
    virtual void DrawBarlinesOfSlice(REPainter& painter, int sliceIndex) const;
    
protected:
    virtual void _DrawSingleStem(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;
    
protected:
    void DrawNoteHeadsOfChord(REPainter& painter, const RESlice* sbar, const REChord* chord, bool transposed=false) const;
    void DrawNoteHead(REPainter& painter, const RENote* note, float x, float y, float headX, float psize, const char* noteHead, bool transposed, bool hasStackedSeconds, float &minX) const;
    
    void ListNoteHeadGizmosOfChord(REGizmoVector& gizmos, const RESlice* sbar, const REChord* chord, bool transposed=false) const;
    
protected:
    GrandStaffHand _hand;
};

#endif
