//
//  REArchive.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 14/04/13.
//
//

#ifndef __Reflow__REArchive__
#define __Reflow__REArchive__

#include "RETypes.h"

class REArchive
{
public:
    static void ReadSongV15(RESong& song, REInputStream& decoder, uint32_t version);
    static void ReadBarV15(REBar& bar, REInputStream& decoder, uint32_t version);
    static void ReadTrackV15(RETrack& track, REInputStream& decoder, uint32_t version);
    static void ReadVoiceV15(REVoice& voice, REInputStream& decoder, uint32_t version);
    static void ReadPhraseV15(REPhrase& phrase, REInputStream& decoder, uint32_t version);
    static void ReadChordV15(REChord& chord, REInputStream& decoder, uint32_t version);
    static void ReadChordDiagramV15(REChordDiagram& diagram, REInputStream& decoder, uint32_t version);
    static void ReadScoreSettingsV15(REScoreSettings& settings, REInputStream& decoder, uint32_t version);
    
public:
    static void WriteSongV15(const RESong& song, REOutputStream& coder, uint32_t version);
    static void WriteBarV15(const REBar& bar, REOutputStream& coder, uint32_t version);
    static void WriteTrackV15(const RETrack& track, REOutputStream& coder, uint32_t version);
    static void WriteVoiceV15(const REVoice& voice, REOutputStream& coder, uint32_t version);
    static void WritePhraseV15(const REPhrase& phrase, REOutputStream& coder, uint32_t version);
    static void WriteChordV15(const REChord& chord, REOutputStream& coder, uint32_t version);
    static void WriteChordDiagramV15(const REChordDiagram& diagram, REOutputStream& coder, uint32_t version);
    static void WriteScoreSettingsV15(const REScoreSettings& settings, REOutputStream& coder, uint32_t version);
};

#endif /* defined(__Reflow__REArchive__) */
