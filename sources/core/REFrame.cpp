//
//  REFrame.cpp
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REFrame.h"

REFrame::REFrame(const RERect& rc, const std::string& text, const REFontDesc& font, Reflow::TextAlignment align)
: REScoreNode(), _text(text), _font(font), _align(align)
{
    SetPosition(rc.origin);
    SetSize(rc.size);
}
