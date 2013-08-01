//
//  REInputStream.h
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REInputStream_H_
#define _REInputStream_H_

#include <string>
#ifdef _WIN32
#  include <cstdint>
#endif

#include "RETypes.h"

class REInputStream
{
public:
	virtual void Read(char* bytes, unsigned long size) = 0;
	virtual const char* Data() const = 0;
	virtual unsigned long Size() const = 0;
	virtual unsigned long Pos() const = 0;
	virtual void SeekTo(unsigned long pos) = 0;
    virtual bool AtEnd() const = 0;
	
public:
	uint32_t ReadUInt32();
	uint16_t ReadUInt16();
	uint8_t ReadUInt8();
	int32_t ReadInt32();
	int16_t ReadInt16();
    int ReadInt24();
	int8_t ReadInt8();
	double ReadDouble();
	float  ReadFloat();
    std::string ReadString();
    std::string ReadBytes(unsigned long size);
    int ReadVLV();
    void Skip(unsigned long count);
    
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

class REConstBufferInputStream : public REInputStream
{
public:
	REConstBufferInputStream(const char* bytes, unsigned long size);
	
public:
	virtual void Read(char* bytes, unsigned long size);
	virtual const char* Data() const;
	virtual unsigned long Size() const;
	virtual unsigned long Pos() const;
	virtual void SeekTo(unsigned long pos);
    virtual bool AtEnd() const;
	
private:
	unsigned long _pos;
    unsigned long _size;
    const char* _data;
};

class REBufferInputStream : public REInputStream
{
public:
	REBufferInputStream(const char* bytes, unsigned long size);
    REBufferInputStream(const std::string& bytes);
	
public:
	virtual void Read(char* bytes, unsigned long size);
	virtual const char* Data() const;
	virtual unsigned long Size() const;
	virtual unsigned long Pos() const;
	virtual void SeekTo(unsigned long pos);
    virtual bool AtEnd() const;
	
private:
	unsigned long _pos;
	std::string _buffer;
};

class REFileInputStream : public REInputStream
{
public:
	REFileInputStream();
    virtual ~REFileInputStream();
    
    bool Open(const std::string& filename);
    void Close();
	
public:
	virtual void Read(char* bytes, unsigned long size);
	virtual const char* Data() const;
	virtual unsigned long Size() const;
	virtual unsigned long Pos() const;
	virtual void SeekTo(unsigned long pos);
	virtual bool AtEnd() const;
    
private:
	FILE* _file;
    unsigned long _size;
};

#endif
