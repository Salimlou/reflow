//
//  REFrame.h
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REFRAME_H_
#define _REFRAME_H_

#include "REScoreNode.h"

class REFrame : public REScoreNode
{
    friend class REPage;
    
public:
    REFrame(const RERect& rc, const std::string& text, const REFontDesc& font, Reflow::TextAlignment align);
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::FrameNode;}
    
    void SetText(const std::string& text) {_text = text;}
    const std::string& Text() const {return _text;}
    
    void SetFont(const REFontDesc& font) {_font = font;}
    const REFontDesc& Font() const {return _font;}
    
    void SetTextAlignment(Reflow::TextAlignment align) {_align = align;}
    Reflow::TextAlignment TextAlignment() const {return _align;}
    
protected:
    std::string _text;
    REFontDesc _font;
    Reflow::TextAlignment _align;
};


#endif
