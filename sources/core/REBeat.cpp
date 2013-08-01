//
//  REBeat.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 13/02/13.
//
//

#include <sstream>

#include "REBeat.h"

#pragma mark RXNote
RXNote::RXNote()
: _d(nullptr), _tick(0), _duration(0)
{}

RXNote::RXNote(const RXNote& rhs)
: _d(nullptr), _tick(rhs.tick()), _duration(rhs.duration())
{
    SetData(rhs._d);
}

RXNote::RXNote(int8_t p)
: _d(nullptr), _tick(0), _duration(0)
{
    _d = new RENoteData;
    _d->_refs = 1;
    _d->_pitch = p;
}

RXNote::RXNote(int32_t tick, int32_t duration, int8_t pitch, int8_t velocity)
: _d(new RENoteData), _tick(tick), _duration(duration)
{
    _d->_refs = 1;
    _d->_pitch = pitch;
    _d->_velocity = velocity;
}

RXNote::~RXNote() {ClearData();}

RXNote& RXNote::operator=(const RXNote& rhs)
{
    _tick = rhs.tick();
    _duration = rhs.duration();
    SetData(rhs._d);
    return *this;
}

void RXNote::SetData(RENoteData* d)
{
    if(_d == d) return;
    ClearData();
    ++d->_refs;
    _d = d;
}

void RXNote::ClearData()
{
    if(_d && --_d->_refs == 0) {
        delete _d;
        _d = nullptr;
    }
}

// t should be in [0 .. duration[
std::pair<RXNote, RXNote> RXNote::cut(int t) const
{
    if(t <= 0) {
        return RXNotePair(RXNote(), *this);
    }
    else if(t >= duration()) {
        return RXNotePair(*this, RXNote());
    }
    else {
        RXNote n1 = *this;
        n1.set_duration(t);
        
        RXNote n2 = *this;
        n2.set_tick(tick() + t);
        n2.set_duration(duration() - t);
        
        return RXNotePair(n1, n2);
    }
}

std::string RXNote::to_s() const
{
    std::ostringstream oss;
    oss << "{tick:" << _tick << ", duration:" << _duration << ", data: " << (_d ? _d->to_s() : "{}") << "}";
    return oss.str();
}

std::string RENoteData::to_s() const
{
    std::ostringstream oss;
    oss << "{pitch: " << (int)_pitch << ", velocity:" << (int)_velocity << ", refs:" << _refs << "}";
    return oss.str();
}


#pragma mark REBeat
REBeat::REBeat()
: _d(nullptr), _tick(0), _duration(0)
{}

REBeat::REBeat(const REBeat& b)
: _d(nullptr)
{
    *this = b;
}

REBeat::REBeat(int32_t duration_)
: _d(nullptr), _tick(0), _duration(duration_)
{}

REBeat::~REBeat()
{
    ClearData();
}

REBeat& REBeat::operator=(const REBeat& b)
{
    _tick = b._tick;
    _duration = b._duration;
    SetData(b._d);
    return *this;
}

bool REBeat::is_rest() const
{
    return _d == nullptr || _d->is_rest();
}

void REBeat::SetData(REBeatData* d)
{
    if(_d == d) return;
    ClearData();
    if(d)
    {
        ++d->_refs;
        _d = d;
    }
    else {
        _d = nullptr;
    }
}

void REBeat::ClearData()
{
    if(_d && --_d->_refs == 0) {
        delete _d;
        _d = nullptr;
    }
}

