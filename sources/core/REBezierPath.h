//
//  REBezierPath.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REBezierPath_h
#define Reflow_REBezierPath_h

#include "RETypes.h"

#ifdef REFLOW_QT
#  include <QPainterPath>
#endif

class REBezierPathImpl;



/** REBezierPath class.
 */
class REBezierPath
{
    friend class REPainter;
    
public:  
    REBezierPath();
    ~REBezierPath();
    
public:
    void MoveToPoint(const REPoint& pt);
    void MoveToPoint(float x, float y);
    void LineToPoint(const REPoint& pt);
    void LineToPoint(float x, float y);
    void CurveToPoint(const REPoint& pt, const REPoint& cp1, const REPoint& cp2);
    void AddEllipseInRect(const RERect& rect);
    void Transform(const REAffineTransform& transform);
    void Close();
        
#ifdef REFLOW_QT
public:
    const QPainterPath& PainterPath() const {return _qpath;}
    
private:
    QPainterPath _qpath;
    
#else
private:
    REBezierPathImpl* const _d;
#endif
};



#endif
