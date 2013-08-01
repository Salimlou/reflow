//
//  RESymbol.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 25/04/13.
//
//

#ifndef __Reflow__RESymbol__
#define __Reflow__RESymbol__

#include "RETypes.h"
#include "RELocator.h"

struct REConstSymbolAnchor
{
    const RESymbol* symbol;
    const REStaff* staff;
    REPoint origin;
    RELocator locator;
};

typedef std::function<void(const REConstSymbolAnchor& anchor)>        REConstSymbolAnchorOperation;


/** RESymbol
 */
class RESymbol
{
public:
    virtual ~RESymbol() {}
    
    inline const REPoint& Offset() const {return _offset;}
    
    void SetOffset(const REPoint& pt) {_offset = pt;}
    
    void SetSelected(bool selected) const {_selected = selected;}
    bool IsSelected() const {return _selected;}
    
    virtual Reflow::SymbolType Type() const = 0;
    virtual RERect Bounds(float unitSpacing) const = 0;
    RERect Frame(float unitSpacing) const;
    virtual void Draw(REPainter& painter, const REPoint& origin, const REColor& color, float unitSpacing) const = 0;
    
    virtual RESymbol* Clone() const = 0;
    
protected:
    RESymbol() : _offset(0,0), _selected(false) {}
    
protected:
    REPoint _offset;
    mutable bool _selected;
};


/** RETextSymbol
 */
class RETextSymbol : public RESymbol
{
public:
    virtual Reflow::SymbolType Type() const {return Reflow::TextSymbol;}
    virtual RERect Bounds(float unitSpacing) const;    
    virtual void Draw(REPainter& painter, const REPoint& origin, const REColor& color, float unitSpacing) const;
    
    virtual RESymbol* Clone() const;
    
    void SetText(const std::string& text) {_text = text;}
    const std::string& Text() const {return _text;}
    
    void SetFont(const REFontDesc& font) {_font = font;}
    const REFontDesc& Font() const {return _font;}
    
    void SetTextAlignment(Reflow::TextAlignment align) {_align = align;}
    Reflow::TextAlignment TextAlignment() const {return _align;}
    
    void SetBounds(const RERect& rc) {_bounds = rc;}
    const RERect& Bounds() const {return _bounds;}
    
protected:
    std::string _text;
    REFontDesc _font;
    Reflow::TextAlignment _align;
    RERect _bounds;
};



/** REPickstrokeSymbol
 */
class REPickstrokeSymbol : public RESymbol
{
public:
    REPickstrokeSymbol();
    virtual ~REPickstrokeSymbol() {}
    
    virtual Reflow::SymbolType Type() const {return Reflow::PickstrokeSymbol;}
    virtual RERect Bounds(float unitSpacing) const;
    virtual void Draw(REPainter& painter, const REPoint& origin, const REColor& color, float unitSpacing) const;
    
    inline bool Up() const {return _up;}
    inline bool Down() const {return !_up;}
    
    void SetUp(bool up) {_up=up;}
    
    const char* MusicalSymbolName() const;
    const REMusicalGlyph* MusicalGlyph() const;
    
    virtual RESymbol* Clone() const;    
    
protected:
    bool _up;
};


#endif /* defined(__Reflow__RESymbol__) */
