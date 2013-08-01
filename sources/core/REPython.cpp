//
//  REPython.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 04/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef REFLOW_NO_PYTHON
#include "REPython.h"

#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "RESongController.h"
#include "REScoreController.h"

#include <sstream>

#ifdef REFLOW_MAC
#  include <Python/listobject.h>
#else
#  include "listobject.h"
#endif

#define REPYTHON_BINDING_GET_STRING(_PYCLASS, _GETTER, _MEMBER, _ACCESSOR)  \
static PyObject *                                                           \
_PYCLASS##_get_##_GETTER (REPython::_PYCLASS *me, void *closure)            \
{                                                                           \
    const char* str = me->_MEMBER->_ACCESSOR.c_str();                       \
    PyObject* obj = PyString_FromString(str);                               \
    return obj;                                                             \
}

/** Example: REPYTHON_BINDING_GET_STRING (REPythonSong, title, _song, Title())
 
static PyObject *
REPythonSong_get_title(REPython::REPythonSong *me, void *closure)
{
    const char* str = me->_song->Title().c_str();
    PyObject* obj = PyString_FromString(str);
    return obj;
}
*/

#define REPYTHON_GETSET_REGISTER_START(_PYCLASS)    static PyGetSetDef _PYCLASS##_getseters[] =

#define REPYTHON_REGISTER_GETTER(_PYCLASS, _GETTER) {(char*)#_GETTER, (getter)_PYCLASS##_get_##_GETTER, (setter)NULL, #_GETTER}

#define REPYTHON_GETSET_REGISTER_END(_PYCLASS)      {NULL}

#define REPYTHON_BINDING_DEALLOC(_PYCLASS, _MEMBER) \
static void \
_PYCLASS##_dealloc(REPython::_PYCLASS* self) \
{\
    self->_MEMBER = NULL;\
    self->_controller = NULL;\
}


#define REPYTHON_BINDING_NEW(_PYCLASS, _MEMBER) \
static PyObject * \
_PYCLASS##_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\
{\
    REPython::_PYCLASS *self;\
    \
    self = (REPython::_PYCLASS *)type->tp_alloc(type, 0);\
    if (self != NULL)\
    {\
        self->_MEMBER = NULL;\
        self->_controller = NULL;\
    }\
    \
    return (PyObject *)self;\
}


#define REPYTHON_TYPE_IMPL(_PYCLASS, _NAME, _DOC) \
PyTypeObject REPython::_PYCLASS##Type = \
{ \
    PyObject_HEAD_INIT(NULL) \
    0,                         /*ob_size*/ \
    #_NAME,         /*tp_name*/ \
    sizeof(REPython::_PYCLASS),  /*tp_basicsize*/ \
    0,                         /*tp_itemsize*/ \
    (destructor)_PYCLASS##_dealloc, /*tp_dealloc*/ \
    0,                         /*tp_print*/ \
    0,                         /*tp_getattr*/ \
    0,                         /*tp_setattr*/ \
    0,                         /*tp_compare*/ \
    0,                         /*tp_repr*/ \
    0,                         /*tp_as_number*/ \
    0,                         /*tp_as_sequence*/ \
    0,                         /*tp_as_mapping*/ \
    0,                         /*tp_hash */ \
    0,                         /*tp_call*/ \
    0,                         /*tp_str*/ \
    0,                         /*tp_getattro*/ \
    0,                         /*tp_setattro*/ \
    0,                         /*tp_as_buffer*/ \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/ \
    #_DOC,           /* tp_doc */ \
    0,		               /* tp_traverse */ \
    0,		               /* tp_clear */ \
    0,		               /* tp_richcompare */ \
    0,		               /* tp_weaklistoffset */ \
    0,		               /* tp_iter */ \
    0,		               /* tp_iternext */ \
    0,             /* tp_methods */ \
    0,             /* tp_members */ \
    _PYCLASS##_getseters,           /* tp_getset */ \
    0,                         /* tp_base */ \
    0,                         /* tp_dict */ \
    0,                         /* tp_descr_get */ \
    0,                         /* tp_descr_set */ \
    0,                         /* tp_dictoffset */ \
    0,      /* tp_init */ \
    0,                         /* tp_alloc */ \
    _PYCLASS##_new,                 /* tp_new */ \
};








