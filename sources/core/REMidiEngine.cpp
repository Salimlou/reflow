#include "REMidiEngine.h"


void REMidiEngine::AddListener(REMidiInputListener* listener)
{
    _listeners.push_back(listener);
}
void REMidiEngine::RemoveListener(REMidiInputListener* listener)
{
    _listeners.erase(std::find(_listeners.begin(), _listeners.end(), listener), _listeners.end());
}


REMidiInputPort::REMidiInputPort()
: _index(0), _connected(false), _sourceIndex(0)
{
    
}







