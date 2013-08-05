QT += core gui widgets xml
CONFIG += c++11
TARGET = Reflow
TEMPLATE = app
RESOURCES += Reflow.qrc
DEFINES += REFLOW_QT
DEFINES += REFLOW_JACK
DEFINES += REFLOW_NO_ICLOUD
DEFINES += REFLOW_NO_FAVORITES
DEFINES += REFLOW_NO_PYTHON
DEFINES += BOOST_MEM_FN_ENABLE_STDCALL
CONFIG(debug, debug|release) {
DEFINES += REFLOW_VERBOSE
}
else {
}
SOURCES += "sources/core/REArchive.cpp" \
    sources/qt/REFilePropertiesDialog.cpp \
    sources/qt/REPianoWidget.cpp \
    sources/qt/REPreferencesDialog.cpp \
    sources/qt/REPartListView.cpp \
    sources/qt/RETextDialog.cpp \
    sources/qt/REClefDialog.cpp \
    sources/qt/REClefPreview.cpp \
    sources/qt/REKeySignatureDialog.cpp \
    sources/qt/REKeySignaturePreview.cpp \
    sources/qt/REBendDialog.cpp \
    sources/qt/RETempoMarkerDialog.cpp \
    sources/qt/REFretboardWidget.cpp
SOURCES += "sources/core/REAudioEngine.cpp"
SOURCES += "sources/core/REAudioExportEngine.cpp"
SOURCES += "sources/core/REAudioSettings.cpp"
SOURCES += "sources/core/REBar.cpp"
SOURCES += "sources/core/REBarMetrics.cpp"
SOURCES += "sources/core/REBeat.cpp"
SOURCES += "sources/core/REBend.cpp"
SOURCES += "sources/core/REChord.cpp"
SOURCES += "sources/core/REChordDiagram.cpp"
SOURCES += "sources/core/REChordFormula.cpp"
SOURCES += "sources/core/REChordFormulaCollection.cpp"
SOURCES += "sources/core/REChordName.cpp"
SOURCES += "sources/core/REClip.cpp"
SOURCES += "sources/core/RECursor.cpp"
SOURCES += "sources/core/REException.cpp"
SOURCES += "sources/core/REFrame.cpp"
SOURCES += "sources/core/REFunctions.cpp"
SOURCES += "sources/core/REGrip.cpp"
SOURCES += "sources/core/REGripCollection.cpp"
SOURCES += "sources/core/REInputStream.cpp"
SOURCES += "sources/core/RELayout.cpp"
SOURCES += "sources/core/RELocator.cpp"
SOURCES += "sources/core/RELogger.cpp"
SOURCES += "sources/core/REManipulator.cpp"
SOURCES += "sources/core/REMidiClip.cpp"
SOURCES += "sources/core/REMidiEngine.cpp"
SOURCES += "sources/core/REMidiFile.cpp"
SOURCES += "sources/core/REMonophonicSynthVoice.cpp"
SOURCES += "sources/core/REMonoSample.cpp"
SOURCES += "sources/core/REMultivoiceIterator.cpp"
SOURCES += "sources/core/REMusicalFont.cpp"
SOURCES += "sources/core/REMusicDevice.cpp"
SOURCES += "sources/core/REMusicRack.cpp"
SOURCES += "sources/core/RENote.cpp"
SOURCES += "sources/core/REObject.cpp"
SOURCES += "sources/core/REOutputStream.cpp"
SOURCES += "sources/core/REPage.cpp"
SOURCES += "sources/core/REPainter.cpp"
SOURCES += "sources/core/REPhrase.cpp"
SOURCES += "sources/core/REPitch.cpp"
SOURCES += "sources/core/REPitchClass.cpp"
SOURCES += "sources/core/REPlaylistBar.cpp"
SOURCES += "sources/core/REPlaylistCompiler.cpp"
SOURCES += "sources/core/REPython.cpp"
SOURCES += "sources/core/RESamplePlayer.cpp"
SOURCES += "sources/core/REScore.cpp"
SOURCES += "sources/core/REScoreController.cpp"
SOURCES += "sources/core/REScoreNode.cpp"
SOURCES += "sources/core/REScoreRoot.cpp"
SOURCES += "sources/core/REScoreSettings.cpp"
SOURCES += "sources/core/RESequencer.cpp"
SOURCES += "sources/core/RESF2Generator.cpp"
SOURCES += "sources/core/RESF2GeneratorPlayer.cpp"
SOURCES += "sources/core/RESF2Patch.cpp"
SOURCES += "sources/core/RESlice.cpp"
SOURCES += "sources/core/RESlur.cpp"
SOURCES += "sources/core/RESong.cpp"
SOURCES += "sources/core/RESongController.cpp"
SOURCES += "sources/core/RESongError.cpp"
SOURCES += "sources/core/RESoundFont.cpp"
SOURCES += "sources/core/RESoundFontManager.cpp"
SOURCES += "sources/core/RESpecialCharacters.cpp"
SOURCES += "sources/core/REStaff.cpp"
SOURCES += "sources/core/REStandardNotationCompiler.cpp"
SOURCES += "sources/core/REStandardStaff.cpp"
SOURCES += "sources/core/REStyle.cpp"
SOURCES += "sources/core/RESVGParser.cpp"
SOURCES += "sources/core/RESymbol.cpp"
SOURCES += "sources/core/RESystem.cpp"
SOURCES += "sources/core/RETablatureStaff.cpp"
SOURCES += "sources/core/RETable.cpp"
SOURCES += "sources/core/RETickRangeModifier.cpp"
SOURCES += "sources/core/RETimeline.cpp"
SOURCES += "sources/core/RETimer.cpp"
SOURCES += "sources/core/RETool.cpp"
SOURCES += "sources/core/RETrack.cpp"
SOURCES += "sources/core/RETrackSet.cpp"
SOURCES += "sources/core/RETypes.cpp"
SOURCES += "sources/core/REViewport.cpp"
SOURCES += "sources/core/REVoice.cpp"
SOURCES += "sources/core/REWavFileWriter.cpp"
SOURCES += "sources/core/REWriteChunkToFile.cpp"
HEADERS += "sources/core/REArchive.h" \
    sources/qt/REFilePropertiesDialog.h \
    sources/qt/REPianoWidget.h \
    sources/qt/REPreferencesDialog.h \
    sources/qt/REPartListView.h \
    sources/qt/RETextDialog.h \
    sources/qt/REClefDialog.h \
    sources/qt/REClefPreview.h \
    sources/qt/REKeySignatureDialog.h \
    sources/qt/REKeySignaturePreview.h \
    sources/qt/REBendDialog.h \
    sources/qt/RETempoMarkerDialog.h \
    sources/qt/REFretboardWidget.h
