// Copyright 2017 Tunnler Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <stdint.h>

// This is for handling various data in ENet packets
class ByteStream final {
public:
    static const uint32_t stackAllocationSize = 32;

    ByteStream(const uint32_t lengthInBytes = stackAllocationSize);
    
    /**
     ** Creates a BitStream
     ** @param data An array of bytes.
     ** @ param lengthInBytes Size of the \a data.
     **/
    ByteStream(unsigned char* inData, const uint32_t lengthInBytes);

    ~ByteStream();

    /**
     ** Ignore data we don't intend to read
     ** @param[in] numberOfBytes The number of bytes to ignore
     **/
    void IgnoreBytes(const uint32_t numberOfBytes);

    /// TODO (B3N30): Add comments
    template <class templateType>
			void Read(templateType& outTemplateVar);
            
    void Read(unsigned char* outputByteArray, const unsigned int numberOfBytes);

    template <class templateType>
			void Write(const templateType& inTemplateVar);

    void Write(const unsigned char* inputByteArray, const unsigned int numberOfBytes);

    unsigned char* GetData() const {
        return data;
    };
    uint32_t Size() {
        return numberOfBytesUsed;
    };

private:

    void WriteBytes(const unsigned char* inByteArray, const uint32_t numberOfBytesToWrite);
    void ChangeSize(const uint32_t numberOfBytesToWrite);
    static void ReverseBytes(unsigned char *inByteArray, unsigned char *inOutByteArray, const unsigned int length);
    uint32_t numberOfBytesUsed;
    uint32_t bytesAllocated;
    uint32_t numberOfBytesAllocated;
    uint32_t readOffset;
    unsigned char* data;
    unsigned char stackData[stackAllocationSize];
};

static inline bool DoEndianSwap();