// ------------------------------------------------------------------------------------------------------------
#pragma Python Song bindings
static PyObject* REPythonSong_get_tracks(REPython::REPythonSong *me, void *closure)
{
    int nbTracks = me->_song->TrackCount();
    PyObject* list = PyList_New(nbTracks);
    
    for(int i=0; i<nbTracks; ++i)
    {
        REPython::REPythonTrack* ptrack = PyObject_New(REPython::REPythonTrack, &REPython::REPythonTrackType);
        ptrack->_controller = me->_controller;
        ptrack->_track = me->_song->Track(i);
        PyList_SetItem(list, i, (PyObject*)ptrack);
    }
    
    return list;
}

REPYTHON_BINDING_DEALLOC(REPythonSong, _song)
REPYTHON_BINDING_NEW(REPythonSong, _song)

REPYTHON_BINDING_GET_STRING (REPythonSong, title, _song, Title())
REPYTHON_BINDING_GET_STRING (REPythonSong, artist, _song, Artist())
REPYTHON_BINDING_GET_STRING (REPythonSong, subtitle, _song, SubTitle())
REPYTHON_BINDING_GET_STRING (REPythonSong, album, _song, Album())
REPYTHON_BINDING_GET_STRING (REPythonSong, music_by, _song, MusicBy())
REPYTHON_BINDING_GET_STRING (REPythonSong, lyrics_by, _song, LyricsBy())
REPYTHON_BINDING_GET_STRING (REPythonSong, transcriber, _song, Transcriber())
REPYTHON_BINDING_GET_STRING (REPythonSong, copyright, _song, Copyright())
REPYTHON_BINDING_GET_STRING (REPythonSong, notice, _song, Notice())

REPYTHON_GETSET_REGISTER_START(REPythonSong)
{
    REPYTHON_REGISTER_GETTER(REPythonSong, title),
    REPYTHON_REGISTER_GETTER(REPythonSong, artist),
    REPYTHON_REGISTER_GETTER(REPythonSong, subtitle),
    REPYTHON_REGISTER_GETTER(REPythonSong, album),
    REPYTHON_REGISTER_GETTER(REPythonSong, music_by),
    REPYTHON_REGISTER_GETTER(REPythonSong, lyrics_by),
    REPYTHON_REGISTER_GETTER(REPythonSong, transcriber),
    REPYTHON_REGISTER_GETTER(REPythonSong, copyright),
    REPYTHON_REGISTER_GETTER(REPythonSong, notice),

    {"tracks", (getter)REPythonSong_get_tracks, (setter)NULL, "track list", NULL},

    REPYTHON_GETSET_REGISTER_END(REPythonSong)
};

REPYTHON_TYPE_IMPL(REPythonSong, "reflow.Song", "Reflow Song Data Object, contains tracks and bars")





// ------------------------------------------------------------------------------------------------------------
#pragma Python Track bindings
static PyObject* REPythonTrack_get_voices(REPython::REPythonTrack *me, void *closure);

REPYTHON_BINDING_DEALLOC(REPythonTrack, _track)
REPYTHON_BINDING_NEW(REPythonTrack, _track)

REPYTHON_BINDING_GET_STRING(REPythonTrack, name, _track, Name())

REPYTHON_GETSET_REGISTER_START(REPythonTrack)
{
    REPYTHON_REGISTER_GETTER(REPythonTrack, name),

    {"voices", (getter)REPythonTrack_get_voices, (setter)NULL, "voices list", NULL},
    
    REPYTHON_GETSET_REGISTER_END(REPythonTrack)
};