HEADERS += "sources/core/REAudioEngine.h"
HEADERS += "sources/core/REAudioExportEngine.h"
HEADERS += "sources/core/REAudioSettings.h"
HEADERS += "sources/core/REBar.h"
HEADERS += "sources/core/REBarMetrics.h"
HEADERS += "sources/core/REBeat.h"
HEADERS += "sources/core/REBend.h"
HEADERS += "sources/core/REBezierPath.h"
HEADERS += "sources/core/REChord.h"
HEADERS += "sources/core/REChordDiagram.h"
HEADERS += "sources/core/REChordFormula.h"
HEADERS += "sources/core/REChordFormulaCollection.h"
HEADERS += "sources/core/REChordName.h"
HEADERS += "sources/core/REClip.h"
HEADERS += "sources/core/RECursor.h"
HEADERS += "sources/core/REException.h"
HEADERS += "sources/core/REFrame.h"
HEADERS += "sources/core/REFunctions.h"
HEADERS += "sources/core/REGrip.h"
HEADERS += "sources/core/REGripCollection.h"
HEADERS += "sources/core/REInputStream.h"
HEADERS += "sources/core/RELayout.h"
HEADERS += "sources/core/RELocator.h"
HEADERS += "sources/core/RELogger.h"
HEADERS += "sources/core/REManipulator.h"
HEADERS += "sources/core/REMidiClip.h"
HEADERS += "sources/core/REMidiEngine.h"
HEADERS += "sources/core/REMidiFile.h"
HEADERS += "sources/core/REMonophonicSynthVoice.h"
HEADERS += "sources/core/REMonoSample.h"
HEADERS += "sources/core/REMultivoiceIterator.h"
HEADERS += "sources/core/REMusicalFont.h"
HEADERS += "sources/core/REMusicDevice.h"
HEADERS += "sources/core/REMusicRack.h"
HEADERS += "sources/core/RENote.h"
HEADERS += "sources/core/REObject.h"
HEADERS += "sources/core/REOutputStream.h"
HEADERS += "sources/core/REPage.h"
HEADERS += "sources/core/REPainter.h"
HEADERS += "sources/core/REPhrase.h"
HEADERS += "sources/core/REPitch.h"
HEADERS += "sources/core/REPitchClass.h"
HEADERS += "sources/core/REPlaylistBar.h"
HEADERS += "sources/core/REPlaylistCompiler.h"
HEADERS += "sources/core/REPython.h"
HEADERS += "sources/core/RESampleGenerator.h"
HEADERS += "sources/core/RESamplePlayer.h"
HEADERS += "sources/core/REScore.h"
HEADERS += "sources/core/REScoreController.h"
HEADERS += "sources/core/REScoreNode.h"
HEADERS += "sources/core/REScoreRoot.h"
HEADERS += "sources/core/REScoreSettings.h"
HEADERS += "sources/core/RESequencer.h"
HEADERS += "sources/core/RESF2Generator.h"
HEADERS += "sources/core/RESF2GeneratorPlayer.h"
HEADERS += "sources/core/RESF2Patch.h"
HEADERS += "sources/core/RESlice.h"
HEADERS += "sources/core/RESlur.h"
HEADERS += "sources/core/RESong.h"
HEADERS += "sources/core/RESongController.h"
HEADERS += "sources/core/RESongError.h"
HEADERS += "sources/core/RESoundFont.h"
HEADERS += "sources/core/RESoundFontManager.h"
HEADERS += "sources/core/RESpecialCharacters.h"
HEADERS += "sources/core/REStaff.h"
HEADERS += "sources/core/REStandardNotationCompiler.h"
HEADERS += "sources/core/REStandardStaff.h"
HEADERS += "sources/core/REStyle.h"
HEADERS += "sources/core/RESVGParser.h"
HEADERS += "sources/core/RESymbol.h"
HEADERS += "sources/core/RESystem.h"
HEADERS += "sources/core/RETablatureStaff.h"
HEADERS += "sources/core/RETable.h"
HEADERS += "sources/core/RETickRangeModifier.h"
HEADERS += "sources/core/RETimeline.h"
HEADERS += "sources/core/RETimer.h"
HEADERS += "sources/core/RETool.h"
HEADERS += "sources/core/RETrack.h"
HEADERS += "sources/core/RETrackSet.h"
HEADERS += "sources/core/RETypes.h"
HEADERS += "sources/core/REViewport.h"
HEADERS += "sources/core/REVoice.h"
HEADERS += "sources/core/REWavFileWriter.h"
HEADERS += "sources/core/REWriteChunkToFile.h"
HEADERS += "sources/core/REXMLParser.h"
SOURCES += "sources/plugins/guitarpro/REGuitarProParser.cpp"
SOURCES += "sources/plugins/guitarpro/REGuitarProWriter.cpp"
HEADERS += "sources/plugins/guitarpro/REGuitarProParser.h"
HEADERS += "sources/plugins/guitarpro/REGuitarProWriter.h"
SOURCES += "sources/qt/main.cpp"
SOURCES += "sources/qt/REBezierPath_qt.cpp"
SOURCES += "sources/qt/RECreateTrackDialog.cpp"
SOURCES += "sources/qt/REDocumentView.cpp"
SOURCES += "sources/qt/REFunctions_qt.cpp"
SOURCES += "sources/qt/REGraphicsFrameItem.cpp"
SOURCES += "sources/qt/REGraphicsPageItem.cpp"
SOURCES += "sources/qt/REGraphicsSliceItem.cpp"
SOURCES += "sources/qt/REGraphicsSystemItem.cpp"
SOURCES += "sources/qt/REJackAudioEngine.cpp"
SOURCES += "sources/qt/REMainWindow.cpp"
SOURCES += "sources/qt/REMixerHeaderWidget.cpp"
SOURCES += "sources/qt/REMixerRowWidget.cpp"
SOURCES += "sources/qt/REMixerWidget.cpp"
SOURCES += "sources/qt/REMusicalFont_qt.cpp"
SOURCES += "sources/qt/RENavigatorHeaderWidget.cpp"
SOURCES += "sources/qt/RENavigatorScene.cpp"
SOURCES += "sources/qt/REPainter_qt.cpp"
SOURCES += "sources/qt/REPartListModel.cpp"
SOURCES += "sources/qt/REPlaybackCursorItem.cpp"
SOURCES += "sources/qt/REPropertiesDialog.cpp"
SOURCES += "sources/qt/REQtPalette.cpp"
SOURCES += "sources/qt/REQtViewport.cpp"
SOURCES += "sources/qt/RERehearsalDialog.cpp"
SOURCES += "sources/qt/RERepeatDialog.cpp"
SOURCES += "sources/qt/RERtAudioEngine.cpp"
SOURCES += "sources/qt/REScoreScene.cpp"
SOURCES += "sources/qt/REScoreSceneView.cpp"
SOURCES += "sources/qt/RESectionListModel.cpp"
SOURCES += "sources/qt/RESequencerWidget.cpp"
SOURCES += "sources/qt/RESoundFontManager_qt.cpp"
SOURCES += "sources/qt/REStringTuningWidget.cpp"
SOURCES += "sources/qt/RETabCursorItem.cpp"
SOURCES += "sources/qt/RETimeSignatureDialog.cpp"
SOURCES += "sources/qt/RETrackListModel.cpp"
SOURCES += "sources/qt/RETrackListView.cpp"
SOURCES += "sources/qt/RETransportWidget.cpp"
SOURCES += "sources/qt/RETuningDialog.cpp"
SOURCES += "sources/qt/REUndoCommand.cpp"
SOURCES += "sources/qt/REXmlParser_qt.cpp"
HEADERS += "sources/qt/RECreateTrackDialog.h"
HEADERS += "sources/qt/REDocumentView.h"
HEADERS += "sources/qt/REGraphicsFrameItem.h"
HEADERS += "sources/qt/REGraphicsPageItem.h"
HEADERS += "sources/qt/REGraphicsSliceItem.h"
HEADERS += "sources/qt/REGraphicsSystemItem.h"
HEADERS += "sources/qt/REJackAudioEngine.h"
HEADERS += "sources/qt/REMainWindow.h"
HEADERS += "sources/qt/REMixerHeaderWidget.h"
HEADERS += "sources/qt/REMixerRowWidget.h"
HEADERS += "sources/qt/REMixerWidget.h"
HEADERS += "sources/qt/RENavigatorHeaderWidget.h"
HEADERS += "sources/qt/RENavigatorScene.h"
HEADERS += "sources/qt/REPartListModel.h"
HEADERS += "sources/qt/REPlaybackCursorItem.h"
HEADERS += "sources/qt/REPropertiesDialog.h"
HEADERS += "sources/qt/REQtPalette.h"
HEADERS += "sources/qt/REQtViewport.h"
HEADERS += "sources/qt/RERehearsalDialog.h"
HEADERS += "sources/qt/RERepeatDialog.h"
HEADERS += "sources/qt/RERtAudioEngine.h"
HEADERS += "sources/qt/REScoreScene.h"
HEADERS += "sources/qt/REScoreSceneView.h"
HEADERS += "sources/qt/RESectionListModel.h"
HEADERS += "sources/qt/RESequencerWidget.h"
HEADERS += "sources/qt/REStringTuningWidget.h"
HEADERS += "sources/qt/RETabCursorItem.h"
HEADERS += "sources/qt/RETimeSignatureDialog.h"
HEADERS += "sources/qt/RETrackListModel.h"
HEADERS += "sources/qt/RETrackListView.h"
HEADERS += "sources/qt/RETransportWidget.h"
HEADERS += "sources/qt/RETuningDialog.h"
HEADERS += "sources/qt/REUndoCommand.h"
FORMS += "sources/qt/RECreateTrackDialog.ui" \
    sources/qt/REFilePropertiesDialog.ui \
    sources/qt/REPreferencesDialog.ui \
    sources/qt/RETextDialog.ui \
    sources/qt/REClefDialog.ui \
    sources/qt/REKeySignatureDialog.ui \
    sources/qt/REBendDialog.ui \
    sources/qt/RETempoMarkerDialog.ui
