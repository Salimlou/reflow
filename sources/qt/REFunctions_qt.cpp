#include "REFunctions.h"
#include "REScore.h"
#include "RESong.h"

#include <QString>

std::string Reflow::LocalizedTextForLyricsByFrame(const REScore* score)
{
    const RESong* song = score->Song();
    const std::string& lyrics = song->LyricsBy();
    QString lyricsStr = QString("Lyrics by %1").arg(QString::fromUtf8(lyrics.data(), lyrics.size()));
    return lyricsStr.toStdString();
}

std::string Reflow::LocalizedTextForMusicByFrame(const REScore* score)
{
    const RESong* song = score->Song();
    const std::string& music = song->MusicBy();
    QString musicStr = QString("Music by %1").arg(QString::fromUtf8(music.data(), music.size()));
    return musicStr.toStdString();
}

std::string Reflow::LocalizedTextForTranscriberFrame(const REScore* score)
{
    const RESong* song = score->Song();
    const std::string& transcriber = song->Transcriber();
    QString transcriberStr = QString("Transcribed by %1").arg(QString::fromUtf8(transcriber.data(), transcriber.size()));
    return transcriberStr.toStdString();
}