REPYTHON_TYPE_IMPL(REPythonTrack, "reflow.Track", "Reflow Track Data Object, contains voices")

static PyObject* REPythonTrack_get_voices(REPython::REPythonTrack *me, void *closure)
{
    int nbVoices = me->_track->VoiceCount();
    PyObject* list = PyList_New(nbVoices);
    
    for(int i=0; i<nbVoices; ++i)
    {
        REPython::REPythonVoice* pvoice = PyObject_New(REPython::REPythonVoice, &REPython::REPythonVoiceType);
        pvoice->_controller = me->_controller;
        pvoice->_voice = me->_track->Voice(i);
        PyList_SetItem(list, i, (PyObject*)pvoice);
    }
    
    return list;
}




// ------------------------------------------------------------------------------------------------------------
#pragma Python Voice bindings
static PyObject* REPythonVoice_get_phrases(REPython::REPythonVoice *me, void *closure);

REPYTHON_BINDING_DEALLOC(REPythonVoice, _voice)
REPYTHON_BINDING_NEW(REPythonVoice, _voice)

REPYTHON_GETSET_REGISTER_START(REPythonVoice)
{
    {"phrases", (getter)REPythonVoice_get_phrases, (setter)NULL, "phrases list", NULL},
    
    REPYTHON_GETSET_REGISTER_END(REPythonVoice)
};

REPYTHON_TYPE_IMPL(REPythonVoice, "reflow.Voice", "Reflow Voice Data Object, contains phrases")

static PyObject* REPythonVoice_get_phrases(REPython::REPythonVoice *me, void *closure)
{
    int nbPhrases = me->_voice->PhraseCount();
    PyObject* list = PyList_New(nbPhrases);
    
    for(int i=0; i<nbPhrases; ++i)
    {
        REPython::REPythonPhrase* pphrase = PyObject_New(REPython::REPythonPhrase, &REPython::REPythonPhraseType);
        pphrase->_controller = me->_controller;
        pphrase->_phrase = me->_voice->Phrase(i);
        PyList_SetItem(list, i, (PyObject*)pphrase);
    }
    
    return list;
}

// ------------------------------------------------------------------------------------------------------------
#pragma Python Phrase bindings
static PyObject* REPythonPhrase_get_chords(REPython::REPythonPhrase *me, void *closure);

REPYTHON_BINDING_DEALLOC(REPythonPhrase, _phrase)
REPYTHON_BINDING_NEW(REPythonPhrase, _phrase)

REPYTHON_GETSET_REGISTER_START(REPythonPhrase)
{
    {"chords", (getter)REPythonPhrase_get_chords, (setter)NULL, "chords list", NULL},
    
    REPYTHON_GETSET_REGISTER_END(REPythonPhrase)
};

REPYTHON_TYPE_IMPL(REPythonPhrase, "reflow.Phrase", "Reflow Phrase Data Object, contains chords")

static PyObject* REPythonPhrase_get_chords(REPython::REPythonPhrase *me, void *closure)
{
    int nbChords = me->_phrase->ChordCount();
    PyObject* list = PyList_New(nbChords);
    
    for(int i=0; i<nbChords; ++i)
    {
        REPython::REPythonChord* pchord = PyObject_New(REPython::REPythonChord, &REPython::REPythonChordType);
        pchord->_controller = me->_controller;
        pchord->_chord = me->_phrase->Chord(i);
        PyList_SetItem(list, i, (PyObject*)pchord);
    }
    
    return list;
}


// ------------------------------------------------------------------------------------------------------------
#pragma Python Chord bindings
static PyObject* REPythonChord_get_notes(REPython::REPythonChord *me, void *closure);

REPYTHON_BINDING_DEALLOC(REPythonChord, _chord)
REPYTHON_BINDING_NEW(REPythonChord, _chord)

REPYTHON_GETSET_REGISTER_START(REPythonChord)
{
    {"notes", (getter)REPythonChord_get_notes, (setter)NULL, "notes list", NULL},
    
    REPYTHON_GETSET_REGISTER_END(REPythonChord)
};

