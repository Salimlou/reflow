//
//  REChordDiagram.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 18/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REChordDiagram_h
#define Reflow_REChordDiagram_h

#include "RETypes.h"
#include "REChordName.h"
#include "REGrip.h"

class REPainter;

class REChordDiagram
{
    friend class REArchive;
    
public:
    REChordDiagram();
    REChordDiagram(const REChordDiagram& rhs);
    
    REChordDiagram& operator=(const REChordDiagram& rhs);
    
    const REChordName& ChordName() const {return _chord;}
    const REGrip& Grip() const {return _grip;}
    
    void SetChordName(const REChordName& name);
    void SetGrip(const REGrip& grip);
    
    void Draw(REPainter& painter, const RERect& rect, float size) const;
    void Draw(REPainter& painter, const REPoint& pt, float size) const;
    RERect BoundingBoxForSize(float size) const; 
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    static float HeightForSize(float size);
    
private:
    REChordName _chord;
    REGrip _grip;
};

typedef std::vector<REChordDiagram> REChordDiagramVector;

#endif