FORMS += "sources/qt/REMainWindow.ui"
FORMS += "sources/qt/REMixerHeaderWidget.ui"
FORMS += "sources/qt/REMixerRowWidget.ui"
FORMS += "sources/qt/REPropertiesDialog.ui"
FORMS += "sources/qt/RERehearsalDialog.ui"
FORMS += "sources/qt/RERepeatDialog.ui"
FORMS += "sources/qt/REStringTuningWidget.ui"
FORMS += "sources/qt/RETimeSignatureDialog.ui"
FORMS += "sources/qt/RETransportWidget.ui"
FORMS += "sources/qt/RETuningDialog.ui"
SOURCES += "depends/rtaudio/RtAudio.cpp"
HEADERS += "depends/rtaudio/RtAudio.h"
INCLUDEPATH += "depends/rapidjson/include"
INCLUDEPATH += "depends/rtaudio"
INCLUDEPATH += "sources/core"
INCLUDEPATH += "sources/plugins/guitarpro"
INCLUDEPATH += "sources/qt"
INCLUDEPATH += "depends"
win32 {
	INCLUDEPATH += "C:\boost_1_52_0"
    INCLUDEPATH += "C:\Program Files (x86)\Jack\includes"
	DEFINES += BOOST_MEM_FN_ENABLE_STDCALL
	DEFINES += __WINDOWS_DS__
    QMAKE_LIBDIR += "C:\Program Files (x86)\Jack\lib"
  LIBS += dsound.lib ole32.lib libjack.lib
}
macx {
  INCLUDEPATH += /opt/Boost
  INCLUDEPATH += /usr/local/include
  DEFINES += __MACOSX_CORE__
  DEFINES += MACOSX
  LIBS += -lpthread -L/usr/local/lib -ljack -framework CoreAudio -framework CoreFoundation
}
linux-clang {
    DEFINES += LINUX __LINUX_PULSE__
    QMAKE_CXXFLAGS += -std=c++11
    LIBS += -lpulse -lpulse-simple -ljack -lpthread -lrt
}
