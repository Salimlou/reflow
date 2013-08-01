//
//  REPage.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#include "REPage.h"
#include "REFrame.h"

REPage::~REPage()
{
    ClearTextFrames();
}


void REPage::AddTextFrame(REFrame* frame)
{
    frame->_parent = this;
    _frames.push_back(frame);
}

void REPage::ClearTextFrames()
{
    std::for_each(_frames.begin(), _frames.end(), [](REFrame* x) {delete x;});
    _frames.clear();
}

const REFrame* REPage::TextFrame(int idx) const
{
    return (idx >= 0 && idx < _frames.size() ? _frames[idx] : nullptr);
}
