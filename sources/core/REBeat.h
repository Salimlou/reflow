//
//  REBeat.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 01/02/13.
//
//

#ifndef __Reflow__REBeat__
#define __Reflow__REBeat__

#include "RETypes.h"

#include <iostream>

class RXNote;
class RENoteData;
class REBeat;
class REBeatData;
class REBeatChordData;
class REBeatNoteData;
class REBeatGroupData;
class REClip;

typedef std::vector<RXNote> RXNoteVector;

/** repeater
 */
template <typename T>
class RERepeater
{
public:
    typedef T src_type;
    typedef typename src_type::item_type item_type;
    
public:
    explicit RERepeater(T& src) : _src(src), _count(1) {}
    explicit RERepeater(T& src, int nb) : _src(src), _count(nb) {}
    
    inline int32_t duration() const {
        return _src.duration() * repeat_count();
    }
    
    int repeat_count() const {return _count;}
    
    
    template <typename F>
    void each(F func)
    {
        for(int i=0; i<repeat_count(); ++i)
        {
            int32_t dt = i * _src.duration();
            _src.each([&](item_type& item_)
                      {
                          auto item = item_;
                          item.set_tick(item.tick() + dt);
                          func(item);
                      });
        }
    }
    
    template <typename F>
    void each_note(F func) const
    {
        for(int i=0; i<repeat_count(); ++i)
        {
            int32_t dt = i * _src.duration();
            _src.each([&](item_type& item_)
                      {
                          auto item = item_;
                          item.set_tick(item.tick() + dt);
                          item.each_note([&](const RXNote& note){func(note);});
                      });
        }
    }
    
protected:
    T& _src;
    int _count;
};


/** cutter
 */
template <typename T>
class RECutter
{
public:
    typedef T src_type;
    typedef typename src_type::item_type item_type;
    typedef std::pair<item_type, item_type> item_pair_type;
    
public:
    explicit RECutter(T& src) : _src(src), _offset(0), _length(src.duration()) {}
    explicit RECutter(T& src, int offset) : _src(src), _offset(offset), _length(src.duration() - offset) {}
    explicit RECutter(T& src, int offset, int length) : _src(src), _offset(offset), _length(length) {}
    
    inline int32_t duration() const {
        return _length;
    }
    
    template <typename F>
    void each(F func)
    {
        _src.each([&](item_type& item_)
                  {
                      int32_t t0 = item_.tick();
                      int32_t t1 = item_.last_tick();
                      int32_t left_cut = _offset;
                      int32_t right_cut = _offset + _length;
                      if(t0 >= left_cut && t0 < right_cut)
                      {
                          if(t1 > right_cut)
                          {
                              item_type a = item_.cut(right_cut - t0).first;
                              if(a.duration() > 0) {
                                  a.set_tick(a.tick() - left_cut);
                                  func(a);
                              }
                          }
                          else {
                              if(item_.duration() > 0) {
                                  item_type a = item_;
                                  a.set_tick(a.tick() - left_cut);
                                  func(a);
                              }
                          }
                      }
                      else if(t1 > left_cut)
                      {
                          item_type a = item_.cut(left_cut - t0).second;
                          
                          if(t1 > right_cut)
                          {
                              item_type b = a.cut(right_cut - a.tick()).first;
                              if(b.duration() > 0) {
                                  b.set_tick(b.tick() - left_cut);
                                  func(b);
                              }
                          }
                          else
                          {
                              if(a.duration() > 0) {
                                  a.set_tick(a.tick() - left_cut);
                                  func(a);
                              }
                          }
                      }
                      
                  });
    }
    
public:
    T& _src;
    int32_t _offset;
    int32_t _length;
};


/** check  that integer is a power of two using complement and compare algo */
inline bool IsPowerOfTwo(unsigned int x) {
    return ((x != 0) && ((x & (~x + 1)) == x));
}


/** RXNote
 */
class RXNote
{
public:
    RXNote();
    RXNote(const RXNote& rhs);
    explicit RXNote(int8_t p);
    RXNote(int32_t tick, int32_t duration, int8_t pitch, int8_t velocity);
    ~RXNote();
    
    RXNote& operator=(const RXNote& rhs);
    
    inline int32_t tick() const {return _tick;}
    inline int32_t duration() const {return _duration;}
    inline int32_t last_tick() const {return tick() + duration();}
    
    inline void set_tick(int32_t tick) {_tick = tick;}
    inline void set_duration(int32_t duration) {_duration = duration;}
    
    inline int8_t pitch() const;
    inline int8_t velocity() const;
    
    inline const RENoteData* data_ptr() const {return _d;}
    
    std::pair<RXNote, RXNote> cut(int tick) const;
    
    std::string to_s() const;
    
    inline RXNote dt(int delta) const {RXNote n = *this; n.inc_tick(delta); return n;}
    inline void inc_tick(int delta) {_tick += delta;}
    
protected:
    void SetData(RENoteData* d);
    void ClearData();
    
protected:
    RENoteData* _d;
    int32_t _tick;
    int32_t _duration;
};

typedef std::pair<RXNote, RXNote> RXNotePair;

typedef std::function<void(const RXNote&)> RXNoteMethod;


/** note_data
 */
class RENoteData
{
    friend class RXNote;
    
public:
    RENoteData() : _refs(0), _pitch(0), _velocity(0) {}
    
