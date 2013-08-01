//
//  RETablatureStaff.h
//  Reflow
//
//  Created by Sebastien on 04/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RETABLATURESTAFF_H_
#define _RETABLATURESTAFF_H_

#include "RETypes.h"
#include "REStaff.h"

class RETablatureStaff : public REStaff
{
public:
    RETablatureStaff();
    virtual ~RETablatureStaff();

public:
    virtual unsigned int LineCount() const;
    virtual float UnitSpacing() const;
    
    virtual void ListNoteGizmos(REGizmoVector& gizmos) const;
    
protected:
    virtual void DrawSlice(REPainter& painter, int sliceIndex) const;
    
protected:
    //virtual void _RefreshMetrics();
    
    void DrawFretsOfChord(REPainter& painter, const RESlice* sbar, const REChord* chord) const;
    void DrawBeaming(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;
    
    //virtual void _DrawBeaming(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;
    virtual void _DrawStem(REPainter& painter, const RESlice* slice, const REChord* chord, const BeamCache* beam) const;
    virtual void _DrawSingleStem(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;

    void ListNoteFretGizmosOfChord(REGizmoVector& gizmos, const RESlice* sbar, const REChord* chord) const;
    
    virtual void IterateSymbolsOfSlice(int sliceIndex, const REConstSymbolAnchorOperation& op) const;
    
public:
    void CalculateBeamingCoordinates(const REChord* firstChord, const REChord* lastChord, BeamCache* beamCache);
};

#endif
