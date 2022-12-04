#include "Keeko/KeekoMessage.h"

#include <cstring>
#include <type_traits>
namespace Keeko
{

KeekoMessage::KeekoMessage(const std::unique_ptr<uint8_t[]>& raw, size_t size)
{
    if(size < KEEKO_MESSAGE_MINIMUM_SIZE)
    {
        throw KeekoMessageInvalidLength(size);
    }
    auto checksum = CRC::calculateCRC16(raw.get(), size - 2);
    uint16_t provided_checksum;
    littleEndianCopy(&provided_checksum, raw.get() + (size - 2), sizeof(uint16_t));
    if(provided_checksum != checksum)
    {
        throw KeekoMessageChecksumMismatch(checksum, provided_checksum);
    }
    for(size_t pos = 0; pos < size - 2;)
    {
        uint32_t element_crc;
        if(pos + sizeof(element_crc) >= size)
        {
            throw KeekoMessageMalformed{};
        }
        littleEndianCopy(&element_crc, raw.get() + pos, 4);
        pos += sizeof(element_crc);

        uint8_t type;
        if(pos + sizeof(type) >= size)
        {
            throw KeekoMessageMalformed{};
        }
        type = raw[pos];
        if(type != variant_index<KeekoMessageValue, std::string>())
        {
            if(pos + sizeof(KeekoMessageStringSize) >= size)
            {
                throw KeekoMessageMalformed{};
            }
        }
        else if(type >= std::variant_size_v<KeekoMessageValue>)
        {
            throw KeekoMessageMalformed{};
        }

        pos += sizeof(uint8_t);

        switch(type)
        {
            case variant_index<KeekoMessageValue, uint8_t>():
            {
                readElementFromRaw<uint8_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, uint16_t>():
            {
                readElementFromRaw<uint16_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, uint32_t>():
            {
                readElementFromRaw<uint32_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, uint64_t>():
            {
                readElementFromRaw<uint64_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, int8_t>():
            {
                readElementFromRaw<int8_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, int16_t>():
            {
                readElementFromRaw<int16_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, int32_t>():
            {
                readElementFromRaw<int32_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, int64_t>():
            {
                readElementFromRaw<int64_t>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, float>():
            {
                readElementFromRaw<float>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, double>():
            {
                readElementFromRaw<double>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, bool>():
            {
                readElementFromRaw<bool>(raw,pos,element_crc);
            }break;
            case variant_index<KeekoMessageValue, std::string>():
            {
                KeekoMessageStringSize str_size;
                littleEndianCopy(&str_size, raw.get() + pos, sizeof(KeekoMessageStringSize));
                pos += sizeof(KeekoMessageStringSize);
                if(pos + str_size >= size)
                {
                    throw KeekoMessageMalformed{};
                }
                elements[element_crc] = std::string{reinterpret_cast<char*>(reinterpret_cast<void*>(raw.get() + pos)), str_size};
                pos += str_size;
            }break;
        }
    }
}

void KeekoMessage::addElement(uint32_t crc, KeekoMessageValue element)
{
    elements[crc] = std::move(element);
}

KeekoMessage::KeekoMessageValue KeekoMessage::getElement(uint32_t crc)
{
    return elements[crc];
}

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;
std::tuple<std::unique_ptr<uint8_t[]>, size_t> KeekoMessage::getRaw()
{
    KeekoMessageValue foo;
    size_t data_size{0};
    for(const auto& [key, value]: elements)
    {
        data_size += sizeof(key) + sizeof(uint8_t);
        if(value.index() == variant_index<KeekoMessageValue,std::string>())
        {
            data_size += std::get<std::string>(value).size() * sizeof(char) + sizeof(KeekoMessageStringSize);;
        }
        else
        {
            data_size += KEEKO_MESSAGE_VALUE_SIZEOF[value.index()];
        }
    }
    data_size += sizeof(uint16_t);//TODO: Use decltype

    size_t pos{0};

    std::unique_ptr<uint8_t[]> result{new uint8_t[data_size]};
    
    for(const auto& [key, value]: elements)
    {
        littleEndianCopy(result.get() + pos, &key, sizeof(key));
        pos += sizeof(key);
        std::visit(overload
        {
            [&result, &pos, this](const auto& v)
            {
                
            },
            [&result, &pos, this](const uint8_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const uint16_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const uint32_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const uint64_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const int8_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const int16_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const int32_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const int64_t& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const float& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const double& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const bool& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                littleEndianCopy(result.get() + pos, &v, sizeof(v));
            },
            [&result, &pos, this](const std::string& v)
            {
                result[pos] = variant_index<KeekoMessageValue, std::decay<decltype(v)>::type>();
                pos += sizeof(uint8_t);
                KeekoMessageStringSize size = v.size();
                std::memcpy(result.get() + pos, &size, sizeof(KeekoMessageStringSize));
                pos += sizeof(KeekoMessageStringSize);
                auto data_size = sizeof(char) * size;
                std::memcpy(result.get() + pos, v.c_str(), data_size);
            }
        }, value);
        if(value.index() == variant_index<KeekoMessageValue,std::string>())
        {
            pos += std::get<std::string>(value).size();
        }
        else
        {
            pos += KEEKO_MESSAGE_VALUE_SIZEOF[value.index()];
        }
    }
    auto checksum = CRC::calculateCRC16(result.get(), pos);
    littleEndianCopy(result.get() + pos, &checksum, sizeof(checksum));
    return std::make_tuple(std::move(result),data_size);
}

//TODO: Can we check at compile time?
void KeekoMessage::littleEndianCopy(void* dest,const void* src, size_t size)
{
    if(isLittleEndian())
    {
        for(int i = 0; i < size; i++)
        {
            reinterpret_cast<uint8_t*>(dest)[i] = reinterpret_cast<const uint8_t*>(src)[i];
        }
    }
    else
    {
        for(int i = 0; i < size; i++)
        {
            reinterpret_cast<uint8_t*>(dest)[i] = reinterpret_cast<const uint8_t*>(src)[size - 1 - i];
        }
    }
}

}

