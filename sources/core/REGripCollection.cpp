//
//  REGripCollection.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REGripCollection.h"
#include "REGrip.h"

#include <cassert>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

REGripCollection::REGripCollection()
{
}

REGripCollection::~REGripCollection()
{
    Clear();
}

void REGripCollection::Clear()
{
    _grips.clear();
}


bool REGripCollection::LoadGripsXML(const char *data, int length)
{
    REXMLParser parser;
    parser.SetDelegate(this);
    return parser.ParseData(data, length);
}

void REGripCollection::FetchGrips(REGripVector& grips, int chromaticStep, const std::string& formulaId) const
{
    REGripCollectionEntryVector::const_iterator it = _grips.begin();
    for(; it != _grips.end(); ++it)
    {
        const REGripCollectionEntry& entry = *it;
        if(formulaId == entry.formulaId)
        {
            REGrip grip = entry.grip;
            if(chromaticStep == entry.chromaticStep)
            {
                grips.push_back(grip);                
            }
            else if(grip.IsTransposable())
            {
                int dfret = chromaticStep - entry.chromaticStep;
                grips.push_back(grip.Transposed(dfret));
            }
        }
    }
}

void REGripCollection::OnStartElement(const REXMLParser& parser, 
                                       const std::string& name, 
                                       const std::string& namespaceURI, 
                                       const std::string &qName, 
                                       const REXMLAttributeMap& attributes)
{
    if(name != "grip") return;
    
    std::string formulaId = "";
    std::string frets = "";
    std::string pitch = "";
    
    REXMLAttributeMap::const_iterator it = attributes.end();

    if((it = attributes.find("form")) != attributes.end()) {formulaId = it->second;}
    if((it = attributes.find("frets")) != attributes.end()) {frets = it->second;}
    if((it = attributes.find("pitch")) != attributes.end()) {pitch = it->second;}
    
    if(formulaId.empty() || frets.empty() || pitch.empty()) {
        return;
    }
    
    // Grip collection entry
    REGripCollectionEntry entry;
    entry.formulaId = formulaId;
    
    // Parse Pitch
    bool err = false;
    entry.chromaticStep = REPitchClass::Parse(pitch, &err).ChromaticStep();
    if(err) {
        return;
    }
    
    // Parse Grip Frets
    if(!entry.grip.BuildWithString(frets.c_str())) {
        return;
    }
    
    // Add it to the collection
    _grips.push_back(entry);
}