REPYTHON_TYPE_IMPL(REPythonChord, "reflow.Chord", "Reflow Chord Data Object, contains notes")

static PyObject* REPythonChord_get_notes(REPython::REPythonChord *me, void *closure)
{
    int nbNotes = me->_chord->NoteCount();
    PyObject* list = PyList_New(nbNotes);
    
    for(int i=0; i<nbNotes; ++i)
    {
        REPython::REPythonNote* pnote = PyObject_New(REPython::REPythonNote, &REPython::REPythonNoteType);
        pnote->_controller = me->_controller;
        pnote->_note = me->_chord->Note(i);
        PyList_SetItem(list, i, (PyObject*)pnote);
    }
    
    return list;
}


// ------------------------------------------------------------------------------------------------------------
#pragma Python Note bindings

REPYTHON_BINDING_DEALLOC(REPythonNote, _note)
REPYTHON_BINDING_NEW(REPythonNote, _note)

REPYTHON_GETSET_REGISTER_START(REPythonNote)
{
    REPYTHON_GETSET_REGISTER_END(REPythonNote)
};

REPYTHON_TYPE_IMPL(REPythonNote, "reflow.Note", "Reflow Note Data Object")


// ------------------------------------------------------------------------------------------------------------
#pragma Reflow Module
static PyMethodDef module_methods[] = {
    {"current_document",  REPython::REPython_current_document, METH_NOARGS,
        "Get the active document user is working on."},
    {NULL}  /* Sentinel */
};

bool REPython::_Initialized = false;

void REPython::Initialize()
{
    if(_Initialized) return;
    Py_Initialize();
    _Initialized = true;
}

bool REPython::IsInitialized()
{
    return _Initialized;
}

void REPython::AppendPathToSysPath(const std::string& path)
{
    std::ostringstream oss;
    oss << "import sys\n";
    oss << "sys.path.append(\"" << path << "\")\n";
    PyRun_SimpleString(oss.str().c_str());
}

void REPython::RegisterModules()
{
    PyObject* m = Py_InitModule3("reflow", module_methods,
                       "Example module that creates an extension type.");
    if (m == NULL)
        return;
    
    
    PyRun_SimpleString("import sys\n"
                       "print sys.path\n");
    
    // reflow.Document
    InitDocumentAPI(m);
    
    // Register Song Type
    PyType_Ready(&REPython::REPythonSongType);
    Py_INCREF(&REPython::REPythonSongType);
    PyModule_AddObject(m, "Song", (PyObject *)&REPython::REPythonSongType);
    
    // Register Track Type
    PyType_Ready(&REPython::REPythonTrackType);
    Py_INCREF(&REPython::REPythonTrackType);
    PyModule_AddObject(m, "Track", (PyObject *)&REPython::REPythonTrackType);
    
    // Register Voice Type
    PyType_Ready(&REPython::REPythonVoiceType);
    Py_INCREF(&REPython::REPythonVoiceType);
    PyModule_AddObject(m, "Voice", (PyObject *)&REPython::REPythonVoiceType);
    
    // Register Phrase Type
    PyType_Ready(&REPython::REPythonPhraseType);
    Py_INCREF(&REPython::REPythonPhraseType);
    PyModule_AddObject(m, "Phrase", (PyObject *)&REPython::REPythonPhraseType);
    
    // Register Chord Type
    PyType_Ready(&REPython::REPythonChordType);
    Py_INCREF(&REPython::REPythonChordType);
    PyModule_AddObject(m, "Chord", (PyObject *)&REPython::REPythonChordType);
    
    // Register Note Type
    PyType_Ready(&REPython::REPythonNoteType);
    Py_INCREF(&REPython::REPythonNoteType);
    PyModule_AddObject(m, "Note", (PyObject *)&REPython::REPythonNoteType);
}

void REPython::Finalize()
{
    Py_Finalize();
    _Initialized = false;
}
#endif