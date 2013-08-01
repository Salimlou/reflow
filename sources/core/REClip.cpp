//
//  REClip.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 13/02/13.
//
//

#include "REClip.h"

REClip::REClip()
: _d(nullptr), _tick(0), _duration(0)
{
    
}
REClip::REClip(const REClip& REClip)
: _d(nullptr), _tick(0), _duration(0)
{
    SetData(REClip._d);
}
REClip::~REClip()
{
    ClearData();
}
REClip& REClip::operator=(const REClip& REClip)
{
    SetData(REClip._d);
    return *this;
}

void REClip::AddNote(const RXNote& note)
{
    if(_d == nullptr) _d = new REClipData;
    _d->AddNote(note);
}

size_t REClip::NoteCount() const
{
    return _d ? _d->note_count() : 0;
}


inline void REClip::SetData(REClipData* d)
{
    if(_d == d) return;
    ClearData();
    ++d->_refs;
    _d = d;
}

inline void REClip::ClearData()
{
    if(_d && --_d->_refs == 0) {
        delete _d;
        _d = nullptr;
    }
}


REClipData::REClipData()
: _refs(0)
{}
REClipData::~REClipData()
{
    _notes.clear();
}

void REClipData::AddNote(const RXNote& note)
{
    _notes.push_back(note);
}