double REBeat::DispatchNote(const RXNote& n)
{
    int dt = -this->tick();
    RXNote local_note = n.dt(dt);
 
    double err = (local_note.tick() * local_note.tick());
    
    if(_d == nullptr)
    {
        SetData(new REBeatNoteData(local_note));
    }
    else if (_d->type() == REBeatData::note_data_type)
    {
        REBeatNoteData* dn = static_cast<REBeatNoteData*>(_d);
        REBeatChordData* dc = new REBeatChordData;
        dc->notes.push_back(dn->note);
        dc->notes.push_back(local_note);
        SetData(dc);
    }
    else if(_d->type() == REBeatData::chord_data_type)
    {
        REBeatChordData* dc = static_cast<REBeatChordData*>(_d);
        dc->notes.push_back(local_note);
    }
    else if(_d->type() == REBeatData::group_data_type)
    {
        REBeatGroupData* dg = static_cast<REBeatGroupData*>(_d);
        
        // round tick
        int tk = local_note.tick();
        if(tk < 0) {
            err = dg->beats.front().DispatchNote(local_note);
        }
        else if(tk >= _duration) {
            err = dg->beats.back().DispatchNote(local_note);
        }
        else {
            int quant = duration() / dg->divs;
            int rtk = ((tk + (quant / 2)) / quant) * quant;
            size_t nb_divs = dg->beats.size();
            bool found = false;
            for(size_t i=1; i<nb_divs; ++i) {
                REBeat& dgb = dg->beats[i];
                if(dgb.tick() > rtk) {
                    err = dg->beats[i-1].DispatchNote(local_note);
                    found = true;
                    break;
                }
            }
            if(!found) {
                err = dg->beats.back().DispatchNote(local_note);
            }
        }
        
        
    }
    return err;
}

double REBeat::DispatchNoteWithTreshold(const RXNote& n, int treshold)
{
    int dt = -this->tick();
    RXNote local_note = n.dt(dt);
    
    double err = (local_note.tick() * local_note.tick());
    
    if(_d == nullptr)
    {
        SetData(new REBeatNoteData(local_note));
    }
    else if (_d->type() == REBeatData::note_data_type)
    {
        REBeatNoteData* dn = static_cast<REBeatNoteData*>(_d);
        REBeatChordData* dc = new REBeatChordData;
        dc->notes.push_back(dn->note);
        dc->notes.push_back(local_note);
        SetData(dc);
    }
    else if(_d->type() == REBeatData::chord_data_type)
    {
        REBeatChordData* dc = static_cast<REBeatChordData*>(_d);
        dc->notes.push_back(local_note);
    }
    else if(_d->type() == REBeatData::group_data_type)
    {
        REBeatGroupData* dg = static_cast<REBeatGroupData*>(_d);
        
        // round tick
        int tk = local_note.tick();
        if(tk < 0) {
            err = dg->beats.front().DispatchNote(local_note);
            dg->noteStartingFlags |= (1 << 0);
            dg->noteSoundingFlags |= (1 << 0);
        }
        else if(tk >= _duration) {
            err = dg->beats.back().DispatchNote(local_note);
            dg->noteStartingFlags |= (1 << (dg->beats.size()-1));
            dg->noteSoundingFlags |= (1 << (dg->beats.size()-1));
        }
        else {
            int quant = duration() / dg->divs;
            int rtk = ((tk + treshold) / quant) * quant;
            int rtk2 = ((tk + local_note.duration() + treshold) / quant) * quant;
            size_t nb_divs = dg->beats.size();
            bool found = false;
            
            size_t i = 1;
            for(; i<nb_divs; ++i) {
                REBeat& dgb = dg->beats[i];
                if(dgb.tick() > rtk) {
                    err = dg->beats[i-1].DispatchNote(local_note);
                    dg->noteStartingFlags |= (1 << (i-1));
                    dg->noteSoundingFlags |= (1 << (i-1));
                    found = true;
                    break;
                }
            }
            
            if(found)
            {
                size_t j = i;
                for(; j<nb_divs; ++j)
                {
                    REBeat& dgb = dg->beats[j];
                    if(dgb.tick() > rtk2)
                    {
                        // Beats [i .. j] have sounding flags
                        for(size_t k=i; k<=j; ++k) {
                            dg->noteSoundingFlags |= (1 << k);
                        }
                        break;
                    }
                }
            }
            
            if(!found) {
                err = dg->beats.back().DispatchNote(local_note);
                dg->noteStartingFlags |= (1 << (dg->beats.size()-1));
                dg->noteSoundingFlags |= (1 << (dg->beats.size()-1));
            }
        }
    }
    return err;
}

REBeat REBeat::BeatGroup(const std::vector<REBeat>& beats)
{
    REBeatGroupData* dg = new REBeatGroupData;
    dg->beats.reserve(beats.size());
    int tick = 0;
    for(REBeat b : beats)
    {
        b.set_tick(tick);
        dg->beats.push_back(b);
        tick += b.duration();
    }
    
    REBeat newBeat(tick);
    newBeat.SetData(dg);
    return newBeat;
}

