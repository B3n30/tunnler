#pragma once

#include <vector>
#include "tunnler/tunnler.h"

class Packet
{
    // A bool-like type that cannot be converted to integer or pointer types
    typedef bool (Packet::*BoolType)(std::size_t);

public :

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates an empty packet.
    ///
    ////////////////////////////////////////////////////////////
    Packet();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Packet();

    ////////////////////////////////////////////////////////////
    /// \brief Append data to the end of the packet
    ///
    /// \param data        Pointer to the sequence of bytes to append
    /// \param sizeInBytes Number of bytes to append
    ///
    /// \see clear
    ///
    ////////////////////////////////////////////////////////////
    void append(const void* data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the packet
    ///
    /// After calling Clear, the packet is empty.
    ///
    /// \see append
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    void IgnoreBytes(uint32_t length);

    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the data contained in the packet
    ///
    /// Warning: the returned pointer may become invalid after
    /// you append data to the packet, therefore it should never
    /// be stored.
    /// The return pointer is NULL if the packet is empty.
    ///
    /// \return Pointer to the data
    ///
    /// \see getDataSize
    ///
    ////////////////////////////////////////////////////////////
    const void* getData() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the data contained in the packet
    ///
    /// This function returns the number of bytes pointed to by
    /// what getData returns.
    ///
    /// \return Data size, in bytes
    ///
    /// \see getData
    ///
    ////////////////////////////////////////////////////////////
    std::size_t getDataSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the reading position has reached the
    ///        end of the packet
    ///
    /// This function is useful to know if there is some data
    /// left to be read, without actually reading it.
    ///
    /// \return True if all data was read, false otherwise
    ///
    /// \see operator bool
    ///
    ////////////////////////////////////////////////////////////
    bool endOfPacket() const;

public:

    ////////////////////////////////////////////////////////////
    /// \brief Test the validity of the packet, for reading
    ///
    /// This operator allows to test the packet as a boolean
    /// variable, to check if a reading operation was successful.
    ///
    /// A packet will be in an invalid state if it has no more
    /// data to read.
    ///
    /// This behaviour is the same as standard C++ streams.
    ///
    /// Usage example:
    /// \code
    /// float x;
    /// packet >> x;
    /// if (packet)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    ///
    /// // -- or --
    ///
    /// float x;
    /// if (packet >> x)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    /// \endcode
    ///
    /// Don't focus on the return type, it's equivalent to bool but
    /// it disallows unwanted implicit conversions to integer or
    /// pointer types.
    ///
    /// \return True if last data extraction from packet was successful
    ///
    /// \see endOfPacket
    ///
    ////////////////////////////////////////////////////////////
    operator BoolType() const;

    ////////////////////////////////////////////////////////////
    /// Overloads of operator >> to read data from the packet
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator >>(bool&         data);
    Packet& operator >>(int8_t&         data);
    Packet& operator >>(uint8_t&        data);
    Packet& operator >>(int16_t&        data);
    Packet& operator >>(uint16_t&       data);
    Packet& operator >>(int32_t&        data);
    Packet& operator >>(uint32_t&       data);
    Packet& operator >>(float&        data);
    Packet& operator >>(double&       data);
    Packet& operator >>(char*         data);
    Packet& operator >>(std::string&  data);
    Packet& operator >>(wchar_t*      data);
    Packet& operator >>(std::wstring& data);
    template<typename T>
        Packet& operator >>(std::vector<T>& data);
    template<typename T, std::size_t S>
        Packet& operator >>(std::array<T,S>& data);

    ////////////////////////////////////////////////////////////
    /// Overloads of operator << to write data into the packet
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator <<(bool                data);
    Packet& operator <<(int8_t                data);
    Packet& operator <<(uint8_t               data);
    Packet& operator <<(int16_t               data);
    Packet& operator <<(uint16_t              data);
    Packet& operator <<(int32_t               data);
    Packet& operator <<(uint32_t              data);
    Packet& operator <<(float               data);
    Packet& operator <<(double              data);
    Packet& operator <<(const char*         data);
    Packet& operator <<(const std::string&  data);
    Packet& operator <<(const wchar_t*      data);
    Packet& operator <<(const std::wstring& data);
    template<typename T>
        Packet& operator <<(const std::vector<T>& data);
    template<typename T, std::size_t S>
        Packet& operator <<(const std::array<T,S>& data);

private :

    ////////////////////////////////////////////////////////////
    /// Disallow comparisons between packets
    ///
    ////////////////////////////////////////////////////////////
    bool operator ==(const Packet& right) const;
    bool operator !=(const Packet& right) const;

    ////////////////////////////////////////////////////////////
    /// \brief Check if the packet can extract a given number of bytes
    ///
    /// This function updates accordingly the state of the packet.
    ///
    /// \param size Size to check
    ///
    /// \return True if \a size bytes can be read from the packet
    ///
    ////////////////////////////////////////////////////////////
    bool checkSize(std::size_t size);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<char> m_data;    ///< Data stored in the packet
    std::size_t       m_readPos; ///< Current reading position in the packet
    bool              m_isValid; ///< Reading state of the packet
};

template<typename T>
Packet& Packet::operator >>(std::vector<T>& data)
{
    // Then extract characters
    for (uint32_t i = 0; i < data.size(); ++i)
    {
        T character = 0;
        *this >> character;
        data[i] = character;
    }
    return *this;
}

template<typename T, std::size_t S>
Packet& Packet::operator >>(std::array<T, S>& data)
{
    // Then extract characters
    for (uint32_t i = 0; i < data.size(); ++i)
    {
        T character = 0;
        *this >> character;
        data[i] = character;
    }
    return *this;
}

template<typename T>
Packet& Packet::operator <<(const std::vector<T>& data)
{
    // Then extract characters
    for (uint32_t i = 0; i < data.size(); ++i)
    {
        *this << data[i];
    }
    return *this;
}

template<typename T, std::size_t S>
Packet& Packet::operator <<(const std::array<T, S>& data)
{
    // Then extract characters
    for (uint32_t i = 0; i < data.size(); ++i)
    {
        *this << data[i];
    }
    return *this;
}
