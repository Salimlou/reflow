#include "REPainter.h"
#include "REBezierPath.h"
#include "REMusicalFont.h"

#include <QPainter>
#include <QFont>
#include <QPainterPath>
#include <QDebug>


class REPainterImpl
{
public:
    QFont batchedTextFont;
    QFontMetrics batchedFontMetrics;
    int batchedTextFlags;

    bool forcedToBlack;
    bool grayOutInactiveVoice;
    int activeVoiceIndex;
    bool drawingToScreen;

    QPainterPath path;

    REPainterImpl()
        : batchedFontMetrics(QFont())
    {
        forcedToBlack = false;
        grayOutInactiveVoice = false;
        drawingToScreen = false;
        activeVoiceIndex = 0;
        batchedTextFont = QFont();
        batchedTextFlags = 0;
    }
};

REPainter::REPainter()
    : _painter(nullptr), _d(new REPainterImpl)
{
}

REPainter::REPainter(QPainter* painter)
    : _painter(painter), _d(new REPainterImpl)
{
}

REPainter::~REPainter()
{
    delete _d;
}

void REPainter::SetGrayOutInactiveVoice(bool b)
{
    _d->grayOutInactiveVoice = b;
}

void REPainter::SetDrawingToScreen(bool b)
{
    _d->drawingToScreen = b;
}

void REPainter::SetForcedToBlack(bool b)
{
    _d->forcedToBlack = b;
}

void REPainter::SetStrokeColor(const REColor& color)
{
    if(!_painter) return;
    QPen pen = _painter->pen();
    pen.setColor(color.ToQColor());
    _painter->setPen(pen);
}

void REPainter::SetFillColor(const REColor& color)
{
    if(!_painter) return;
    _painter->setBrush(QBrush(color.ToQColor()));
}

void REPainter::StrokePath(const REBezierPath& path)
{
    if(!_painter) return;
    _painter->strokePath(path.PainterPath(), _painter->pen());
}

void REPainter::FillPath(const REBezierPath& path)
{
    if(!_painter) return;
    _painter->fillPath(path.PainterPath(), _painter->brush());
}


void REPainter::PathBegin()
{
    _d->path = QPainterPath();
}

void REPainter::PathClose()
{
    _d->path.closeSubpath();
}

void REPainter::PathStroke()
{
    if(!_painter) return;
    _painter->strokePath(_d->path, _painter->pen());
}

void REPainter::PathFill()
{
    if(!_painter) return;
    _painter->fillPath(_d->path, _painter->brush());
}

void REPainter::PathMoveToPoint(const REPoint &pt)
{
    _d->path.moveTo(pt.ToQPointF());
}

void REPainter::PathMoveToPoint(float x, float y)
{
    _d->path.moveTo(x, y);
}

void REPainter::PathLineToPoint(const REPoint& pt)
{
    _d->path.lineTo(pt.ToQPointF());
}

void REPainter::PathLineToPoint(float x, float y)
{
    _d->path.lineTo(x, y);
}

void REPainter::PathCurveToPoint(const REPoint& pt, const REPoint& cp1, const REPoint& cp2)
{
    _d->path.cubicTo(cp1.ToQPointF(), cp2.ToQPointF(), pt.ToQPointF());
}

void REPainter::PathQuadCurveToPoint(const REPoint& pt, const REPoint& cp)
{
    _d->path.quadTo(cp.ToQPointF(), pt.ToQPointF());
}

void REPainter::PathAddEllipseInRect(const RERect& rect)
{
    _d->path.addEllipse(rect.ToQRectF());
}

void REPainter::StrokeLine(const REPoint& pt1, const REPoint& pt2)
{
    if(!_painter) return;
    _painter->drawLine(pt1.ToQPointF(), pt2.ToQPointF());
}

void REPainter::SetLineDash(const REReal* lengths, int count, float phase)
{
    if(!_painter) return;
    QPen pen = _painter->pen();

    QVector<qreal> pattern;
    for(int i=0; i<count; ++i) pattern << lengths[i];

    pen.setDashPattern(pattern);
    _painter->setPen(pen);
}


void REPainter::StrokeRect(const RERect& rc)
{
    if(!_painter) return;
    _painter->drawRect(rc.ToQRectF());
}

void REPainter::StrokeRect(const RERect& rc, float lineWidth)
{
    if(!_painter) return;
    QPen pen = _painter->pen();
    QBrush brush = _painter->brush();
    float oldLineWidth = pen.widthF();

    pen.setWidthF(lineWidth);
    _painter->setPen(pen);
    _painter->setBrush(Qt::NoBrush);
    _painter->drawRect(rc.ToQRectF());

    pen.setWidthF(oldLineWidth);
    _painter->setPen(pen);
    _painter->setBrush(brush);
}

void REPainter::FillRect(const RERect& rc)
{
    if(!_painter) return;
    _painter->fillRect(rc.ToQRectF(), _painter->brush());
}


void REPainter::Save()
{
    if(!_painter) return;
    _painter->save();
}

void REPainter::Restore()
{
    if(!_painter) return;
    _painter->restore();
}

void REPainter::Translate(float dx, float dy)
{
    if(!_painter) return;
    _painter->translate(dx, dy);
}

void REPainter::Scale(float sx, float sy)
{
    if(!_painter) return;
    _painter->scale(sx, sy);
}


