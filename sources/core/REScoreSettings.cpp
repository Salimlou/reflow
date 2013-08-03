//
//  REScoreSettings.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 11/06/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "REScoreSettings.h"
#include "RESong.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REStaff.h"
#include "RETablatureStaff.h"
#include "REStandardStaff.h"
#include "RETrack.h"
#include "REFunctions.h"
#include "REPainter.h"
#include "REBarMetrics.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "REBar.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "RELayout.h"
#include "REStyle.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>

REScoreSettings::REScoreSettings()
: _index(-1), _paperSize (210*4, 297*4), _flags(0), _scalingFactor(1.00), _paperOrientation(Reflow::Portrait), _paperName("iso-a4"), _layout(nullptr), _style(nullptr)
{
    _virtualMargins[Reflow::TopMargin] = Reflow::MillimetersToScoreUnits(15);
    _virtualMargins[Reflow::RightMargin] = Reflow::MillimetersToScoreUnits(10);
    _virtualMargins[Reflow::BottomMargin] = Reflow::MillimetersToScoreUnits(15);
    _virtualMargins[Reflow::LeftMargin] = Reflow::MillimetersToScoreUnits(10);
}

REScoreSettings::REScoreSettings(const REScoreSettings& rhs) : _layout(nullptr), _style(nullptr)
{
    *this = rhs;
}

REScoreSettings::~REScoreSettings()
{
    delete _layout;
    delete _style;
}

RESize REScoreSettings::PaperSizeInMillimeters() const
{
    return RESize(Reflow::ScoreUnitsToMillimeters(_paperSize.w), Reflow::ScoreUnitsToMillimeters(_paperSize.h));
}

double REScoreSettings::VirtualMarginInMillimeters(Reflow::MarginLocation margin) const
{
    return Reflow::ScoreUnitsToMillimeters(VirtualMargin(margin));
}

RERect REScoreSettings::PageRect() const
{
    float w = _paperSize.w;
    float h = _paperSize.h;
    if(_paperOrientation == Reflow::Landscape) {std::swap(w,h);}
    return RERect(REPoint(0,0), RESize(::ceilf(w/_scalingFactor), ::ceilf(h/_scalingFactor)));
}

RERect REScoreSettings::ContentRect() const
{
    float w = _paperSize.w;
    float h = _paperSize.h;
    if(_paperOrientation == Reflow::Landscape) {
        std::swap(w,h);
    }
    
    RERect rc;
    rc.origin.x = ::ceilf(VirtualMargin(Reflow::LeftMargin) / _scalingFactor);
    rc.origin.y = ::ceilf(VirtualMargin(Reflow::TopMargin) / _scalingFactor);
    rc.size.w = ::ceilf((w - VirtualMargin(Reflow::LeftMargin) - VirtualMargin(Reflow::RightMargin)) / _scalingFactor);
    rc.size.h = ::ceilf((h - VirtualMargin(Reflow::TopMargin) - VirtualMargin(Reflow::BottomMargin)) / _scalingFactor);
    return rc;
}

bool REScoreSettings::InConcertTone() const
{
    return !HasFlag(REScoreSettings::TransposingScore);
}

void REScoreSettings::SetInConcertTone(bool ct)
{
    if(ct) UnsetFlag(REScoreSettings::TransposingScore);
    else SetFlag(REScoreSettings::TransposingScore);
}


bool REScoreSettings::HasSystemBreakAtBarIndex(int barIndex) const
{
    return _systemBreaks.find(barIndex) != _systemBreaks.end();
}

void REScoreSettings::SetSystemBreakAtBarIndex(int barIndex, bool systemBreak)
{
    if(systemBreak)
    {
        _systemBreaks.insert(barIndex);
    }
    else {
        _systemBreaks.erase(barIndex);
    }
}

void REScoreSettings::SetStyle(REStyle *style)
{
    if(_style) delete _style;
    _style = style;
}

