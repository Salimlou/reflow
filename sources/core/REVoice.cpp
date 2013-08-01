//
//  REVoice.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RETrack.h"
#include "RESong.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#include <algorithm>
#include <boost/bind.hpp>

REVoice::REVoice()
: _parent(0), _index(-1)
{
    
}
REVoice::~REVoice()
{
    Clear();
}

void REVoice::Clear() {
    REPhraseVector::const_iterator it = _phrases.begin();
    for(; it != _phrases.end(); ++it) {
        delete *it;
    }
    _phrases.clear();
}

void REVoice::Refresh()
{
    REPhraseVector::const_iterator it = _phrases.begin();
    for(; it != _phrases.end(); ++it) {
        (*it)->Refresh();
    }
}

const REPhrase* REVoice::Phrase(int idx) const
{
    if(idx >= 0 && idx < _phrases.size()) {
        return _phrases[idx];
    }
    return 0;
}

REPhrase* REVoice::Phrase(int idx)
{
    if(idx >= 0 && idx < _phrases.size()) {
        return _phrases[idx];
    }
    return 0;
}

void REVoice::AddPhrase(REPhrase* phrase)
{
    _phrases.push_back(phrase);
    phrase->_parent = this;
    _UpdateIndices();
}

void REVoice::InsertPhrase(REPhrase* phrase, int idx)
{
    _phrases.insert(_phrases.begin() + idx, phrase);
    phrase->_parent = this;
    _UpdateIndices();
}

void REVoice::RemovePhrase(int idx)
{
    if(idx >= 0 && idx < _phrases.size()) {
        REPhrase* phrase = _phrases[idx];
        delete phrase;
        _phrases.erase(_phrases.begin() + idx);
        _UpdateIndices();
    }
}

static inline void _DeletePhrase(REPhrase* phrase) {delete phrase;}
void REVoice::RemovePhrasesInRange(int idx, int count)
{
    REPhraseVector::iterator it0 = _phrases.begin()+idx;
    REPhraseVector::iterator it1 = _phrases.begin()+idx+count;
    
    std::for_each(it0, it1, boost::bind(_DeletePhrase, _1));
    _phrases.erase(it0, it1);
    _UpdateIndices();
}

void REVoice::_UpdateIndices() {
    for(unsigned int i=0; i<_phrases.size(); ++i) {
        _phrases[i]->_index = i;
    }
}

bool REVoice::operator==(const REVoice &rhs) const 
{
    if(PhraseCount() != rhs.PhraseCount()) {
        return false;
    }
    
    for(unsigned int i=0; i<_phrases.size(); ++i) {
        if(*Phrase(i) != *rhs.Phrase(i)) {
            return false;
        }
    }    
    return true;
}

REPhrase* REVoice::IsolateNewPhraseFromSelection(const REChord* firstChord, const REChord* lastChord) const
{
    REPhrase* phrase = new REPhrase();
    
    if(!firstChord) return phrase;
    
    // Process operation on chord range
    const REChord* chord = firstChord;
    while(chord) 
    {   
        phrase->AddChord(chord->Clone());
        
        if(chord == lastChord) break;
        else chord = chord->NextSiblingOverMultipleBarlines();
    }
    
    //phrase->Refresh();
    return phrase;
}

REPhrase* REVoice::IsolateNewPhraseFromSelectionInTablature(const REChord* firstChord, const REChord* lastChord, int firstString, int lastString) const 
{
    REPhrase* phrase = new REPhrase();
    
    if(!firstChord) return phrase;
    
    // Process operation on chord range
    const REChord* chord = firstChord;
    while(chord) 
    {   
        phrase->AddChord(chord->CloneKeepingNotesInStringRange(firstString, lastString));
        
        if(chord == lastChord) break;
        else chord = chord->NextSiblingOverMultipleBarlines();
    }
    
    //phrase->Refresh();
    return phrase;
}

REPhrase* REVoice::IsolateNewPhraseFromSelectionInStaff(const REChord* firstChord, const REChord* lastChord, int firstLine, int lastLine, bool transposing) const
{
    REPhrase* phrase = new REPhrase();
    
    if(!firstChord) return phrase;
    
    // Process operation on chord range
    const REChord* chord = firstChord;
    while(chord) 
    {   
        phrase->AddChord(chord->CloneKeepingNotesInLineRange(firstLine, lastLine, transposing));
        
        if(chord == lastChord) break;
        else chord = chord->NextSiblingOverMultipleBarlines();
    }
    
    //phrase->Refresh();
    return phrase;
}

bool REVoice::IsEmpty() const
{
    for(const REPhrase* phrase : _phrases)
    {
        if(!phrase->IsEmpty()) return false;
        if(phrase->ChordDiagramCount() > 0) return false;
    }
    return true;
}

void REVoice::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    // Phrases
    if(!IsEmpty())
    {
        writer.String("phrases");
        {
            writer.StartArray();
            for(unsigned int i=0; i<_phrases.size(); ++i)
            {
                _phrases[i]->WriteJson(writer, version);
            }
            writer.EndArray();
        }
    }
    
    writer.EndObject();
}

void REVoice::ReadJson(const REJsonValue& obj, uint32_t version)
{
    Clear();
    
    // Phrases
    const REJsonValue& phrases_ = obj["phrases"];
    if(phrases_.IsArray()) {
        for(auto it = phrases_.Begin(); it != phrases_.End(); ++it)
        {
            REPhrase* phrase = new REPhrase;
            phrase->_index = _phrases.size();
            phrase->_parent = this;
            _phrases.push_back(phrase);
            
            phrase->ReadJson(*it, version);
            phrase->Refresh();
        }
    }
    else {
        // Create empty phrases, voice is empty
        int barCount = Track()->Song()->BarCount();
        for(int i=0; i<barCount; ++i) {
            REPhrase* phrase = new REPhrase;
            phrase->_index = i;
            phrase->_parent = this;
            _phrases.push_back(phrase);
            phrase->Refresh();
        }
    }
}

void REVoice::EncodeTo(REOutputStream& coder) const
{
    uint16_t barCount = _phrases.size();
    coder.WriteUInt16(barCount);
    for(uint16_t i=0; i<barCount; ++i) {
        const REPhrase* phrase = _phrases[i];
        phrase->EncodeTo(coder);
    }
}

void REVoice::DecodeFrom(REInputStream& decoder)
{
    Clear();
    uint16_t barCount = decoder.ReadUInt16();
    for(uint16_t i=0; i<barCount; ++i) {
        REPhrase* phrase = new REPhrase;
        phrase->_index = i;
        phrase->_parent = this;
        _phrases.push_back(phrase);
        
        phrase->DecodeFrom(decoder);
        phrase->Refresh();
    }
}

void REVoice::EncodeKeepingPhrasesInRange(REOutputStream& coder, int firstBar, int lastBar) const
{
    uint16_t barCount = (lastBar-firstBar+1);
    coder.WriteUInt16(barCount);
    for(uint16_t i=firstBar; i<=lastBar; ++i) {
        const REPhrase* phrase = _phrases[i];
        phrase->EncodeTo(coder);
    }
}
