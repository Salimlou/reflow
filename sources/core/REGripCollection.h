//
//  REGripCollection.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REGripCollection_h
#define Reflow_REGripCollection_h

#include "RETypes.h"
#include "REGrip.h"
#include "REPitchClass.h"
#include "REXMLParser.h"


class REGripCollectionEntry
{
public:
    REGripCollectionEntry() {}
    REGripCollectionEntry(const REGripCollectionEntry& rhs) {*this = rhs;}
    
    REGripCollectionEntry& operator=(const REGripCollectionEntry& rhs){
        formulaId = rhs.formulaId;
        chromaticStep = rhs.chromaticStep;
        grip = rhs.grip;
        return *this;
    }
    
    std::string formulaId;
    int chromaticStep;
    REGrip grip;
};

typedef std::vector<REGripCollectionEntry> REGripCollectionEntryVector;



class REGripCollection : private REXMLParserDelegate
{
public:
    REGripCollection();
    virtual ~REGripCollection();
    
    bool LoadGripsXML(const char* data, int length);
    
    void FetchGrips(REGripVector& grips, int chromaticStep, const std::string& formulaId) const;
    
    void Clear();
    
    static REGripCollection* BuiltinCollection();
    
private:
    REGripCollectionEntryVector _grips;
    static REGripCollection* _builtinCollection;
    
private:
    virtual void OnStartDocument(const REXMLParser& parser) {}
    
    virtual void OnEndDocument(const REXMLParser& parser) {}
    
    virtual void OnStartElement(const REXMLParser& parser, 
                                const std::string& name, 
                                const std::string& namespaceURI, 
                                const std::string &qName, 
                                const REXMLAttributeMap& attributes);
    
    virtual void OnEndElement(const REXMLParser& parser, 
                              const std::string& name, 
                              const std::string& namespaceURI, 
                              const std::string &qName) {}
};

#endif
