//
//  REMidiEngine.h
//  Reflow
//
//  Created by Sebastien on 11/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMidiEngine_h
#define Reflow_REMidiEngine_h

#include "RETypes.h"


/** REMidiInputPort class.
 */
class REMidiInputPort
{
    friend class REMidiEngine;
    
public:
    void SetIndex(int index) {_index = index;}
    
    unsigned int Index() const {return _index;}
    unsigned int SourceIndex() const {return _sourceIndex;}
    bool Connected() const {return _connected;}
    
    virtual void ConnectToSource(unsigned int src) = 0;
    virtual void Disconnect() = 0;
    
protected:
    REMidiInputPort();
    
public:
    virtual ~REMidiInputPort() {}
    
protected:
    unsigned int _index;
    unsigned int _sourceIndex;
    bool _connected;
};

typedef std::vector<REMidiInputPort*> REMidiInputPortVector;



/** REMidiEngine class.
 */
class REMidiEngine
{
public:
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;
    
    virtual unsigned int SourceCount() const = 0;
    virtual std::string SourceName(unsigned int src) const = 0;
    virtual int IndexOfSourceNamed(const std::string& name) const = 0;
    
    virtual unsigned int PortCount() const = 0;
    virtual REMidiInputPort* CreatePort() = 0;
    virtual void DestroyPort(unsigned int portIndex) = 0;
    virtual REMidiInputPort* Port(unsigned int portIndex) = 0;
    
    void AddListener(REMidiInputListener* listener);
    void RemoveListener(REMidiInputListener* listener);
    
protected:
    REMidiEngine() {};
    
public:
    virtual ~REMidiEngine() {}
    
protected:
    std::vector<REMidiInputListener*> _listeners;
    REMidiInputPortVector _inputPorts;
};

#endif
