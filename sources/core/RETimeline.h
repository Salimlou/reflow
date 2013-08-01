//
//  RETimeline.h
//  Reflow
//
//  Created by Sebastien on 21/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RETIMELINE_H_
#define _RETIMELINE_H_

#include "RETypes.h"
#include "REFunctions.h"
#include "REOutputStream.h"
#include "REInputStream.h"

/** RETimeline class.
 */
template <typename T>
class RETimeline
{
public:
    typedef T ValueType;
    
public:
    unsigned int ItemCount() const {return _items.size();}
    
    const T* Item(int idx) const {if(idx>=0 && idx < ItemCount()) return &_items[idx]; else return 0;}
    
    void SetItem(int idx, const T& value) {_items[idx].value = value;}
    
    // Returns the (index,exact) pair for the event that is applied at given <bar,beat>.
    int IndexOfItemAt(int bar, const RETimeDiv& beat, bool* exact) const {
        int idx=0;
        for(; idx<ItemCount(); ++idx)
        {
            const T* it = &_items[idx];
            if(it->bar > bar) {
                break;
            }
            else if (it->bar == bar) 
            {
                if(it->beat > beat) {
                    break;
                }
                else if(it->beat == beat) {
                    if(exact != 0) *exact = true;
                    return idx;
                }
                else { // it->beat < beat
                }
            }
            else {  // (it->bar < bar)
            }
        }
        
        if(exact != 0) *exact = false;
        return idx-1;
    }
    
    const T* ItemAt(int bar, const RETimeDiv& beat, bool* exact=0) const
    {
        return Item(IndexOfItemAt(bar,beat,exact));
    }
    
    void InsertItem(const T& val) 
    {
        bool exact;
        int index = IndexOfItemAt(val.bar, val.beat, &exact);
        if(exact) {
            _items[index] = val;
        }
        else {
            _items.insert(_items.begin() + (index+1), val);
        }
    }
    
    void InsertBarAtIndex(int barIndex)
    {
        if(_items.empty()) return;
        
        T first = _items.front();
        
        // Shift items one bar right
        for(int i=0; i<_items.size(); ++i) {
            T& item = _items[i];
            if(item.bar >= barIndex) {
                item.bar += 1;
            }
        }
        
        // Reinsert the first item if shifting the first bar
        if(barIndex == 0) {
            _items.insert(_items.begin(), first);
        }
    }
    
    void RemoveBarAtIndex(int barIndex)
    {
        if(_items.empty()) return;
        
        T first = _items.front();
        
        // Shift items one bar left
        for(int i=0; i<_items.size(); ++i) {
            T& item = _items[i];
            if(item.bar >= barIndex) {
                item.bar -= 1;
            }
        }
        
        if(barIndex == 0 && NULL == ItemAt(barIndex, RETimeDiv(0))) {
            _items.insert(_items.begin(), first);
        }
    }
    
    void RemoveBarsInRange(int barIndex, int count)
    {
        if(_items.empty()) return;
        
        T first = _items.front();
        
        // Shift items <count> bar left
        for(int i=0; i<_items.size(); ++i) {
            T& item = _items[i];
            if(item.bar >= barIndex) {
                item.bar -= count;
            }
        }
        
        if(barIndex == 0 && NULL == ItemAt(barIndex, RETimeDiv(0))) {
            _items.insert(_items.begin(), first);
        }
    }
    
    void RemoveItemsInBarRange(int firstBar, int lastBar)
    {
        bool found = false;
        int firstIdxToRemove = 0;
        int lastIdxToRemove = 0;
        for(int i=0; i<_items.size(); ++i) {
            const T& item = _items[i];
            if(firstBar <= item.bar && item.bar <= lastBar) {
                if(found) {
                    lastIdxToRemove = i;
                }
                else {
                    found = true;
                    firstIdxToRemove = i;
                    lastIdxToRemove = i;
                }
            }
        }
        
        if(found) {
            _items.erase(_items.begin()+firstIdxToRemove, _items.begin()+(lastIdxToRemove+1));
        }
    }
    
    bool HasItemInBarRange(int firstBar, int lastBar) const 
    {
        for(int i=0; i<_items.size(); ++i) 
        {
            const T& item = _items[i];
            if(firstBar <= item.bar && item.bar <= lastBar) {
                return true;
            }
        }
        return false;
    }
    
