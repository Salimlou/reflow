//
//  REMidiFile.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 13/02/13.
//
//

#ifndef __Reflow__REMidiFile__
#define __Reflow__REMidiFile__

#include "RETypes.h"
#include "REClip.h"
#include "REBeat.h"

#ifndef Q_MOC_RUN
#include <boost/array.hpp>
#endif

class REMidiFileMetaEvent
{
    friend class REMidiFile;
    friend class REMidiFileTrack;
    
public:
    REMidiFileMetaEvent() {}
    
    inline unsigned int Tick() const {return _tick;}
    inline unsigned char Type() const {return _type;}
    inline const std::string& Data() const {return _data;}
    
private:
    uint32_t _tick;
    unsigned char _type;
    std::string _data;
};
typedef std::vector<REMidiFileMetaEvent> REMidiFileMetaEventVector;



class REMidiFileBar
{
    friend class REMidiFile;
    friend class REMidiFileTrack;
    
public:
    unsigned int Tick() const {return _tick;}
    unsigned int Duration() const {return _duration;}
    const RETimeSignature& TimeSignature() const {return _timeSignature;}
    
private:
    uint32_t _tick;
    uint32_t _duration;
    RETimeSignature _timeSignature;
};
typedef std::vector<REMidiFileBar> REMidiFileBarVector;



class REMidiFileTrack
{
    friend class REMidiFile;
    
public:
    bool ReadEvents(const char* bytes, int length);
    
    const REMidiFileMetaEvent* TimeSignatureEventAtTick(int tick) const;
    const REMidiEvent* ProgramChangeEventAtTick(int tick, int channel) const;
    
    int BarCount() const {return _bars.size();}
    const REMidiFileBar* Bar(int idx) const;
    
    const REBeat& MainBeat() const {return _mainBeat;}
    void SetMainBeat(REBeat mb) {_mainBeat = mb;}
    
    RETrack* ImportTrackMerged(int nbBars) const;
    RETrack* ImportTrack(int nbBars, int channel) const;
    int NoteCountOfChannel(int channel) const;
    
    int ChannelIndexWithMostNotes() const;
    
protected:
    REMidiFileTrack();
    
    int _ComputeBars(int ppqn, int maxTick);
    void _ImportNotesMerged(const REMidiFileTrack& tempoTrack, int ppqn, int maxTick);
    void _ImportNotes(const REMidiFileTrack& tempoTrack, int ppqn, int maxTick, int channel);
    void _ImportTrackFromBeat(RETrack* track, int nbBars, const REBeat& mainBeat) const;
    
    REPhrase* ImportPhrase(const RETrack* track, REBeat beat) const;
    
protected:
    int _index;
    int32_t _tempo;
    REMidiFileMetaEventVector _metaEvents;
    REMidiFileBarVector _bars;
    REBeat _mainBeat;
    boost::array<REBeat, 16> _mainBeatPerChannel;
    std::string _name;
    uint32_t _lastTick;
    boost::array<REMidiEventVector, 16> _midiEventsPerChannel;
    boost::array<int, 16> _noteCountPerChannel;
};
typedef std::vector<REMidiFileTrack*> REMidiFileTrackVector;


struct REMidiFileLoadOptions
{
    REMidiFileLoadOptions() : mergeChannels(true) {}
    
    bool mergeChannels;
};


class REMidiFile
{
public:
    REMidiFile();
    ~REMidiFile();
    
    void Load(REInputStream& stream, const REMidiFileLoadOptions& options);
    void Load(const std::string& filename, const REMidiFileLoadOptions& options);
    
    bool IsOK() {return _error.empty();}
    const std::string& Error() const {return _error;}
    
    int TrackCount() const {return _tracks.size();}
    const REMidiFileTrackVector& Tracks() const {return _tracks;}
    const REMidiFileTrack* Track(int idx) const;
    
    RESong* ImportSong() const;
    
private:
    void Clear();
    
    
private:
    std::string _error;
    bool _verbose;
    uint32_t _quarterDiv;
    uint32_t _lastTick;
    REMidiFileTrackVector _tracks;
    REMidiFileLoadOptions _options;
};

#endif /* defined(__Reflow__REMidiFile__) */
