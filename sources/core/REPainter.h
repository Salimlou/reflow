//
//  REPainter.h
//  Reflow
//
//  Created by Sebastien on 03/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REPAINTER_H_
#define _REPAINTER_H_

#include "RETypes.h"

#ifdef REFLOW_QT
class QPainter;
#endif


class REPainterImpl;


/** REPainter class.
 */
class REPainter
{
public:
    enum DrawTextFlags {
        Default = 0x00,
        Bold = 0x01,
        Italic = 0x02,
        
        LeftAligned = 0x10,
        RightAligned = 0x20
    };

public:
    virtual bool IsGraphicsPainter() const {return false;}
    
    virtual void FillRect(const RERect& rc);    
    virtual void FillQuad(const REPoint* points);
    
    virtual void StrokeLine(const REPoint& pt1, const REPoint& pt2);
    virtual void StrokeHorizontalLine(float x0, float x1, float y);
    virtual void StrokeVerticalLine(float x, float y0, float y1);
    
    virtual void SetStrokeColor(const REColor& color);
    virtual void SetFillColor(const REColor& color);
    
    virtual void DrawMusicSymbol(const char* symbol, float x, float y, float size);
    
    virtual void Save();
    virtual void Restore();
    
    virtual void Translate(float dx, float dy);
    virtual void Scale(float sx, float sy);
    
    virtual void Flush() {}
    
    virtual void StrokePath(const REBezierPath& path);
    virtual void FillPath(const REBezierPath& path);
    
    virtual void PathBegin();
    virtual void PathClose();
    virtual void PathStroke();
    virtual void PathFill();
    virtual void PathMoveToPoint(const REPoint &pt);
    virtual void PathMoveToPoint(float x, float y);
    virtual void PathLineToPoint(const REPoint& pt);
    virtual void PathLineToPoint(float x, float y);
    virtual void PathCurveToPoint(const REPoint& pt, const REPoint& cp1, const REPoint& cp2);
    virtual void PathQuadCurveToPoint(const REPoint& pt, const REPoint& cp);
    virtual void PathAddEllipseInRect(const RERect& rect);
    
    virtual void SetLineDash(const REReal* lengths, int count, float phase);
    
    virtual void StrokeRect(const RERect& rc);
    virtual void StrokeRect(const RERect& rc, float lineWidth);
    
    virtual void DrawText(const std::string& textUTF8, const REPoint& pt, const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color);    
    virtual void DrawTextInRect(const std::string& textUTF8, const RERect& rect, const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color);
    
    virtual RESize SizeOfText(const std::string& textUTF8, const std::string& fontNameUTF8, unsigned int flags, float size);
    
    virtual void BeginTextBatched(const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color);
    virtual void DrawTextBatched(const std::string& textUTF8, const REPoint& pt);
    virtual void EndTextBatched();
    
    virtual void DrawMusicSymbol(const char* symbol, const REPoint& pt, float size);
    virtual void DrawMusicSymbolFlipped(const char* symbol, float x, float y, float size);
    
public:
    RERect BoundingBoxOfMusicSymbol(const char* symbol, float size);
    
    int ActiveVoiceIndex() const;
    void SetActiveVoiceIndex(int index);
    
    void SetInvertedColors(bool ic);
    bool InvertedColors() const;
    
    bool ShouldGrayOutInactiveVoice() const;
    bool IsForcedToBlack() const;
    bool IsDrawingToScreen() const;
    
private:
    REPainterImpl* const _d;
    
#ifdef REFLOW_QT
public:
    REPainter();
    REPainter(QPainter* painter);
    ~REPainter();

private:
    QPainter* _painter;
#else
public:
    REPainter();
    ~REPainter();
#endif
    
#ifdef REFLOW_IOS
    REPainter(void* contextRef);
    
    void SetDrawingToScreen(bool drawingToScreen);
#endif
};

#endif
