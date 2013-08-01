//
//  REMusicalFont.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 10/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REMusicalFont.h"
#include "REPainter.h"
#include "REFunctions.h"
#include "RESVGParser.h"
#include "REBezierPath.h"

#include <cassert>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

REMusicalGlyph::REMusicalGlyph()
{
}

REMusicalGlyph::~REMusicalGlyph()
{
    Clear();
}

void REMusicalGlyph::Fill(REPainter& painter) const
{
    REBezierPathVector::const_iterator it = _paths.begin();
    for(; it != _paths.end(); ++it) {
        painter.FillPath(**it);
    }
}

void REMusicalGlyph::Stroke(REPainter& painter) const
{
    REBezierPathVector::const_iterator it = _paths.begin();
    for(; it != _paths.end(); ++it) {
        painter.StrokePath(**it);
    }    
}

RERect REMusicalGlyph::BoundingBoxForSize(float size) const
{
    RERect rc = _boundingBox;
    rc.origin.x *= size;
    rc.origin.y *= size;
    rc.size.w *= size;
    rc.size.h *= size;
    return rc;
}

void REMusicalGlyph::Clear()
{
    REBezierPathVector::const_iterator it = _paths.begin();
    for(; it != _paths.end(); ++it) {
        delete *it;
    }
    _paths.clear();
}








REMusicalFont::REMusicalFont()
: _fontScale(8.0)
{
    
}
REMusicalFont::~REMusicalFont()
{
    
}

bool REMusicalFont::LoadSVG(const char* data, int length)
{
    REXMLParser parser;
    parser.SetDelegate(this);
    return parser.ParseData(data, length);
}

void REMusicalFont::QuadForDrawGlyph(const REMusicalGlyph* glyph, const REPoint& point, REReal size, REPoint* vertexCoords, REPoint* textureCoords) const
{
    float invSourceWidth = 1.0 / _viewBox.Width();
    float invSourceHeight = 1.0 / _viewBox.Height();
    float invFontScale = 1.0 / _fontScale;
    
    float u0 = glyph->_sourceBox.origin.x * invSourceWidth;
    float u1 = u0 + glyph->_sourceBox.size.w * invSourceWidth;
    float v0 = glyph->_sourceBox.origin.y * invSourceHeight;
    float v1 = v0 + glyph->_sourceBox.size.h * invSourceHeight;
    
    // TODO (use anchor)
    float dx = (glyph->_sourceAnchor.x - glyph->_sourceBox.origin.x) * invFontScale * size;
    float dy = (glyph->_sourceAnchor.y - glyph->_sourceBox.origin.y) * invFontScale * size;
    float x0 = point.x - dx;
    float y0 = point.y - dy;
    float x1 = x0 + glyph->_sourceBox.size.w * invFontScale * size;
    float y1 = y0 + glyph->_sourceBox.size.h * invFontScale * size;
    
    vertexCoords[0] = REPoint(x0, y0);
    vertexCoords[1] = REPoint(x1, y0);
    vertexCoords[2] = REPoint(x1, y1);
    vertexCoords[3] = REPoint(x0, y1);
    
    textureCoords[0] = REPoint(u0, v0);
    textureCoords[1] = REPoint(u1, v0);
    textureCoords[2] = REPoint(u1, v1);
    textureCoords[3] = REPoint(u0, v1);
}

REMusicalGlyph* REMusicalFont::GlyphNamed(const char* name)
{
    REMusicalGlyphMap::const_iterator it = _glyphs.find(name);
    if(it != _glyphs.end()) {
        return it->second;
    }
    return NULL;
}

void REMusicalFont::OnStartDocument(const REXMLParser& parser)
{
    _currentParsedGlyph = NULL;
}

void REMusicalFont::OnEndDocument(const REXMLParser& parser)
{
    
}

