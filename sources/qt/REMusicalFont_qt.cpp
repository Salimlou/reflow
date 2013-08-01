#include "REMusicalFont.h"
#include "RESVGParser.h"
#include "REPainter.h"

#include <QFile>

REMusicalFont* REMusicalFont::_builtinFont = NULL;

REMusicalFont* REMusicalFont::BuiltinFont()
{
    if(_builtinFont == NULL)
    {
        QFile svgFile(":/musical-font.svg");
        if(!svgFile.open(QIODevice::ReadOnly)) {
            printf("[Reflow] Fatal error: Musical font was not found\n");
            exit(-1);
        }
        QByteArray data = svgFile.readAll();

        _builtinFont = new REMusicalFont;
        _builtinFont->LoadSVG(data.constData(), data.length());
    }
    return _builtinFont;
}

void REMusicalFont::DrawGlyph(REPainter& painter, const REMusicalGlyph* glyph, const REPoint& point, REReal size) const
{
    if(glyph == NULL) return;

    painter.Save();

    painter.Translate(point.x, point.y);
    painter.Scale(size, size);

    glyph->Fill(painter);

    painter.Restore();
}

void REMusicalFont::DrawGlyphFlipped(REPainter& painter, const REMusicalGlyph* glyph, const REPoint& point, REReal size) const
{
    /*if(glyph == NULL) return;

#ifdef REFLOW_MAC
    CGContextRef ctx = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
#else
    CGContextRef ctx = UIGraphicsGetCurrentContext();
#endif
    CGContextSaveGState(ctx);

    CGContextTranslateCTM(ctx, point.x, point.y);
    CGContextScaleCTM(ctx, size, -size);

    glyph->Fill(painter);

    CGContextRestoreGState(ctx);*/
}
