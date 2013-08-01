//
//  REAudioSettings.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/10/12.
//
//

#ifndef __Reflow__REAudioSettings__
#define __Reflow__REAudioSettings__

#include "RETypes.h"

class REAudioSettings
{
public:
    REAudioSettings();
    REAudioSettings(const REAudioSettings& settings);
    
    REAudioSettings& operator=(const REAudioSettings& settings);
    
    void SetMasterGain(double gain) {_masterGain = gain;}
    double MasterGain() const {return _masterGain;}
    
    void SetMetronomeGain(double gain) {_metronomeGain = gain;}
    double MetronomeGain() const {return _metronomeGain;}
    
    bool MetronomeEnabled() const {return _metronome;}
    void SetMetronomeEnabled(bool metronome) {_metronome = metronome;}
    
    double MetronomeBarClickVolume() const {return _metronomeBarClickVolume;}
    double MetronomeQuarterClickVolume() const {return _metronomeQuarterClickVolume;}
    double MetronomeEighthClickVolume() const {return _metronomeEighthClickVolume;}
    double MetronomeTripletClickVolume() const {return _metronomeTripletClickVolume;}
    double MetronomeSixteenthClickVolume() const {return _metronomeSixteenthClickVolume;}
    
    void SetMetronomeBarClickVolume(double vol) {_metronomeBarClickVolume = vol;}
    void SetMetronomeQuarterClickVolume(double vol) {_metronomeQuarterClickVolume = vol;}
    void SetMetronomeEighthClickVolume(double vol) {_metronomeEighthClickVolume = vol;}
    void SetMetronomeTripletClickVolume(double vol) {_metronomeTripletClickVolume = vol;}
    void SetMetronomeSixteenthClickVolume(double vol) {_metronomeSixteenthClickVolume = vol;}
    
    int PreclickBarCount() const {return _preclickBarCount;}
    void SetPreclickBarCount(int bc) {_preclickBarCount = bc;}
    
    static const REAudioSettings& DefaultAudioSettings();
    
private:
    double _masterGain;
    double _metronomeGain;
    bool _metronome;
    int _preclickBarCount;
    
    double _metronomeBarClickVolume;
    double _metronomeQuarterClickVolume;
    double _metronomeEighthClickVolume;
    double _metronomeTripletClickVolume;
    double _metronomeSixteenthClickVolume;
    
    static REAudioSettings _defaultSettings;
};

#endif /* defined(__Reflow__REAudioSettings__) */
