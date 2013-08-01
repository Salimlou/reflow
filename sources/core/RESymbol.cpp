//
//  RESymbol.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 25/04/13.
//
//

#include "RESymbol.h"
#include "REPainter.h"
#include "REMusicalFont.h"

RERect RESymbol::Frame(float unitSpacing) const
{
    return Bounds(unitSpacing).Translated(Offset());
}

#pragma mark -
#pragma mark RETextSymbol
RERect RETextSymbol::Bounds(float unitSpacing) const
{
    return _bounds;
}

void RETextSymbol::Draw(REPainter& painter, const REPoint& origin, const REColor& color, float unitSpacing) const
{
//    painter.DrawTextInRect(_text, Bounds(unitSpacing).Translated(origin), "Times New Roman", 0, 10.0, color);
    painter.DrawText(_text, Bounds(unitSpacing).Translated(origin).origin, "Times New Roman", 0, 10.0, color);
}

RESymbol* RETextSymbol::Clone() const
{
    RETextSymbol* symbol = new RETextSymbol();
    symbol->_offset = _offset;
    symbol->_selected = _selected;
    return symbol;
}


#pragma mark -
#pragma mark REPickstrokeSymbol
REPickstrokeSymbol::REPickstrokeSymbol()
: _up(false)
{
    
}

RERect REPickstrokeSymbol::Bounds(float unitSpacing) const
{
    const REMusicalGlyph* glyph = MusicalGlyph();
    return glyph ? glyph->BoundingBoxForSize(unitSpacing) : RERect(0,0,0,0);
}

const REMusicalGlyph* REPickstrokeSymbol::MusicalGlyph() const
{
    REMusicalFont* font = REMusicalFont::BuiltinFont();
    assert(font != NULL);
    
    return font->GlyphNamed(MusicalSymbolName());
}

void REPickstrokeSymbol::Draw(REPainter &painter, const REPoint &origin, const REColor &color, float unitSpacing) const
{
    painter.SetFillColor(color);
    
    painter.DrawMusicSymbol(MusicalSymbolName(), origin, unitSpacing);
}

const char* REPickstrokeSymbol::MusicalSymbolName() const
{
    return _up ? "upstroke" : "downstroke";
}

RESymbol* REPickstrokeSymbol::Clone() const
{
    REPickstrokeSymbol* symbol = new REPickstrokeSymbol();
    symbol->_offset = _offset;
    symbol->_selected = _selected;
    symbol->_up = _up;
    return symbol;
}
