//
//  REMusicalFont.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 30/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef _REMUSICALFONT_H_
#define _REMUSICALFONT_H_

#include "RETypes.h"
#include "REXMLParser.h"

#include <deque>

/** REMusicalGlyph
 */
class REMusicalGlyph
{
    friend class REMusicalFont;
    
public:
    REMusicalGlyph();
    REMusicalGlyph(const std::string& name) : _name(name) {}
    virtual ~REMusicalGlyph();
    
    void SetName(const std::string& name) {_name = name;}
    const std::string& Name() const {return _name;}
    
    void SetBoundingBox(const RERect& bbox) {_boundingBox = bbox;}
    const RERect& BoundingBox() const {return _boundingBox;}
    
    int BezierPathCount() const {return _paths.size();}
    void AddBezierPath(REBezierPath* path) {_paths.push_back(path);}
    
    void Fill(REPainter& painter) const;
    void Stroke(REPainter& painter) const;
    
    RERect BoundingBoxForSize(float size) const;
    
    void Clear();
    
private:
    std::string _name;
    RERect _boundingBox;
    REBezierPathVector _paths;
    
    RERect _sourceBox;
    REPoint _sourceAnchor;
};

typedef std::map<std::string, REMusicalGlyph*> REMusicalGlyphMap;



/** REMusicalFont
 */
class REMusicalFont : private REXMLParserDelegate
{
public:
    REMusicalFont();
    virtual ~REMusicalFont();
    
    bool LoadSVG(const char* data, int length);

    void DrawGlyph(REPainter& painter, const REMusicalGlyph* glyph, const REPoint& point, REReal size) const;
    void DrawGlyphFlipped(REPainter& painter, const REMusicalGlyph* glyph, const REPoint& point, REReal size) const;
    
    void QuadForDrawGlyph(const REMusicalGlyph* glyph, const REPoint& point, REReal size, REPoint* vertexCoords, REPoint* textureCoords) const;
    
    REMusicalGlyph* GlyphNamed(const char* name);
    
    static REMusicalFont* BuiltinFont();
    
private:
    REMusicalGlyphMap _glyphs;
    float _fontScale;
    RERect _viewBox;
    static REMusicalFont* _builtinFont;
    
private:
    virtual void OnStartDocument(const REXMLParser& parser);
    
    virtual void OnEndDocument(const REXMLParser& parser);
    
    virtual void OnStartElement(const REXMLParser& parser, 
                                const std::string& name, 
                                const std::string& namespaceURI, 
                                const std::string &qName, 
                                const REXMLAttributeMap& attributes);
    
    virtual void OnEndElement(const REXMLParser& parser, 
                              const std::string& name, 
                              const std::string& namespaceURI, 
                              const std::string &qName);
    
    REMusicalGlyph* _currentParsedGlyph;
    RERect _currentBoundingBox;
    REPoint _currentAnchor;
    std::vector<REBezierPath*> _currentPaths;
    std::deque<std::string> _glyphNameStack;
};


#endif