void REScoreSettings::SetLayout(RELayout* layout)
{
    if(_layout) {
        delete _layout;
    }
    _layout = layout;
}

void REScoreSettings::SetTrackSet(const RETrackSet& trackSet)
{
    _trackSet = trackSet;
}

REScoreSettings& REScoreSettings::operator=(const REScoreSettings& rhs)
{
    REBufferOutputStream coder;
    rhs.EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    DecodeFrom(decoder);
    return *this;
}

bool REScoreSettings::operator==(const REScoreSettings& rhs) const {
    return true; // TODO
}

REConstTrackVector REScoreSettings::Tracks(const RESong* song) const
{
    REConstTrackVector tracks;
    for(unsigned int i=0; i<song->TrackCount(); ++i) {
        if(_trackSet.IsSet(i)) {
            tracks.push_back(song->Track(i));
        }
    }
    return tracks;    
}

RETrackVector REScoreSettings::Tracks(RESong* song) const
{
    RETrackVector tracks;
    for(unsigned int i=0; i<song->TrackCount(); ++i) {
        if(_trackSet.IsSet(i)) {
            tracks.push_back(song->Track(i));
        }
    }
    return tracks;
}

void REScoreSettings::SetTracks(const RETrackVector& tracks)
{
    RETrackSet trackSet;
    RETrackVector::const_iterator it = tracks.begin();
    for(; it != tracks.end(); ++it) {
        trackSet.Set((*it)->Index());
    }
    
    SetTrackSet(trackSet);
}

const RETrack* REScoreSettings::FirstTrack(const RESong* song) const
{
    REConstTrackVector tracks = Tracks(song);
    return tracks.empty() ? NULL : tracks.front();
}

void REScoreSettings::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    writer.String("name"); writer.String(_name.data(), _name.length());
    writer.String("tracks"); _trackSet.WriteJson(writer, version);
    
    // Paper
    writer.String("paper");
    {
        writer.StartObject();
        
        writer.String("width"); writer.Double(_paperSize.w);
        writer.String("height"); writer.Double(_paperSize.h);
        writer.String("margins");
        {
            writer.StartArray();
            for(int i=0; i<4; ++i) writer.Double(_virtualMargins[i]);
            writer.EndArray();
        }
        writer.String("name"); writer.String(_paperName.data(), _paperName.length());
        writer.String("orientation"); writer.String(Reflow::NameOfPaperOrientation(_paperOrientation));
        writer.String("scaling"); writer.Double(_scalingFactor);
        
        writer.EndObject();
    }
    
    // System breaks
    writer.String("system_breaks");
    {
        writer.StartArray();
        std::for_each(_systemBreaks.begin(), _systemBreaks.end(), std::bind(&REJsonWriter::Int, std::ref(writer), std::placeholders::_1));
        writer.EndArray();
    }
    
    writer.EndObject();
}

void REScoreSettings::ReadJson(const REJsonValue& obj, uint32_t version)
{
    // Name
    const REJsonValue& name = obj["name"];
    if(name.IsString()) {
        _name = std::string(name.GetString(), name.GetStringLength());
    }

    // Tracks
    const REJsonValue& trackSet = obj["tracks"];
    if(trackSet.IsArray()) {
        _trackSet.ReadJson(trackSet, version);
    }

    // Paper
    const REJsonValue& paper = obj["paper"];
    if(paper.IsObject())
    {
        if(paper["width"].IsDouble()) _paperSize.w = paper["width"].GetDouble();
        if(paper["height"].IsDouble()) _paperSize.h = paper["height"].GetDouble();
        
        const REJsonValue& margins = paper["margins"];
        if(margins.IsArray() && margins.Size() == 4)
        {
            _virtualMargins[0] = margins[0u].GetDouble();
            _virtualMargins[1] = margins[1u].GetDouble();
            _virtualMargins[2] = margins[2u].GetDouble();
            _virtualMargins[3] = margins[3u].GetDouble();
        }
        
        const REJsonValue& paperName = paper["name"];
        if(paperName.IsString()) _paperName = std::string(paperName.GetString(), paperName.GetStringLength());
        
        const REJsonValue& orientation = paper["orientation"];
        if(orientation.IsString()) {
            _paperOrientation = Reflow::ParsePaperOrientation(std::string(orientation.GetString(), orientation.GetStringLength()));
        }
        
        if(paper["scaling"].IsDouble()) _scalingFactor = paper["scaling"].GetDouble();
    }
    
    // System breaks
    const REJsonValue& breaks = obj["system_breaks"];
    if(breaks.IsArray()) {
        for(auto it = breaks.Begin(); it != breaks.End(); ++it) {
            if(it->IsInt()) {
                _systemBreaks.insert(it->GetInt());
            }
        }
    }
}

