//
//  REAudioSettings.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/10/12.
//
//

#include "REAudioSettings.h"

REAudioSettings REAudioSettings::_defaultSettings = REAudioSettings();

REAudioSettings::REAudioSettings()
: _metronome(false),
  _metronomeGain(1.0),
  _masterGain(1.0),
  _preclickBarCount(0),
  _metronomeBarClickVolume(1.0),
  _metronomeQuarterClickVolume(1.0),
  _metronomeEighthClickVolume(0.0),
  _metronomeTripletClickVolume(0.0),
  _metronomeSixteenthClickVolume(0.0)
{    
}

REAudioSettings::REAudioSettings(const REAudioSettings& settings)
{
    *this = settings;
}

REAudioSettings& REAudioSettings::operator=(const REAudioSettings& settings)
{
    _metronome = settings._metronome;
    _metronomeGain = settings._metronomeGain;
    _masterGain = settings._masterGain;
    _preclickBarCount = settings._preclickBarCount;
    _metronomeBarClickVolume       = settings._metronomeBarClickVolume;
    _metronomeQuarterClickVolume   = settings._metronomeQuarterClickVolume;
    _metronomeEighthClickVolume    = settings._metronomeEighthClickVolume;
    _metronomeTripletClickVolume   = settings._metronomeTripletClickVolume;
    _metronomeSixteenthClickVolume = settings._metronomeSixteenthClickVolume;
    
    return *this;
}

const REAudioSettings& REAudioSettings::DefaultAudioSettings()
{
    return _defaultSettings;
}
