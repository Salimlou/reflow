//
//  REChordFormulaCollection.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REChordFormulaCollection.h"

#include <cassert>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using std::vector;
using std::string;
using namespace boost;

REChordFormulaCollection::REChordFormulaCollection()
{}

REChordFormulaCollection::~REChordFormulaCollection()
{
    Clear();
}

void REChordFormulaCollection::Clear()
{
    _chords.clear();
}

bool REChordFormulaCollection::LoadChordsXML(const char *data, int length)
{
    REXMLParser parser;
    parser.SetDelegate(this);
    return parser.ParseData(data, length);
}

void REChordFormulaCollection::OnStartElement(const REXMLParser& parser, 
                                              const std::string& name, 
                                              const std::string& namespaceURI, 
                                              const std::string &qName, 
                                              const REXMLAttributeMap& attributes)
{
    if(name != "chord") return;
    
    std::string formulaId = "";
    std::string formula = "";
    std::string type = "";
    std::string chordName = "";
    std::string symbols = "";
    
    REXMLAttributeMap::const_iterator it = attributes.end();
    
    if((it = attributes.find("id")) != attributes.end()) {formulaId = it->second;}
    if((it = attributes.find("formula")) != attributes.end()) {formula = it->second;}
    if((it = attributes.find("type")) != attributes.end()) {type = it->second;}
    if((it = attributes.find("name")) != attributes.end()) {chordName = it->second;}
    if((it = attributes.find("symbols")) != attributes.end()) {symbols = it->second;}
    
    if(formulaId.empty() || formula.empty() || type.empty() || chordName.empty() || symbols.empty()) {
        return;
    }
    
    // Chord Formula entry
    REChordFormulaCollectionEntry entry;
    entry.formulaId = formulaId;
    entry.rawFormula = formula;
    entry.name = chordName;
    entry.type = type;
    
    // Symbols
    split(entry.symbols, symbols, is_any_of(" "));
    if(formulaId == "maj") {entry.symbols.push_back("");}
    if(entry.symbols.empty()) {
        return;
    }
    
    // Add formula to collection
    entry.formula = REChordFormula(formula, entry.symbols[0]);
    _chords.push_back(entry);
}

int REChordFormulaCollection::EntryCount() const
{
    return _chords.size();
}

const REChordFormulaCollectionEntry* REChordFormulaCollection::Entry(int idx) const
{
    if(idx >= 0 && idx < EntryCount()) {
        return &_chords[idx];
    }
    return NULL;
}

const REChordFormulaCollectionEntry* REChordFormulaCollection::EntryWithId(const std::string& chordId) const
{
    REChordFormulaCollectionEntryVector::const_iterator it = _chords.begin();
    for(; it != _chords.end(); ++it)
    {
        const REChordFormulaCollectionEntry* entry = &(*it);
        if(entry->formulaId == chordId) {
            return entry;
        }
    }
    return NULL;
}

const REChordFormulaCollectionEntry* REChordFormulaCollection::EntryWithSymbol(const std::string& symbol) const
{
    REChordFormulaCollectionEntryVector::const_iterator it = _chords.begin();
    for(; it != _chords.end(); ++it)
    {
        const REChordFormulaCollectionEntry* entry = &(*it);
        if(std::find(entry->symbols.begin(), entry->symbols.end(), symbol) != entry->symbols.end())
        {
            return entry;
        }
    }
    return NULL;
}

const REChordFormulaCollectionEntry* REChordFormulaCollection::EntryWithRawFormula(const std::string& rawFormula) const
{
    REChordFormulaCollectionEntryVector::const_iterator it = _chords.begin();
    for(; it != _chords.end(); ++it)
    {
        const REChordFormulaCollectionEntry* entry = &(*it);
        REPrintf("(%s - %s)\n", entry->rawFormula.c_str(), rawFormula.c_str());
        if(entry->rawFormula == rawFormula) {
            return entry;
        }
    }
    return NULL;
}

int REChordFormulaCollection::IndexOfEntry(const REChordFormulaCollectionEntry* entry) const
{
    for(int i=0; i<_chords.size(); ++i) {
        if(&_chords[i] == entry) {
            return i;
        }
    }
    return -1;
}