void REScoreSettings::EncodeTo(REOutputStream& coder) const
{
    coder.WriteUInt32(_flags);
    coder.WriteString(_name);
    _trackSet.EncodeTo(coder);
    coder.WriteDouble(_paperSize.w);
    coder.WriteDouble(_paperSize.h);
    for(int i=0; i<4; ++i) {
        coder.WriteDouble(_virtualMargins[i]);
    }
    coder.WriteDouble(_scalingFactor);
    coder.WriteInt8(_paperOrientation);
    coder.WriteString(_paperName);
    
    coder.WriteInt32(_systemBreaks.size());
    std::for_each(_systemBreaks.begin(), _systemBreaks.end(), std::bind(&REOutputStream::WriteInt32, std::ref(coder), std::placeholders::_1));
    
    if(coder.Version() >= REFLOW_IO_VERSION_1_8_0)
    {
        // Style
        if(_style) {
            coder.WriteString(_style->Identifier());
            _style->EncodeTo(coder);
        }
        else {
            coder.WriteString(REStyle::DefaultReflowStyle()->Identifier());
            REStyle::DefaultReflowStyle()->EncodeTo(coder);
        }
        
        // Layout
        if(_layout) {
            coder.WriteString(_layout->Identifier());
            _layout->EncodeTo(coder);
        }
        else {
            REFlexibleLayout layout;
            coder.WriteString(layout.Identifier());
            layout.EncodeTo(coder);
        }
    }
}

void REScoreSettings::DecodeFrom(REInputStream& decoder)
{
    _flags = decoder.ReadUInt32();
    _name = decoder.ReadString();
    _trackSet.DecodeFrom(decoder);
    _paperSize.w = decoder.ReadDouble();
    _paperSize.h = decoder.ReadDouble();
    for(int i=0; i<4; ++i) {
        _virtualMargins[i] = decoder.ReadDouble();
    }
    _scalingFactor = decoder.ReadDouble();
    _paperOrientation = (Reflow::PaperOrientation)decoder.ReadInt8();
    _paperName = decoder.ReadString();
    
    int nbSystemBreaks = decoder.ReadInt32();
    _systemBreaks.clear();
    for(int i=0; i<nbSystemBreaks; ++i) {
        _systemBreaks.insert(decoder.ReadInt32());
    }
    
    SetStyle(nullptr);
    SetLayout(nullptr);
    if(decoder.Version() >= REFLOW_IO_VERSION_1_8_0)
    {
        std::string identifier = decoder.ReadString();
        _style = REStyleCollection::Instance().CreateStyleWithIdentifier(identifier);
        _style->DecodeFrom(decoder);
        
        identifier = decoder.ReadString();
        _layout = RELayout::CreateLayoutWithIdentifier(identifier);
        _layout->DecodeFrom(decoder);
    }
}

REScoreSettings* REScoreSettings::Clone() const
{
    REBufferOutputStream coder;
    EncodeTo(coder);
    
    REConstBufferInputStream decoder(coder.Data(), coder.Size());
    REScoreSettings* score = new REScoreSettings;
    score->DecodeFrom(decoder);
    return score;
}
