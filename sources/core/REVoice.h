//
//  REVoice.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REVOICE_H_
#define _REVOICE_H_

#include "RETypes.h"

class REVoice
{
	friend class RETrack;
    friend class REArchive;
	
public:
    REVoice();
    ~REVoice();
	
public:
	const REPhraseVector& Phrases() const {return _phrases;}
	REPhraseVector& Phrases() {return _phrases;}
	
	unsigned int PhraseCount() const {return (unsigned int)_phrases.size();}
	
	const REPhrase* Phrase(int idx) const;
	REPhrase* Phrase(int idx);
	
	const RETrack* Track() const {return _parent;}
	RETrack* Track() {return _parent;}
	
    bool IsEmpty() const;
    
	void Clear();
    void Refresh();
	
	int Index() const {return _index;}
	
	void InsertPhrase(REPhrase* phrase, int idx);
    void AddPhrase(REPhrase* phrase);
	void RemovePhrase(int idx);
    void RemovePhrasesInRange(int idx, int count);
    
    REPhrase* IsolateNewPhraseFromSelection(const REChord* firstChord, const REChord* lastChord) const;
    REPhrase* IsolateNewPhraseFromSelectionInTablature(const REChord* firstChord, const REChord* lastChord, int firstString, int lastString) const;
    REPhrase* IsolateNewPhraseFromSelectionInStaff(const REChord* firstChord, const REChord* lastChord, int firstLine, int lastLine, bool transposing) const;
	
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
    void EncodeKeepingPhrasesInRange(REOutputStream& coder, int firstBar, int lastBar) const;
    
public:
    bool operator==(const REVoice& rhs) const;
    bool operator!=(const REVoice& rhs) const {return !(*this == rhs);}
    
private:
	void _UpdateIndices();
	
private:
	RETrack* _parent;
	REPhraseVector _phrases;
	int _index;
};

#endif
