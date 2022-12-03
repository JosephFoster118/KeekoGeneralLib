#pragma once

#include <memory>
#include <map>
#include <exception>
#include <utility>
#include <variant>
#include <array>
#include <tuple>
//#include <sstream>

#include "CRC.h"

namespace Keeko
{

class KeekoMessage
{
public:

    typedef std::variant<
        uint8_t, uint16_t, uint32_t, uint64_t,
        int8_t, int16_t, int32_t, int64_t,
        float, double,
        bool,
        std::string
        > KeekoMessageValue;

    typedef uint16_t KeekoMessageStringSize;

    //                                                   element CRC32      element type      min element size  size of checksum
    static constexpr size_t KEEKO_MESSAGE_MINIMUM_SIZE = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t);

    static constexpr std::array<size_t, std::variant_size_v<KeekoMessageValue>> KEEKO_MESSAGE_VALUE_SIZEOF
    {
        {
            sizeof(std::variant_alternative_t<0,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<1,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<2,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<3,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<4,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<5,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<6,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<7,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<8,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<9,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<10,KeekoMessageValue>),
            sizeof(std::variant_alternative_t<11,KeekoMessageValue>)
        }
    };
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

    inline KeekoMessage(){};
    KeekoMessage(const KeekoMessage&) = delete;
    KeekoMessage(const std::unique_ptr<uint8_t[]>& raw, size_t size);

    inline ~KeekoMessage(){};

    void addElement(uint32_t crc, KeekoMessageValue element);
    KeekoMessageValue getElement(uint32_t crc);
    std::tuple<std::unique_ptr<uint8_t[]>, size_t> getRaw();
private:
    std::map<uint32_t, KeekoMessageValue> elements;
    template<typename VariantType, typename T, std::size_t index = 0>
    static constexpr std::size_t variant_index()
    {
        static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
        if constexpr (index == std::variant_size_v<VariantType>) {
            return index;
        } else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
            return index;
        } else {
            return variant_index<VariantType, T, index + 1>();
        }
    }
    void littleEndianCopy(void* dest,const void* src, size_t size);
    inline bool isLittleEndian()//TODO: Move to src
    {
        short int number = 0x1;
        char *numPtr = (char*)&number;
        return (numPtr[0] == 1);
    }

    template <typename T>
    void readElementFromRaw(const std::unique_ptr<uint8_t[]>& raw, size_t& pos, uint32_t element_crc)
    {   
        T value = *(reinterpret_cast<T*>(reinterpret_cast<void*>(raw.get() + pos)));
        elements[element_crc] = *(reinterpret_cast<T*>(reinterpret_cast<void*>(raw.get() + pos)));
        pos += sizeof(T);
    }
};


struct KeekoMessageChecksumMismatch : std::exception
{
    uint16_t real_checksum;
    uint16_t given_checksum;
    std::string error_string;

    KeekoMessageChecksumMismatch() = delete;
    inline KeekoMessageChecksumMismatch(uint16_t real_checksum, uint16_t givin_checksum)
    {
        this->real_checksum = real_checksum;
        this->given_checksum = given_checksum;
        //std::ostringstream builder;
        //builder << "Checksum mismatch. Real [" << real_checksum << "] Given [" << given_checksum << "]";
        error_string = "builder.str()";//TODO: Use sprintf since iomanip and sstream are bloated
    }
    inline const char* what() const throw ()
    {
        return error_string.c_str();
    }
};

struct KeekoMessageInvalidLength : std::exception
{
    std::string error_string;

    KeekoMessageInvalidLength() = delete;
    inline KeekoMessageInvalidLength(size_t length)
    {
        //std::ostringstream builder;
       // builder << "Invalid message length. Must be greater than " << KeekoMessage::KEEKO_MESSAGE_MINIMUM_SIZE
       // << " but was " << length;
        error_string = "builder.str()";//TODO: Use sprintf since iomanip and sstream are bloated
    }
    inline const char* what() const throw ()
    {
        return error_string.c_str();
    }
};

struct KeekoMessageMalformed : std::exception
{
    std::string error_string;
    inline KeekoMessageMalformed()
    {
        error_string = "Malformed message";
    }
    inline const char* what() const throw ()
    {
        return error_string.c_str();
    }
};

}