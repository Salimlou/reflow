//
//  REStyle.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 28/06/13.
//
//

#include "REStyle.h"
#include "RESlice.h"
#include "RESystem.h"
#include "REBar.h"
#include "REScore.h"
#include "RESong.h"
#include "REBarMetrics.h"
#include "REViewport.h"
#include "REPainter.h"

#include "REInputStream.h"
#include "REOutputStream.h"

REStyleCollection* REStyleCollection::_instance = nullptr;

REStyleCollection& REStyleCollection::Instance()
{
    if(_instance == nullptr) {
        _instance = new REStyleCollection;
    }
    return *_instance;
}

REStyleCollection::REStyleCollection()
{
    
}

void REStyleCollection::Register(const std::string& identifier, const REStyleCreator& creator)
{
    _creators[identifier] = creator;
}


REStyle* REStyleCollection::CreateStyleWithIdentifier(const std::string& identifier)
{
    auto it = _creators.find(identifier);
    return it != _creators.end() ? it->second() : new REStyle;
}





REStyle* REStyle::_defaultReflowStyle = nullptr;

REStyle::REStyle()
{
    _chordNameFontSize = 9.0;
    _leftMarginOfFirstSystem = 65.0;
    _leftMarginOfOtherSystems = 35.0;
}

REStyle::~REStyle()
{
    
}

REStyle* REStyle::Clone() const
{
    REStyle* style = new REStyle;
    
    REBufferOutputStream out;
    EncodeTo(out);
    
    REConstBufferInputStream in(out.Data(), out.Size());
    style->DecodeFrom(in);
    return style;
}

std::string REStyle::Identifier() const
{
    return "default";
}

void REStyle::EncodeTo(REOutputStream& coder) const
{
    coder.WriteFloat(_chordNameFontSize);
    coder.WriteFloat(_leftMarginOfFirstSystem);
    coder.WriteFloat(_leftMarginOfOtherSystems);
}

void REStyle::DecodeFrom(REInputStream& decoder)
{
    _chordNameFontSize = decoder.ReadFloat();
    _leftMarginOfFirstSystem = decoder.ReadFloat();
    _leftMarginOfOtherSystems = decoder.ReadFloat();
}

const REStyle* REStyle::DefaultReflowStyle()
{
    if(_defaultReflowStyle == nullptr) {
        auto style = new REStyle;
        style->_chordNameFontSize = 9.0;
        style->_leftMarginOfFirstSystem = 65.0;
        style->_leftMarginOfOtherSystems = 35.0;
        _defaultReflowStyle = style;
    }
    return _defaultReflowStyle;
}

void REStyle::DrawChordNamesOfSlice(REPainter& painter, const RESlice* slice, float yOffset) const
{
    const REBar* bar = slice->Bar();
    
    for(int i=0; i<bar->ChordNameCount(); ++i)
    {
        const REChordName* chordName = bar->ChordNameAtIndex(i);
        std::string txt = chordName->ToString();
        int tick = bar->TickOfChordNameAtIndex(i);
        float x = slice->XOffsetOfTick(tick);
        
        float fontSize = ChordNameFontSize();
        const char* fontName = "Helvetica";
        REPoint pt = REPoint(x + 0.5, yOffset + 0.5);
        
        // Text metrics
        RESize sz = painter.SizeOfText(txt, fontName, 0, fontSize);
        painter.DrawText(txt, REPoint(pt.x - sz.w/2, pt.y + 0.5), fontName, 0, fontSize, REColor(0,0,0));
    }
}

