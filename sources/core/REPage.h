//
//  REPage.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#ifndef __Reflow__REPage__
#define __Reflow__REPage__

#include "REScoreNode.h"

class REPage : public REScoreNode
{
    friend class REScoreRoot;
    
public:
    virtual ~REPage();
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::PageNode;}
    
    unsigned int Number() const {return _number;}
    void SetNumber(unsigned int number) {_number = number;}
    
    void AddTextFrame(REFrame* frame);
    int TextFrameCount() const {return _frames.size();}
    void ClearTextFrames();
    const REFrame* TextFrame(int idx) const;
    
    const REFrameVector& TextFrames() const {return _frames;}
    
protected:
    uint16_t _number;
    REFrameVector _frames;
};

#endif /* defined(__Reflow__REPage__) */
