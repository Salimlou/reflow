//
//  REChordFormulaCollection.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 18/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef _RECHORDFORMULACOLLECTION_H_
#define _RECHORDFORMULACOLLECTION_H_


#include "RETypes.h"
#include "REChordFormula.h"
#include "REXMLParser.h"


class REChordFormulaCollectionEntry
{
public:
    REChordFormulaCollectionEntry() {}
    REChordFormulaCollectionEntry(const REChordFormulaCollectionEntry& rhs) {*this = rhs;}
    
    REChordFormulaCollectionEntry& operator=(const REChordFormulaCollectionEntry& rhs)
    {
        formulaId = rhs.formulaId;
        rawFormula = rhs.rawFormula;
        formula = rhs.formula;
        type = rhs.type;
        name = rhs.name;
        symbols = rhs.symbols;
        return *this;
    }
    
public:
    std::string formulaId;
    std::string rawFormula;
    REChordFormula formula;
    std::string type;
    std::string name;
    std::vector<std::string> symbols;
};

typedef std::vector<REChordFormulaCollectionEntry> REChordFormulaCollectionEntryVector;


class REChordFormulaCollection : private REXMLParserDelegate
{
public:
    REChordFormulaCollection();
    virtual ~REChordFormulaCollection();
    
    bool LoadChordsXML(const char* data, int length);
    
    void Clear();
    
    int EntryCount() const;
    const REChordFormulaCollectionEntry* Entry(int idx) const;
    const REChordFormulaCollectionEntry* EntryWithId(const std::string& chordId) const;
    const REChordFormulaCollectionEntry* EntryWithSymbol(const std::string& symbol) const;
    const REChordFormulaCollectionEntry* EntryWithRawFormula(const std::string& rawFormula) const;
    int IndexOfEntry(const REChordFormulaCollectionEntry* entry) const;
    
    static REChordFormulaCollection* BuiltinCollection();
    
private:
    REChordFormulaCollectionEntryVector _chords;
    static REChordFormulaCollection* _builtinCollection;
    
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
