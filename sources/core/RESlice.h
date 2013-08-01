//
//  RESlice.h
//  Reflow
//
//  Created by Sebastien on 01/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESYSTEMBAR_H_
#define _RESYSTEMBAR_H_

#include "RETypes.h"
#include "REScoreNode.h"

class RESlice : public REScoreNode
{
    friend class RESystem;
    friend class REScore;
    
public:
    enum QueryColumnResult {
        Nothing = 0x00,
        OnColumn = 0x01,
        InLeadingSpace = 0x02,
        InTrailingSpace = 0x04,
        OnLeftEdgeOfColumn = 0x10,
        OnRightEdgeOfColumn = 0x20
    };
    
public:
    RESlice();
    explicit RESlice(REBarMetrics* bm);
    virtual ~RESlice();
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::SliceNode;}
    
public:
    const RESystem* System() const;
    RESystem* System();
    int Index() const {return _index;}
    
    virtual int BarCount() const {return 1;}
    int BarIndex() const;
    int LastBarIndex() const {return BarIndex() + BarCount() - 1;}
    const REBar* Bar() const;
    
    float XOffset() const;
    
    const REBarMetrics& Metrics() const;
    
    bool IsFirstInSystem() const;
    bool IsLastInSystem() const;
    
    const RESlice* NextSibling() const;
    const RESlice* PrevSibling() const;    
    
    float StretchFactor() const;
    float XOffsetOfTick(unsigned int tick) const;
    float XOffsetAtOrBefore(unsigned int tick) const;
    float XOffsetOfLastColumn() const;
    float XOffsetAfterLeadingSpace() const;
    float XOffsetBeforeTrailingSpace() const;
    QueryColumnResult QueryColumnAtX(float x, int* columnIndex, float* snapX) const;
    
    virtual bool IsMultiRest() const {return false;}
    
public:
    int _index;
    REBarMetrics* _metrics;
};

/** REMultiRestSlice
 */
class REMultiRestSlice : public RESlice
{
    friend class RESystem;
    friend class REScore;
    
public:
    REMultiRestSlice();
    explicit REMultiRestSlice(REBarMetrics* bm);
    virtual ~REMultiRestSlice();
    
public:
    virtual bool IsMultiRest() const {return true;}
    
    virtual int BarCount() const {return _repeatCount;}
    virtual void SetBarCount(int bc) {_repeatCount = bc;}
    
private:
    int _repeatCount;
};

#endif
