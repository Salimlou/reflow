//
//  REInputStream.cpp
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>

#include "REInputStream.h"
#include "RETypes.h"
#include "REException.h"


uint32_t REInputStream::ReadUInt32()
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness))
    {
        uint32_t val;
        Read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else
    {
        uint8_t b0 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        uint8_t b2 = ReadUInt8();
        uint8_t b3 = ReadUInt8();
        return static_cast<uint32_t>(((b0<<24) & 0xFF000000) | ((b1<<16) & 0x00FF0000) | ((b2<<8) & 0x0000FF00) | (b3 & 0xFF));
    }
}

uint16_t REInputStream::ReadUInt16()
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness))
    {
        uint16_t val;
        Read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else
    {
        uint8_t b0 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        return ((b0<<8) & 0xFF00) | (b1 & 0xFF);
    }
}

uint8_t REInputStream::ReadUInt8()
{
    uint8_t val;
    Read(reinterpret_cast<char*>(&val), sizeof(val));
    return val;
}

int32_t REInputStream::ReadInt32()
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness))
    {
        int32_t val;
        Read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else
    {
        uint8_t b0 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        uint8_t b2 = ReadUInt8();
        uint8_t b3 = ReadUInt8();
        return static_cast<int32_t>(((b0<<24) & 0xFF000000) | ((b1<<16) & 0x00FF0000) | ((b2<<8) & 0x0000FF00) | (b3 & 0xFF));
    }
}

int REInputStream::ReadInt24()
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness))
    {
        uint8_t b2 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        uint8_t b0 = ReadUInt8();
        return ((b0<<16) & 0x00FF0000) | ((b1<<8) & 0x0000FF00) | (b2 & 0xFF);
    }
    else
    {
        uint8_t b0 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        uint8_t b2 = ReadUInt8();
        return ((b0<<16) & 0x00FF0000) | ((b1<<8) & 0x0000FF00) | (b2 & 0xFF);
    }
}

int16_t REInputStream::ReadInt16()
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness))
    {
        int16_t val;
        Read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
    else
    {
        uint8_t b0 = ReadUInt8();
        uint8_t b1 = ReadUInt8();
        return ((b0<<8) & 0xFF00) | (b1 & 0xFF);
    }
}

int8_t REInputStream::ReadInt8()
{
    int8_t val;
    Read(reinterpret_cast<char*>(&val), sizeof(val));
    return val;
}

double REInputStream::ReadDouble()
{
    double val;
    Read(reinterpret_cast<char*>(&val), sizeof(val));
    return val;
}

float REInputStream::ReadFloat()
{
    float val;
    Read(reinterpret_cast<char*>(&val), sizeof(val));
    return val;
}

std::string REInputStream::ReadString()
{
    uint32_t size = ReadUInt32();
    if(size == 0) return "";
    
    char* str = (char*)malloc(size);
    Read(str, size);
    std::string result(str, size);
    free(str);
    return result;
}

std::string REInputStream::ReadBytes(unsigned long size)
{
    char* data = (char*) ::malloc(size);
    Read(data, size);
    std::string buffer(data, size);
    ::free(data);
    return buffer;
}

int REInputStream::ReadVLV()
{
    unsigned int value = 0;
    uint8_t c = ReadUInt8();
    value = c;
    int i=0;
    if(value & 0x80)
    {
        value &= 0x7F;
        do {
            c = ReadUInt8();
            value = (value << 7) + (c & 0x7F);
            ++i;
        }
        while(c & 0x80 && i < 3);
    }
    return value;
}


REBufferInputStream::REBufferInputStream(const char* bytes, unsigned long size)
    : _pos(0), _buffer(bytes, size)
{
    SetVersion(REFLOW_IO_VERSION);
    SetSubType(REFLOW_IO_REFLOW2);
    SetEndianness(Reflow::LittleEndian);    
}

REBufferInputStream::REBufferInputStream(const std::string& bytes)
: _pos(0), _buffer(bytes)
{
    SetVersion(REFLOW_IO_VERSION);    
    SetSubType(REFLOW_IO_REFLOW2);
    SetEndianness(Reflow::LittleEndian);    
}

void REBufferInputStream::Read(char* bytes, unsigned long size)
{
    if(_pos + size > Size()) {
        REThrow("IO Error");
    }
    memcpy(bytes, static_cast<const char*>(_buffer.data()) + _pos, size);
    _pos += size;
}
const char* REBufferInputStream::Data() const
{
    return _buffer.data();
}
unsigned long REBufferInputStream::Size() const 
{
    return _buffer.size();
}
unsigned long REBufferInputStream::Pos() const
{
    return _pos;
}
void REBufferInputStream::SeekTo(unsigned long pos)
{
    if(pos <= Size()) {
        _pos = pos;
    }
    else REThrow("IO Error");
}

bool REBufferInputStream::AtEnd() const {
    return _pos < Size();
}

void REInputStream::Skip(unsigned long count)
{
    SeekTo(Pos() + count);
}






REConstBufferInputStream::REConstBufferInputStream(const char* bytes, unsigned long size)
: _pos(0), _data(bytes), _size(size)
{
    SetVersion(REFLOW_IO_VERSION);
    SetSubType(REFLOW_IO_REFLOW2);
    SetEndianness(Reflow::LittleEndian);
}

void REConstBufferInputStream::Read(char* bytes, unsigned long size)
{
    if(_pos + size > Size()) {
        REThrow("IO Error");
    }
    memcpy(bytes, static_cast<const char*>(_data) + _pos, size);
    _pos += size;
}
const char* REConstBufferInputStream::Data() const
{
    return _data;
}
unsigned long REConstBufferInputStream::Size() const
{
    return _size;
}
unsigned long REConstBufferInputStream::Pos() const
{
    return _pos;
}
void REConstBufferInputStream::SeekTo(unsigned long pos)
{
    if(pos <= Size()) {
        _pos = pos;
    }
    else REThrow("IO Error");
}

bool REConstBufferInputStream::AtEnd() const {
    return _pos < Size();
}






REFileInputStream::REFileInputStream()
: _file(NULL), _size(0)
{
    SetVersion(REFLOW_IO_VERSION);
    SetEndianness(Reflow::LittleEndian);
}

REFileInputStream::~REFileInputStream()
{
    if(_file) {
        Close();
    }
}

bool REFileInputStream::Open(const std::string& filename)
{
    _file = ::fopen(filename.c_str(), "rb");
    if(!_file) {
        std::cout << "Error: Failed to open " << filename << " for reading" << std::endl;
        return false;
    }
    
    ::fseek(_file, 0, SEEK_END);
    _size = ::ftell(_file);
    ::fseek(_file, 0, SEEK_SET);
    return true;
}
void REFileInputStream::Close()
{
    if(_file) {
        ::fclose(_file);
        _file = NULL;
    }
}

void REFileInputStream::Read(char* bytes, unsigned long size)
{
    if(1 != ::fread(bytes, size, 1, _file)) {
        REThrow("IO Error");
    }
}
const char* REFileInputStream::Data() const
{
    return NULL;
}
unsigned long REFileInputStream::Size() const
{
    return _size;
}
unsigned long REFileInputStream::Pos() const
{
    return ::ftell(_file);
}
void REFileInputStream::SeekTo(unsigned long pos)
{
    ::fseek(_file, pos, SEEK_SET);
}

bool REFileInputStream::AtEnd() const {
    return Pos() >= Size();
}
