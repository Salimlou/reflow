//
//  REClip.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 13/02/13.
//
//

#ifndef __Reflow__REClip__
#define __Reflow__REClip__

#include "REBeat.h"

class REClip;
class REClipData;

/** REClip
 */
class REClip
{
public:
    typedef RXNote item_type;
    
public:
    REClip();
    REClip(const REClip& REClip);
    virtual ~REClip();
    
    REClip& operator=(const REClip& REClip);
    
public:
    void AddNote(const RXNote& note);
    size_t NoteCount() const;
    
    inline int32_t tick() const {return _tick;}
    inline int32_t duration() const {return _duration;}
    
    inline void set_tick(int32_t tick) {_tick = tick;}
    inline void set_duration(int32_t duration) {_duration = duration;}
    
    inline const REClipData* data_ptr() const {return _d;}
    
    template <typename F>
    void each(F func);
    
protected:
    void SetData(REClipData* d);
    void ClearData();
    
protected:
    REClipData* _d;
    int32_t _tick;
    int32_t _duration;
};

typedef RERepeater<REClip> REClipRepeater;
typedef RECutter<REClip> REClipCutter;
typedef std::vector<REClip> REClipVector;

/** REClipData
 */
class REClipData
{
    friend class REClip;
    
public:
    REClipData();
    ~REClipData();
    
public:
    size_t note_count() const {return _notes.size();}
    int32_t ref_count() const {return _refs;}
    
protected:
    void AddNote(const RXNote& note);
    
protected:
    int32_t _refs;
    RXNoteVector _notes;
};


template <typename F>
void REClip::each(F func)
{
    std::for_each(_d->_notes.begin(), _d->_notes.end(), func);
}
#endif /* defined(__Reflow__REClip__) */
