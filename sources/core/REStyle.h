//
//  REStyle.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 28/06/13.
//
//

#ifndef __Reflow__REStyle__
#define __Reflow__REStyle__

#include "RETypes.h"

typedef std::function<REStyle*(void)> REStyleCreator;
typedef std::map<std::string, REStyleCreator> REStyleCreatorMap;

class REStyleCollection
{
public:
    static REStyleCollection& Instance();
    
    void Register(const std::string& identifier, const REStyleCreator& creator);
    REStyle* CreateStyleWithIdentifier(const std::string& identifier);
    
private:
    REStyleCollection();
    
private:
    static REStyleCollection* _instance;
    REStyleCreatorMap _creators;
};

class REStyle
{
public:
    REStyle();
    virtual ~REStyle();
    
    virtual REStyle* Clone() const;
    
    virtual std::string Identifier() const;
    
    static const REStyle* DefaultReflowStyle();
    
    void EncodeTo(REOutputStream& coder) const;
    void DecodeFrom(REInputStream& decoder);
    
public:
    virtual void PostDrawSystem(REPainter& painter, const RESystem& system) const {}
    
public:
    virtual void DrawChordNamesOfSlice(REPainter& painter, const RESlice* slice, float yOffset) const;
    
public:
    float ChordNameFontSize() const {return _chordNameFontSize;}
    float LeftMarginOfFirstSystem() const {return _leftMarginOfFirstSystem;}
    float LeftMarginOfOtherSystems() const {return _leftMarginOfOtherSystems;}
    
    void SetChordNameFontSize(float sz) {_chordNameFontSize = sz;}
    void SetLeftMarginOfFirstSystem(float m) {_leftMarginOfFirstSystem = m;}
    void SetLeftMarginOfOtherSystems(float m) {_leftMarginOfOtherSystems = m;}
    
private:
    float _chordNameFontSize;
    float _leftMarginOfFirstSystem;
    float _leftMarginOfOtherSystems;
    
private:
    static REStyle* _defaultReflowStyle;
};


#endif /* defined(__Reflow__REStyle__) */