REBeat& REBeat::operator<< (const RXNote& n)
{
    DispatchNote(n);
    return *this;
}

std::pair<REBeat, double> REBeat::DividedBeat(int divs) const
{
    if(divs < 1) return std::pair<REBeat, double>(REBeat(*this), 0.0);
    
    double accumulatedError = 0.0;
    RXNoteVector backup_notes = notes();
    
    REBeat divided_beat(duration());
    divided_beat.set_tick(tick());
    REBeatGroupData* dg = new REBeatGroupData;
    dg->beats.reserve(divs);
    dg->divs = divs;
    for(int i=0; i<divs; ++i)
    {
        int btick0 = (i * _duration) / divs;
        int btick1 = ((i+1) * _duration) / divs;
        int bdur = btick1 - btick0;
        
        REBeat b(bdur);
        b.set_tick(btick0);
        dg->beats.push_back(b);
    }
    divided_beat.SetData(dg);
    
    // dispatch notes
    for(const RXNote& n : backup_notes) {
        accumulatedError += divided_beat.DispatchNote(n);
    }
    return std::pair<REBeat, double>(divided_beat, accumulatedError);
}

std::pair<REBeat, double> REBeat::DividedBeatWithTreshold(int divs, int treshold) const
{
    if(divs < 1) return std::pair<REBeat, double>(REBeat(*this), 0.0);
    
    double accumulatedError = 0.0;
    RXNoteVector backup_notes = notes();
    
    REBeat divided_beat(duration());
    divided_beat.set_tick(tick());
    REBeatGroupData* dg = new REBeatGroupData;
    dg->beats.reserve(divs);
    dg->divs = divs;
    for(int i=0; i<divs; ++i)
    {
        int btick0 = (i * _duration) / divs;
        int btick1 = ((i+1) * _duration) / divs;
        int bdur = btick1 - btick0;
        
        REBeat b(bdur);
        b.set_tick(btick0);
        dg->beats.push_back(b);
    }
    divided_beat.SetData(dg);
    
    // dispatch notes
    for(const RXNote& n : backup_notes) {
        accumulatedError += divided_beat.DispatchNoteWithTreshold(n, treshold);
    }
    return std::pair<REBeat, double>(divided_beat, accumulatedError);
}

REBeat REBeat::operator/ (int divs) const
{
    return DividedBeat(divs).first;
}

REBeat REBeat::BestDividedBeat(int divFlags) const
{
    double err = 1.e9;
    REBeat dbeat;
    
    for(int divs = 1; divs <= 16; ++divs)
    {
        if(divFlags & (1<<divs))
        {
            std::pair<REBeat, double> db = DividedBeat(divs);
            if(db.second < err)
            {
                err = db.second;
                dbeat = db.first;
            }
        }
    }
    
    return dbeat;
}

// t should be in [0 .. duration[
REBeat::pair_type REBeat::cut(int t) const
{
    if(t <= 0) {
        return REBeatPair(REBeat(), *this);
    }
    else if(t >= duration()) {
        return REBeatPair(*this, REBeat());
    }
    else
    {
        if(_d == nullptr)
        {
            REBeat b1 = *this;
            b1._duration = t;
            
            REBeat b2 = *this;
            b2.set_tick(tick() + t);
            b2._duration = duration() - t;
            
            return REBeatPair(b1, b2);
        }
        else
        {
            REBeat b1(t);
            b1.set_tick(_tick);
            
            REBeat b2(duration() - t);
            b2.set_tick(_tick + t);
            
            if(_d->type() == REBeatData::note_data_type)
            {
                const REBeatNoteData* d = static_cast<const REBeatNoteData*>(_d);
                RXNotePair notes = d->note.cut(t - d->note.tick());
                
                if(notes.first.duration()) b1 << notes.first.dt(_tick);
                if(notes.second.duration()) b2 << notes.second.dt(_tick);
            }
            else if(_d->type() == REBeatData::chord_data_type)
            {
                const REBeatChordData* d = static_cast<const REBeatChordData*>(_d);
                
                for(const RXNote& note : d->notes)
                {
                    RXNotePair notes = note.cut(t - note.tick());
                    if(notes.first.duration()) b1 << notes.first.dt(_tick);
                    if(notes.second.duration()) b2 << notes.second.dt(_tick);
                }
            }
            else if(_d->type() == REBeatData::group_data_type)
            {
                const REBeatGroupData* d = static_cast<const REBeatGroupData*>(_d);
                
                REBeatGroupData* dg1 = new REBeatGroupData();
                dg1->divs = d->divs;
                
                REBeatGroupData* dg2 = new REBeatGroupData();
                dg2->divs = d->divs;
                
                for(const REBeat& b : d->beats)
                {
                    REBeatPair beats = b.cut(t - b.tick());
                    if(beats.first.duration()) dg1->beats.push_back(beats.first);
                    if(beats.second.duration()) dg2->beats.push_back(beats.second.dt(-t));
                }
                
                b1.SetData(dg1);
                b2.SetData(dg2);
            }
            
            return REBeatPair(b1, b2);
        }
    }
}