    inline int32_t ref_count() const {return _refs;}
    
    std::string to_s() const;
    
protected:
    int32_t _refs;
    int8_t _pitch;
    int8_t _velocity;
};



/** REBeat
 */
class REBeat
{
public:
    typedef REBeat item_type;
    typedef std::pair<REBeat, REBeat> pair_type;
    
public:
    REBeat();
    REBeat(const REBeat& b);
    explicit REBeat(int32_t duration_);
    ~REBeat();
    
    REBeat& operator=(const REBeat& beat);
    
    inline int32_t tick() const {return _tick;}
    inline int32_t duration() const {return _duration;}
    inline int32_t last_tick() const {return tick() + duration();}
    
    inline void set_tick(int32_t tick) {_tick = tick;}
    inline REBeat dt(int delta) const {REBeat b = *this; b.set_tick(b.tick() + delta); return b;}
    
    bool is_rest() const;
    
    inline const REBeatData* raw_data() const {return _d;}
    
    std::string to_s() const;
    std::string to_pretty_s(int spaces=0) const;
    
    REBeat& operator<< (const RXNote& n);
    
    double DispatchNote(const RXNote& n);
    
    REBeat operator/ (int divs) const;
    
    std::pair<REBeat, double> DividedBeat(int divs) const;
    std::pair<REBeat, double> DividedBeatWithTreshold(int divs, int treshold) const;
    
    REBeat BestDividedBeat(int divFlags) const;

    REBeat::pair_type cut(int tick) const;
    
    template <typename F>
    void each_note(F func) const;
    
    template <typename F>
    void each(F func);
    
    template <typename F>
    void EachFinalBeat(F func);
    
    RXNoteVector notes() const;
    
    bool IsBeatGroup() const;
    int SubBeatCount() const;
    const REBeat& SubBeat(int idx) const;
    
    static REBeat BeatGroup(const std::vector<REBeat>& beats);
    
protected:
    void SetData(REBeatData* d);
    void ClearData();
    
    double DispatchNoteWithTreshold(const RXNote& n, int treshold);
    
protected:
    REBeatData* _d;
    int32_t _tick;
    int32_t _duration;
};

typedef std::vector<REBeat> REBeatVector;
typedef REBeat::pair_type REBeatPair;


/** REBeatData
 */
class REBeatData
{
    friend class REBeat;
    
public:
    enum data_type
    {
        note_data_type = 0,
        chord_data_type = 1,
        group_data_type = 2
    };
    
public:
    inline int32_t ref_count() const {return _refs;}
    
    virtual data_type type() const = 0;
    virtual bool is_rest() const = 0;
    
    virtual std::string to_s() const = 0;
    
    virtual void each_note(RXNoteMethod f) const = 0;
    
protected:
    REBeatData() : _refs(0) {}
    virtual ~REBeatData() {}
    
protected:
    int32_t _refs;
};


/** beat::data_note
 */
class REBeatNoteData : public REBeatData
{
public:
    RXNote note;
    
public:
    REBeatNoteData() : note() {}
    REBeatNoteData(const RXNote& n) : note(n) {}
    
    
    virtual data_type type() const {return REBeatData::note_data_type;}
    virtual bool is_rest() const {return false;}
    
    virtual std::string to_s() const;
    
    virtual void each_note(RXNoteMethod f) const;
};

/** beat::data_chord
 */
class REBeatChordData : public REBeatData
{
public:
    RXNoteVector notes;
    
    virtual data_type type() const {return REBeatData::chord_data_type;}
    virtual bool is_rest() const {return notes.empty();}
    
    virtual std::string to_s() const;
    
    virtual void each_note(RXNoteMethod f) const;
};

/** beat::data_group
 */
class REBeatGroupData : public REBeatData
{
public:
    REBeatVector beats;
    int divs;
    uint32_t noteStartingFlags;
    uint32_t noteSoundingFlags;
    
    virtual data_type type() const {return REBeatData::group_data_type;}
    virtual bool is_rest() const;
    
    virtual std::string to_s() const;
    
    virtual void each_note(RXNoteMethod f) const;
};



template <typename F>
void REBeat::each_note(F func) const
{
    if(_d == nullptr) return;
    _d->each_note([&](const RXNote& n_){
        RXNote n = n_.dt(_tick);
        func(n);
    });
}

template <typename F>
void REBeat::each(F func)
{
    if(_d != nullptr && _d->type() == REBeatData::group_data_type)
    {
        REBeatGroupData* d = static_cast<REBeatGroupData*>(_d);
        std::for_each(d->beats.begin(), d->beats.end(), func);
    }
    else {
        func(*this);
    }
}

template <typename F>
void REBeat::EachFinalBeat(F func)
{
    if(_d != nullptr && _d->type() == REBeatData::group_data_type)
    {
        REBeatGroupData* d = static_cast<REBeatGroupData*>(_d);
        std::for_each(d->beats.begin(), d->beats.end(), std::bind(&REBeat::EachFinalBeat, std::placeholders::_1, func));
    }
    else {
        func(*this);
    }
}

int8_t RXNote::pitch() const {return _d ? _d->_pitch : 0;}
int8_t RXNote::velocity() const {return _d ? _d->_velocity : 0;}

typedef RERepeater<REBeat> REBeatRepeater;

#endif /* defined(__Reflow__REBeat__) */