    bool FindItemsInBarRange(int firstBar, int lastBar, int* firstItemIndex, int* lastItemIndex) const
    {
        bool found = false;
        int firstIdx = 0;
        int lastIdx = 0;
        for(int i=0; i<_items.size(); ++i) 
        {
            const T& item = _items[i];
            if(firstBar <= item.bar && item.bar <= lastBar) 
            {
                if(found) {
                    lastIdx = i;
                }
                else {
                    found = true;
                    firstIdx = i;
                    lastIdx = i;
                }
            }
        }
        
        if(found) {
            if(firstItemIndex != NULL) *firstItemIndex = firstIdx;
            if(lastItemIndex != NULL) *lastItemIndex = lastIdx;
            return true;
        }
        else return false;
    }
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const
    {
        writer.StartArray();
        for(uint32_t i=0; i<_items.size(); ++i)
        {
            writer.StartObject();
            Item(i)->WriteContentsToJson(writer, version);
            writer.EndObject();
        }
        writer.EndArray();
    }
    void ReadJson(const REJsonValue& obj, uint32_t version)
    {
        _items.clear();
        for(auto it = obj.Begin(); it != obj.End(); ++it)
        {
            if(it->IsObject()) {
                T item;
                item.ReadJson(*it, version);
                _items.push_back(item);
            }
        }
    }
    
    void EncodeTo(REOutputStream& coder) const {
        coder.WriteUInt32(ItemCount());
        for(uint32_t i=0; i<_items.size(); ++i) {
            Item(i)->EncodeTo(coder);
        }
    }
    
    void DecodeFrom(REInputStream& decoder) {
        _items.clear();
        uint32_t nbItems = decoder.ReadUInt32();
        for(uint32_t i=0; i<nbItems; ++i) {
            T item;
            item.DecodeFrom(decoder);
            _items.push_back(item);
        }
    }
    
    RETimeline<T> * Clone() const 
    {
        REBufferOutputStream buffer;
        EncodeTo(buffer);
        
        REConstBufferInputStream input(buffer.Data(), buffer.Size());
        RETimeline<T>* timeline = new RETimeline<T>();
        timeline->DecodeFrom(input);
        return timeline;
    }
    
    void RemoveIdenticalSiblingItems()
    {
        for(int i=(int)_items.size()-1; i>0; --i)
        {
            if(_items[i] == _items[i-1])
            {
                _items.erase(_items.begin() + i);
            }
        }
    }
    
private:
    std::vector<T> _items;
};

class RETimelineItem
{
public:    
    RETimeDiv beat;
    int bar;
    
    RETimelineItem() {}
    
    RETimelineItem(int bar_, const RETimeDiv& beat_) 
    : beat(beat_), bar(bar_)
    {}
    
    virtual ~RETimelineItem() {}
    
    virtual void WriteContentsToJson(REJsonWriter& writer, uint32_t version) const
    {
        writer.String("at");
        writer.StartObject();
        {
            writer.String("bar"); writer.Int(bar);
            writer.String("div"); Reflow::WriteTimeDivToJson(beat, writer, version);
        }
        writer.EndObject();
    }
    
    virtual void ReadJson(const REJsonValue& obj, uint32_t version)
    {
        const REJsonValue& at = obj["at"];
        if(at.IsObject())
        {
            const REJsonValue& bar_ = at["bar"];
            if(bar_.IsInt()) {bar = bar_.GetInt();}
            
            const REJsonValue& beat_ = at["div"];
            if(!beat_.IsNull()) {Reflow::ReadTimeDivFromJson(beat, beat_, version);}
        }
    }
    
    virtual void EncodeTo(REOutputStream& coder) const
    {
        coder.WriteUInt32(beat.numerator());
        coder.WriteUInt32(beat.denominator());
        coder.WriteUInt16(bar);
    }
    
    virtual void DecodeFrom(REInputStream& decoder)
    {
        uint32_t num = decoder.ReadUInt32();
        uint32_t den = decoder.ReadUInt32();
        beat = RETimeDiv(num,den);
        bar = decoder.ReadUInt16();
    }  
};


class REClefItem : public RETimelineItem
{
public:
    Reflow::ClefType clef;
    Reflow::OttaviaType ottavia;
    
    REClefItem() : clef(Reflow::TrebleClef), ottavia(Reflow::NoOttavia) {}
    
    REClefItem(int bar_, const RETimeDiv& beat_, Reflow::ClefType clef_=Reflow::TrebleClef, Reflow::OttaviaType ott_=Reflow::NoOttavia) 
        : RETimelineItem(bar_, beat_), clef(clef_), ottavia(ott_)
    {}
    
    virtual ~REClefItem() {}
    
    virtual void WriteContentsToJson(REJsonWriter& writer, uint32_t version) const
    {
        RETimelineItem::WriteContentsToJson(writer, version);
        
        writer.String("clef"); writer.String(Reflow::NameOfClefType(clef));
        if(ottavia != Reflow::NoOttavia) {
            writer.String("ottavia"); writer.String(Reflow::NameOfOttaviaType(ottavia));
        }
    }
    
