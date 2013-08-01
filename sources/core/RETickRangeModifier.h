//
//  RETickRangeModifier.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RETickRangeModifier_h
#define Reflow_RETickRangeModifier_h

#include "RETypes.h"
#include "REOutputStream.h"
#include "REInputStream.h"

#include <sstream>

template <typename T>
class RETickRangeModifier
{
public:
    unsigned int ItemCount() const {return _items.size();}
    
    const T* Item(int idx) const {if(idx>=0 && idx < ItemCount()) return &_items[idx]; else return 0;}
    
    void SetItem(int idx, const T& value) {_items[idx].value = value;}
    
    void Insert(const T& elem) {
        Erase(elem.t0, elem.t1);
        int idx = 0;
        int count = ItemCount();
        while(idx < count && _items[idx].t0 < elem.t0) {++idx;}
        _items.insert(_items.begin() + idx, elem);
    }
    
    const T* ItemAppliedAt(int tick) const {
        for(int i=0; i<_items.size(); ++i) {
            if(_items[i].TickInRange(tick)) {
                return &_items[i];
            }
        }
        return NULL;
    }
    
    int MaxTick() const {
        if(_items.empty()) return 0;
        return _items[_items.size()-1].t1;
    }
    
    void EraseFromTick(int start) {
        Erase(start, MaxTick());
    }
    
    void Erase(int start, int end) {
        int firstIndexToErase = 0;
        int countToErase = 0;
        std::vector<T> itemsToReinsertAfterErase;
        
        for(int i=0; i<_items.size(); ++i) 
        {
            const T& elem = _items[i];
            int t0 = elem.t0;
            int t1 = elem.t1;
            
            // Elem is contained totally in cut, simply erase it
            if(start <= t0 && t1 <= end) 
            {
                if(countToErase == 0) {
                    firstIndexToErase = i;
                    countToErase = 1;
                }
                else {
                    ++countToErase;
                }
            }
            
            // Cut is contained totally in element, split it 
            else if(t0 <= start && end <= t1) 
            {
                if(t0 == start) {
                    _items[i].t0 = end;
                }
                else if(t1 == end) {
                    _items[i].t1 = start;
                }
                else {  // cut is stricly contained, split in two
                    T split = elem;
                    _items[i].t1 = start;
                    split.t0 = end;
                    itemsToReinsertAfterErase.push_back(split);
                }
            }
            
            // Cut overlaps left side
            else if(start < t0 && end <= t1 && end >= t0) {
                _items[i].t0 = end;
            }
            
            // Cut overlaps left side
            else if(t0 < start && start < t1 && end > t1) {
                _items[i].t1 = start;
            }
        }
        
        // Erase elements
        if(countToErase) {
            int lastIndexToErase = firstIndexToErase + countToErase - 1;
            _items.erase(_items.begin() + firstIndexToErase, _items.begin() + lastIndexToErase);
        }
        
        // Elements to reinsert
        for(int i=0; i<itemsToReinsertAfterErase.size(); ++i) {
            Insert(itemsToReinsertAfterErase[i]);
        }
    }
    
    void Clear() {
        _items.clear();
    }
    
    void EncodeTo(REOutputStream& coder) const {
        coder.WriteUInt32(ItemCount());
        for(uint32_t i=0; i<_items.size(); ++i) {
            Item(i)->EncodeTo(coder);
        }
    }
    
    void DecodeFrom(REInputStream& decoder) {
        Clear();
        uint32_t nbItems = decoder.ReadUInt32();
        for(uint32_t i=0; i<nbItems; ++i) {
            T item;
            item.DecodeFrom(decoder);
            _items.push_back(item);
        }
    }
    
    std::string ToString() const {
        if(_items.empty()) return "";
        std::ostringstream oss;
        for(int i=0; i<_items.size(); ++i) {
            if(i != 0) {oss << " ";}
            oss << _items[i].ToString();
        }
        return oss.str();
    }
    
private:
    std::vector<T> _items;
};


class RETickRangeModifierElement
{
public:    
    int t0;     // inclusive
    int t1;     // exclusive
    
    RETickRangeModifierElement() : t0(0), t1(0) {}
    
    RETickRangeModifierElement(int t0_, int t1_) 
    : t0(t0_), t1(t1_)
    {}
    
    bool TickInRange(int tick) const {
        return t0 <= tick && tick < t1;
    }
    
    void EncodeTo(REOutputStream& coder) const 
    {
        coder.WriteInt32(t0);
        coder.WriteInt32(t1);
    }
    
    void DecodeFrom(REInputStream& decoder) 
    {
        t0 = decoder.ReadInt32();
        t1 = decoder.ReadInt32();
    }  
    
    std::string ToString() const;
    
protected:
    virtual std::string ValueAsString() const = 0;
};


class REOttaviaRangeModifierElement : public RETickRangeModifierElement
{
public:
    Reflow::OttaviaType value;
    
    REOttaviaRangeModifierElement() : RETickRangeModifierElement(), value(Reflow::NoOttavia) {}
    
    REOttaviaRangeModifierElement(int t0_, int t1_, Reflow::OttaviaType value_) 
    : RETickRangeModifierElement(t0_, t1_), value(value_)
    {}
    
    void EncodeTo(REOutputStream& coder) const 
    {
        RETickRangeModifierElement::EncodeTo(coder);
        coder.WriteInt8(value);
    }
    
    void DecodeFrom(REInputStream& decoder) 
    {
        RETickRangeModifierElement::DecodeFrom(decoder);
        value = (Reflow::OttaviaType) decoder.ReadInt8();
    }  
    
protected:
    virtual std::string ValueAsString() const;
};

/** REOttaviaRangeModifier
 */
typedef RETickRangeModifier<REOttaviaRangeModifierElement> REOttaviaRangeModifier;

#endif
