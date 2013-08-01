//
//  REPython.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 04/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef REFLOW_NO_PYTHON

#ifndef Reflow_REPython_h
#define Reflow_REPython_h

#include "RETypes.h"

#ifdef REFLOW_MAC
#   include <Python/Python.h>
#else
#   include "Python.h"
#endif

#define REPYTHON_DECL(_PYCLASS, _RECLASS, _MEMBER)      typedef struct \
{\
    PyObject_HEAD \
    const _RECLASS* _MEMBER; \
    REScoreController* _controller; \
} _PYCLASS; \
    static PyTypeObject _PYCLASS##Type;

class REPython
{
public:
    static bool IsInitialized();
    static void Initialize();
    static void AppendPathToSysPath(const std::string& path);
    static void RegisterModules();
    static void Finalize();
    
protected:
    static void InitDocumentAPI(PyObject* reflowModule);
    
public:
    REPYTHON_DECL(REPythonSong, RESong, _song);
    REPYTHON_DECL(REPythonTrack, RETrack, _track);
    REPYTHON_DECL(REPythonBar, REBar, _bar);
    REPYTHON_DECL(REPythonVoice, REVoice, _voice);
    REPYTHON_DECL(REPythonPhrase, REPhrase, _phrase);
    REPYTHON_DECL(REPythonChord, REChord, _chord);
    REPYTHON_DECL(REPythonNote, RENote, _note);
    
public:
    static PyObject* REPython_current_document(PyObject*, PyObject*);
    
private:
    static bool _Initialized;
};

#endif

#endif