    virtual void ReadJson(const REJsonValue& obj, uint32_t version)
    {
        RETimelineItem::ReadJson(obj, version);
        
        const REJsonValue& clef_ = obj["clef"];
        if(clef_.IsString()) {
            clef = Reflow::ParseClefType(std::string(clef_.GetString(), clef_.GetStringLength()));
        }
        
        const REJsonValue& ott_ = obj["ottavia"];
        if(ott_.IsString()) {
            ottavia = Reflow::ParseOttaviaType(std::string(ott_.GetString(), ott_.GetStringLength()));
        }
    }
    
    void EncodeTo(REOutputStream& coder) const 
    {
        RETimelineItem::EncodeTo(coder);
        
        coder.WriteUInt8(clef);
        coder.WriteUInt8(ottavia);
    }
    
    void DecodeFrom(REInputStream& decoder) 
    {
        RETimelineItem::DecodeFrom(decoder);
        
        clef = (Reflow::ClefType)decoder.ReadUInt8();
        ottavia = (Reflow::OttaviaType)decoder.ReadUInt8();
    }
    
    bool operator==(const REClefItem& rhs) const {return clef == rhs.clef && ottavia == rhs.ottavia;}
};


class RETempoItem : public RETimelineItem
{
public:
    uint16_t tempo;
    Reflow::TempoUnitType unitType;
    
    RETempoItem() : tempo(90), unitType(Reflow::QuarterTempoUnit) {}
    
    RETempoItem(int bar_, const RETimeDiv& beat_, uint16_t tempo_) 
        : RETimelineItem(bar_, beat_), tempo(tempo_), unitType(Reflow::QuarterTempoUnit)
    {}
    
    RETempoItem(int bar_, const RETimeDiv& beat_, uint16_t tempo_, Reflow::TempoUnitType ut)
    : RETimelineItem(bar_, beat_), tempo(tempo_), unitType(ut)
    {}
    
    virtual ~RETempoItem() {}
    
    virtual void WriteContentsToJson(REJsonWriter& writer, uint32_t version) const
    {
        RETimelineItem::WriteContentsToJson(writer, version);
        
        writer.String("tempo"); writer.Int(tempo);
        if(unitType != Reflow::QuarterTempoUnit)
        {
            writer.String("unit"); writer.String(Reflow::NameOfTempoUnitType(unitType));
        }
    }
    
    virtual void ReadJson(const REJsonValue& obj, uint32_t version)
    {
        RETimelineItem::ReadJson(obj, version);
        
        const REJsonValue& tempo_ = obj["tempo"];
        if(tempo_.IsInt()) {
            tempo = tempo_.GetInt();
        }
        
        const REJsonValue& unit_ = obj["unit"];
        if(unit_.IsString()) {
            unitType = Reflow::ParseTempoUnitType(std::string(unit_.GetString(), unit_.GetStringLength()));
        }
    }
    
    void EncodeTo(REOutputStream& coder) const 
    {
        RETimelineItem::EncodeTo(coder);
        
        coder.WriteUInt16(tempo);
        coder.WriteUInt8(unitType);
    }
    
    void DecodeFrom(REInputStream& decoder) 
    {
        RETimelineItem::DecodeFrom(decoder);
        
        tempo = decoder.ReadUInt16();
        if(decoder.Version() >= REFLOW_IO_VERSION_1_7_0) {
            unitType = (Reflow::TempoUnitType)decoder.ReadUInt8();
        }
        else unitType = Reflow::QuarterTempoUnit;
    }
    
    bool operator==(const RETempoItem& rhs) const {return tempo == rhs.tempo && unitType == rhs.unitType;}
    
    double BeatsPerMinute() const {
        switch(unitType) {
            case Reflow::QuarterDottedTempoUnit:
                return 1.5 * (double)tempo;
            case Reflow::QuarterTempoUnit:
            default:
                return (double)tempo;
        }
    }
};

/*class REKeySignatureItem
{
public:  
    RETimeDiv beat;
    int bar;
    
    REKeySignature keySignature;
    
    REKeySignatureItem(int bar_, const RETimeDiv& beat_, REKeySignature ks_) : beat(beat_), bar(bar_), keySignature(ks_) {}
};*/

typedef RETimeline<REClefItem> REClefTimeline;
typedef RETimeline<RETempoItem> RETempoTimeline;
/*typedef RETimeline<REKeySignatureItem> REKeySignatureTimeline;*/


#endif