std::string REBeat::to_s() const
{
    std::ostringstream oss;
    oss << "{tick:" << _tick << ", duration:" << _duration << ", data: " << (_d ? _d->to_s() : "{}") << ")";
    return oss.str();
}

std::string REBeat::to_pretty_s(int spaces) const
{
    std::ostringstream oss;
    oss << std::string(spaces,   ' ') << "{" << std::endl;
    oss << std::string(spaces+2, ' ') << "tick:" << _tick << ", " << std::endl;
    oss << std::string(spaces+2, ' ') << "duration:" << _duration << ", " << std::endl;
    oss << std::string(spaces+2, ' ') << "data: " << std::endl;
    oss << std::string(spaces+2, ' ') << (_d ? _d->to_s() : "{}") << std::endl;
    oss << std::string(spaces,   ' ') << "}" << std::endl;
    return oss.str();
}

RXNoteVector REBeat::notes() const
{
    RXNoteVector notes_;
    each_note([&](const RXNote& n) {notes_.push_back(n);});
    return notes_;
}

bool REBeat::IsBeatGroup() const
{
    return _d && _d->type() == REBeatData::group_data_type;
}

int REBeat::SubBeatCount() const
{
    if(_d && _d->type() == REBeatData::group_data_type)
    {
        const REBeatVector& beats = static_cast<const REBeatGroupData*>(_d)->beats;
        return beats.size();
    }
    return 0;
}
const REBeat& REBeat::SubBeat(int idx) const
{
    assert(_d && _d->type() == REBeatData::group_data_type);
    
    const REBeatVector& beats = static_cast<const REBeatGroupData*>(_d)->beats;
    assert(idx >= 0 && idx < beats.size());
    return beats[idx];
}


std::string REBeatNoteData::to_s() const
{
    std::ostringstream oss;
    oss << "{note : " << note.to_s() << "}";
    return oss.str();
}

void REBeatNoteData::each_note(RXNoteMethod f) const
{
    f(this->note);
}

std::string REBeatChordData::to_s() const
{
    std::ostringstream oss;
    oss << "{notes : [";
    bool first = true;
    for(const RXNote& n : notes)
    {
        if(!first)
            oss << ", ";
        else first = false;
        oss << n.to_s();
    }
    oss << "]}";
    return oss.str();
}

void REBeatChordData::each_note(RXNoteMethod f) const
{
    for(const RXNote& n : notes) {
        f(n);
    }
}

bool REBeatGroupData::is_rest() const
{
    for(const REBeat& b : beats) {if(!b.is_rest()) return false;}
    return true;
}

std::string REBeatGroupData::to_s() const
{
    std::ostringstream oss;
    oss << "{beats : [";
    bool first = true;
    for(const REBeat& b : beats)
    {
        if(!first) oss << ", ";
        else first = false;
        
        oss << b.to_s();
    }
    oss << "]}";
    return oss.str();
}

void REBeatGroupData::each_note(RXNoteMethod f) const
{
    for(const REBeat& b : beats) {
        b.each_note(f);
    }
}

