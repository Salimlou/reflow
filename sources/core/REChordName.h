//
//  REChordName.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 16/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REChordName_h
#define Reflow_REChordName_h

#include "RETypes.h"
#include "REChordFormula.h"
#include "REPitchClass.h"

class REChordName
{
public:
    REChordName();
    REChordName(const REChordName& rhs) {*this = rhs;}
    REChordName(const REPitchClass& root, const REChordFormula& formula);

    const REPitchClass& Root() const;
    const REPitchClass& Tonic() const;
    const REChordFormula& Formula() const;
    
    int DegreeCount() const;
    REPitchClass Degree(int degree) const;
    
    void SetInversion(int inversion) {_inversion = inversion;}
    int Inversion() const {return _inversion;}
    
    REChordName& operator=(const REChordName& rhs);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    std::string ToString() const;
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
private:
    REPitchClass _root;
    REChordFormula _formula;
    int8_t _inversion;
};

typedef std::vector<REChordName> REChordNameVector;

#endif