void REPainter::DrawText(const std::string& textUTF8, const REPoint& pt, const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color)
{
    if(!_painter) return;
    QFont::Weight weight = (flags & REPainter::Bold ? QFont::Bold : QFont::Normal);
    QFont font(QString::fromStdString(fontNameUTF8), size, weight, flags & REPainter::Italic);
	font.setPixelSize(size);
    QFontMetrics fm(font);

    _painter->setFont(font);
    SetStrokeColor(color);
    int qflags = Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine;

    _painter->drawText(QPointF(pt.x, pt.y + fm.ascent()), QString::fromStdString(textUTF8));
}

void REPainter::DrawTextInRect(const std::string& textUTF8, const RERect& rect, const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color)
{
    if(!_painter) return;
    QFont::Weight weight = (flags & REPainter::Bold ? QFont::Bold : QFont::Normal);
    QFont font(QString::fromStdString(fontNameUTF8), size, weight, flags & REPainter::Italic);
	font.setPixelSize(size);
    _painter->setFont(font);
    SetStrokeColor(color);

    int qAlignFlags = 0;
    if(flags & REPainter::LeftAligned) qAlignFlags |= Qt::AlignLeft;
    else if(flags & REPainter::RightAligned) qAlignFlags  |= Qt::AlignRight;
    else qAlignFlags  |= Qt::AlignHCenter;

    int qflags = qAlignFlags | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine;

    QRectF rc = rect.ToQRectF();
    _painter->setFont(font);
    _painter->drawText(rc, qflags, QString::fromStdString(textUTF8));
}


RESize REPainter::SizeOfText(const std::string& textUTF8, const std::string& fontNameUTF8, unsigned int flags, float size)
{
    QFont::Weight weight = (flags & REPainter::Bold ? QFont::Bold : QFont::Normal);
    QFont font(QString::fromStdString(fontNameUTF8), size, weight, flags & REPainter::Italic);
	font.setPixelSize(size);
    QFontMetrics fm(font);
    int qflags = Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine;

    return RESize(fm.size(qflags, QString::fromStdString(textUTF8)));
}


void REPainter::BeginTextBatched(const std::string& fontNameUTF8, unsigned int flags, float size, const REColor& color)
{
    if(!_painter) return;
    QFont::Weight weight = (flags & REPainter::Bold ? QFont::Bold : QFont::Normal);
    QFont font = QFont(QString::fromStdString(fontNameUTF8), size, weight, flags & REPainter::Italic);
	font.setPixelSize(size);
	
	_d->batchedTextFont = font;
    _d->batchedFontMetrics = QFontMetrics(_d->batchedTextFont);
    _painter->setFont(_d->batchedTextFont);

    SetStrokeColor(color);

    _d->batchedTextFlags = Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine;
}

void REPainter::DrawTextBatched(const std::string& textUTF8, const REPoint& pt)
{
    if(!_painter) return;
    _painter->setFont(_d->batchedTextFont);
    _painter->drawText(QPointF(pt.x, pt.y + _d->batchedFontMetrics.ascent()), QString::fromStdString(textUTF8));
}

void REPainter::EndTextBatched()
{

}

void REPainter::DrawMusicSymbol(const char* symbol, const REPoint& pt, float size)
{
    DrawMusicSymbol(symbol, pt.x, pt.y, size);
}

void REPainter::DrawMusicSymbol(const char* symbol, float x, float y, float size)
{
    if(!_painter) return;
    REMusicalFont* font = REMusicalFont::BuiltinFont();
    assert(font != NULL);

    REMusicalGlyph* glyph = font->GlyphNamed(symbol);
    if(glyph == NULL) return;

    font->DrawGlyph(*this, glyph, REPoint(x,y), size);
}

void REPainter::DrawMusicSymbolFlipped(const char* symbol, float x, float y, float size)
{
    if(!_painter) return;
    REMusicalFont* font = REMusicalFont::BuiltinFont();
    assert(font != NULL);

    REMusicalGlyph* glyph = font->GlyphNamed(symbol);
    if(glyph == NULL) return;

    font->DrawGlyphFlipped(*this, glyph, REPoint(x,y), size);
}

RERect REPainter::BoundingBoxOfMusicSymbol(const char* symbol, float size)
{
    REMusicalFont* font = REMusicalFont::BuiltinFont();
    assert(font != NULL);

    REMusicalGlyph* glyph = font->GlyphNamed(symbol);
    if(glyph == NULL) return RERect(0,0,0,0);

    return glyph->BoundingBoxForSize(size);
}

void REPainter::FillQuad(const REPoint* points)
{
    if(!_painter) return;
	QPainterPath path;
	path.moveTo(points[0].x, points[0].y);
	path.lineTo(points[1].x, points[1].y);
	path.lineTo(points[2].x, points[2].y);
	path.lineTo(points[3].x, points[3].y);
	path.lineTo(points[0].x, points[0].y);
    _painter->fillPath(path, _painter->brush());
}

void REPainter::StrokeHorizontalLine(float x0, float x1, float y)
{
    StrokeLine(REPoint(x0,y), REPoint(x1,y));
}

void REPainter::StrokeVerticalLine(float x, float y0, float y1)
{
    StrokeLine(REPoint(x, y0), REPoint(x, y1));
}

int REPainter::ActiveVoiceIndex() const
{
    return _d->activeVoiceIndex;
}

void REPainter::SetActiveVoiceIndex(int index)
{
    _d->activeVoiceIndex = index;
}


bool REPainter::ShouldGrayOutInactiveVoice() const
{
    return _d->grayOutInactiveVoice;
}

bool REPainter::IsForcedToBlack() const
{
    return false;
}

bool REPainter::IsDrawingToScreen() const
{
    return true;
}
