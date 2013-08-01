//
//  REOutputStream.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RECODER_H_
#define _RECODER_H_

#include <string>
#ifdef WIN32
#  include <cstdint>
#endif

#include "RETypes.h"

class REOutputStream
{
public:
	virtual void Write(const char* bytes, unsigned long size) = 0;
	virtual const char* Data() const = 0;
	virtual unsigned long Size() const = 0;
	virtual unsigned long Pos() const = 0;
	virtual void SeekTo(unsigned long pos) = 0;
    virtual void Skip(unsigned long nb) = 0;
    
public:
	void WriteUInt32(uint32_t val);
	void WriteUInt16(uint16_t val);
	void WriteUInt8(uint8_t val);
	void WriteInt32(int32_t val);
	void WriteInt16(int16_t val);
	void WriteInt8(int8_t val);
    void WriteUInt24(uint32_t val);
	void WriteDouble(double val);
	void WriteFloat(float val);
    void WriteString(const std::string&);
    void WriteVLV(int x);
    
    inline void Put(char ch) {Write(&ch, 1);}
    
    int Version() const {return _version;}
    void SetVersion(int v) {_version=v;}
    
    int SubType() const {return _subtype;}
    void SetSubType(int st) {_subtype=st;}
    
    Reflow::Endianness Endianness() const {return _endianness;}
    bool IsLittleEndian() const {return _endianness == Reflow::LittleEndian;}
    bool IsBigEndian() const {return _endianness == Reflow::BigEndian;}
    void SetEndianness(Reflow::Endianness endianness) {_endianness = endianness;}
        
protected:
    int _version;
    int _subtype;
    Reflow::Endianness _endianness;
};


class REBufferOutputStream : public REOutputStream
{
public:
	REBufferOutputStream();
	
public:
	virtual void Write(const char* bytes, unsigned long size);
	virtual const char* Data() const;
	virtual unsigned long Size() const;
	virtual unsigned long Pos() const;
	virtual void SeekTo(unsigned long pos);	
    virtual void Skip(unsigned long nb);
	
private:
	unsigned long _pos;
	std::string _buffer;
};


#endif