void REMusicalFont::OnStartElement(const REXMLParser& parser, 
                                   const std::string& name, 
                                   const std::string& namespaceURI, 
                                   const std::string &qName, 
                                   const REXMLAttributeMap& attributes)
{
    // Find Id
    std::string idAttr = "";
    REXMLAttributeMap::const_iterator idIt = attributes.find("id");
    if(idIt != attributes.end()) {
        idAttr = idIt->second;
    }
    _glyphNameStack.push_back(idAttr);
    
    // SVG Root Element
    if(name == "svg")
    {
        REXMLAttributeMap::const_iterator viewBoxIt = attributes.find("viewBox");
        if(viewBoxIt != attributes.end())
        {
            std::istringstream iss(viewBoxIt->second);
            double x, y, w, h;
            iss >> x >> y >> w >> h;
            _viewBox = RERect(x,y,w,h);
        }
        else {
            _viewBox = RERect(0, 0, 0, 0);
        }
    }
    
    // symbol element
    if(boost::ends_with(idAttr, "-symbol"))
    {
        std::string glyphName = idAttr.substr(0, idAttr.length()-7);
        _currentParsedGlyph = new REMusicalGlyph(glyphName);
        _currentAnchor = REPoint(0,0);
        _currentBoundingBox = RERect(0,0,0,0);
        _currentPaths.clear();
    }
    
    // anchor element
    else if(boost::ends_with(idAttr, "-anchor") && _currentParsedGlyph != NULL && name == "rect")
    {
        double x=0, y=0, w=0, h=0;
        REXMLAttributeMap::const_iterator it = attributes.end();
        if((it = attributes.find("x")) != attributes.end()) {x = atof(it->second.c_str());}
        if((it = attributes.find("y")) != attributes.end()) {y = atof(it->second.c_str());}
        if((it = attributes.find("width")) != attributes.end()) {w = atof(it->second.c_str());}
        if((it = attributes.find("height")) != attributes.end()) {h = atof(it->second.c_str());}
        _currentAnchor = REPoint(x + w/2, y + h/2);
    }

    // bbox element
    else if(boost::ends_with(idAttr, "-bbox") && _currentParsedGlyph != NULL && name == "rect")
    {
        double x=0, y=0, w=0, h=0;
        REXMLAttributeMap::const_iterator it = attributes.end();
        if((it = attributes.find("x")) != attributes.end()) {x = atof(it->second.c_str());}
        if((it = attributes.find("y")) != attributes.end()) {y = atof(it->second.c_str());}
        if((it = attributes.find("width")) != attributes.end()) {w = atof(it->second.c_str());}
        if((it = attributes.find("height")) != attributes.end()) {h = atof(it->second.c_str());}
        _currentBoundingBox = RERect(x,y,w,h);
    }

    //  element
    else if(boost::ends_with(idAttr, "-body") && _currentParsedGlyph != NULL)
    {
        if(name == "path") 
        {
            std::string d = "";
            REXMLAttributeMap::const_iterator it = attributes.find("d");
            if(it != attributes.end()) {d = it->second;}
            
            if(!d.empty())
            {
                RESVGParser parser;
                _currentPaths.push_back(parser.ParsePathData(d, _viewBox));
            }
        }
    }

    // path Element
    else if(name == "path" && _currentParsedGlyph != NULL)
    {
        std::string d = "";
        REXMLAttributeMap::const_iterator it = attributes.find("d");
        if(it != attributes.end()) {d = it->second;}
        
        if(!d.empty())
        {
            RESVGParser parser;
            _currentPaths.push_back(parser.ParsePathData(d, _viewBox));
        }
    }
}

void REMusicalFont::OnEndElement(const REXMLParser& parser, 
                                 const std::string& name, 
                                 const std::string& namespaceURI, 
                                 const std::string &qName)
{
    assert(!_glyphNameStack.empty());
    std::string idAttr = _glyphNameStack.back();
    _glyphNameStack.pop_back();
    
    // symbol element
    if(boost::ends_with(idAttr, "-symbol"))
    {   
        // Transform
        REAffineTransform b = REAffineTransform::Scaling(1.0 / _fontScale, 1.0 / _fontScale);
        REAffineTransform a = REAffineTransform::Translation(-_currentAnchor.x, -_currentAnchor.y);
        REAffineTransform transform = REAffineTransform::Multiply(a, b);
        
        // Translate all paths
        BOOST_FOREACH(REBezierPath* path, _currentPaths) {
            if(path) {
                path->Transform(transform);
                _currentParsedGlyph->AddBezierPath(path);
            }
        }

        _currentParsedGlyph->_sourceBox = _currentBoundingBox;
        _currentParsedGlyph->_sourceAnchor = _currentAnchor;
        
        // Translate and scale bounding box
        _currentBoundingBox.origin.x -= _currentAnchor.x;
        _currentBoundingBox.origin.y -= _currentAnchor.y;
        _currentBoundingBox.origin.x /= _fontScale;
        _currentBoundingBox.origin.y /= _fontScale;
        _currentBoundingBox.size.w   /= _fontScale;
        _currentBoundingBox.size.h   /= _fontScale;        
        _currentParsedGlyph->SetBoundingBox(_currentBoundingBox);
        
        // Add glyph to our glyph map
        _glyphs[_currentParsedGlyph->Name()] = _currentParsedGlyph;
        _currentParsedGlyph = NULL;
        _currentPaths.clear();
    }
}
