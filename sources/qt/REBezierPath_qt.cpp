#include "REBezierPath.h"

#include <QPainter>
#include <QPainterPath>


REBezierPath::REBezierPath()
{
}

REBezierPath::~REBezierPath()
{
}

void REBezierPath::MoveToPoint(const REPoint& pt)
{
    _qpath.moveTo(pt.x, pt.y);
}

void REBezierPath::MoveToPoint(float x, float y)
{
    _qpath.moveTo(x, y);
}

void REBezierPath::LineToPoint(const REPoint& pt)
{
    _qpath.lineTo(pt.x, pt.y);
}

void REBezierPath::LineToPoint(float x, float y)
{
    _qpath.lineTo(x, y);
}

void REBezierPath::CurveToPoint(const REPoint& pt, const REPoint& cp1, const REPoint& cp2)
{
    _qpath.cubicTo(cp1.ToQPointF(), cp2.ToQPointF(), pt.ToQPointF());
}

void REBezierPath::AddEllipseInRect(const RERect& rect)
{
    _qpath.addEllipse(rect.ToQRectF());
}

void REBezierPath::Transform(const REAffineTransform& transform)
{
    _qpath = _qpath * transform.ToQTransform();
}

void REBezierPath::Close()
{
    _qpath.closeSubpath();
}

