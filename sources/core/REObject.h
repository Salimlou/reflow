//
//  REObject.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 05/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REObject_h
#define Reflow_REObject_h

#include "RETypes.h"

class REObject
{
public:
    REObject();
    virtual ~REObject();
    
    REObject* Retain();
    REObject* Release();
    
    int RefCount() const {return _refcount;}
    
private:
    int32_t _refcount;
};


template<class T>
class REObjectRef
{
public:
    typedef REObjectRef<T> REObjectRefType;
    
public:
    REObjectRef() : _object(NULL) {}
    explicit REObjectRef(T* obj) : _object(NULL) {*this = obj;};
    REObjectRef(const REObjectRefType& ref) : _object(NULL) {*this = ref;}
    ~REObjectRef() {SetObject(NULL);}
    
    REObjectRefType& operator= (T* obj) 
    {
        SetObject(obj);
        return *this;
    }
    
    REObjectRefType& operator= (const REObjectRefType& ref) 
    {
        SetObject(ref.Object());
        return *this;
    }
    
    void SetObject(T* obj)
    {
        if(obj) {obj->Retain();}
        if(_object) {
            _object->Release();
        }
        _object = obj;
    }
    
    T* Object() const {return _object;}
    
private:
    T* _object;
};

typedef std::vector<REObjectRef<REObject> > REObjectRefVector;

#endif
