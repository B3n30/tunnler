// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <stdlib.h>
#include <string>
#include "tunnler/bytestream.h"

ByteStream::ByteStream(const uint32_t lengthInBytes) {
    if(lengthInBytes <= stackAllocationSize)
        ByteStream(stackData,lengthInBytes);
    else
        data = static_cast<unsigned char*>(malloc(lengthInBytes));
    numberOfBytesUsed=0;
    readOffset=0;
    numberOfBytesAllocated=lengthInBytes;
}
    

ByteStream::ByteStream(unsigned char* inData, const uint32_t lengthInBytes) {
    numberOfBytesUsed=lengthInBytes;
    readOffset=0;
    numberOfBytesAllocated=lengthInBytes;
    data=inData;
}

ByteStream::~ByteStream() {
    free(data);
}

void ByteStream::IgnoreBytes(const uint32_t numberOfBytes) {
    readOffset += numberOfBytes;
}

template<class templateType>
    inline void ByteStream::Write(const templateType& inTemplateVar) {
        if (sizeof(inTemplateVar)==1) {
            Write(static_cast<const unsigned char*>(&inTemplateVar), 1);
        } else {
            if (DoEndianSwap()) {
                unsigned char output[sizeof(templateType)];
                ReverseBytes(static_cast<const unsigned char*>(&inTemplateVar), output, sizeof(templateType));
                Write(static_cast<const unsigned char*>(&output), sizeof(templateType));
            } else {
                Write(static_cast<const unsigned char*>(&inTemplateVar), sizeof(templateType));
            }
        }
    }

template <>
    inline void ByteStream::Write(const uint32_t& inTemplateVar) {
        ChangeSize(sizeof(uint32_t));
        numberOfBytesUsed+=sizeof(uint32_t);
        if (DoEndianSwap()) {
            data[numberOfBytesUsed + 0] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[3];
			data[numberOfBytesUsed + 1] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[2];
			data[numberOfBytesUsed + 2] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[1];
			data[numberOfBytesUsed + 3] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[0];
        } else {
            data[numberOfBytesUsed + 0] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[0];
			data[numberOfBytesUsed + 1] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[1];
			data[numberOfBytesUsed + 2] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[2];
			data[numberOfBytesUsed + 3] = reinterpret_cast<const unsigned char*>(&inTemplateVar)[3];
        }
    }

template<>
    inline void ByteStream::Write(const std::string& inTemplateVar) {
        Write(static_cast<uint32_t>(inTemplateVar.length()));
        Write(reinterpret_cast<const unsigned char*>(inTemplateVar.c_str()), inTemplateVar.length());
    }

void ByteStream::Write(const unsigned char* inputByteArray, const uint32_t numberOfBytes) {
    ChangeSize(numberOfBytes);
    memcpy(data+numberOfBytesUsed, inputByteArray, numberOfBytes);
    numberOfBytesUsed+=numberOfBytes;
}

template<class templateType>
    inline void ByteStream::Read(templateType& outTemplateVar) {
        if (sizeof(outTemplateVar)==1) {
            Read(static_cast<unsigned char*>(&outTemplateVar), 1);
        } else {
            if (DoEndianSwap()) {
                unsigned char output[sizeof(templateType)];
                Read(static_cast<unsigned char*>(&output), sizeof(templateType));
                ReverseBytes(output, static_cast<const unsigned char*>(&outTemplateVar), sizeof(templateType));
            } else {
                Read(static_cast<unsigned char*>(&outTemplateVar), sizeof(templateType));
            }
        }
    }

template <>
    inline void ByteStream::Read(uint32_t& outTemplateVar) {
        readOffset += sizeof(uint32_t);
        if (DoEndianSwap()) {
            outTemplateVar = (data[numberOfBytesUsed + 3] << (3*8)) +
                             (data[numberOfBytesUsed + 2] << (2*8)) +
                             (data[numberOfBytesUsed + 1] << (1*8)) +
                              data[numberOfBytesUsed + 0];
        } else {
            outTemplateVar = (data[numberOfBytesUsed + 0] << (3*8)) +
                             (data[numberOfBytesUsed + 1] << (2*8)) +
                             (data[numberOfBytesUsed + 2] << (1*8)) +
                              data[numberOfBytesUsed + 3];
        }
    }

template<>
    inline void ByteStream::Read(std::string& outTemplateVar) {
        uint32_t length;
        Read(length);
        char* str = static_cast<char*>(malloc(length));
        Read(reinterpret_cast<unsigned char*>(str), length);
        outTemplateVar.assign(str,length);
    }

void ByteStream::Read(unsigned char* outputByteArray, const uint32_t numberOfBytes) {
    memcpy(outputByteArray, data + readOffset, numberOfBytes );
    readOffset += numberOfBytes;
}

void ReverseBytes(const unsigned char *inByteArray, unsigned char *inOutByteArray, const uint32_t length){
    for (uint32_t i=0; i < length; i++)
        inOutByteArray[i]=inByteArray[length-i-1];
}

void ByteStream::ChangeSize(const uint32_t numberOfBytesToWrite){
    if (bytesAllocated >= numberOfBytesToWrite+numberOfBytesUsed)
        return;
    data = static_cast<unsigned char*>(realloc(data, numberOfBytesUsed+numberOfBytesToWrite));
    bytesAllocated = numberOfBytesUsed+numberOfBytesToWrite;
}

bool DoEndianSwap() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};
    return bint.c[0] != 1;
}
