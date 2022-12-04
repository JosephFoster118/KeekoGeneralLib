#include "Keeko/CRC.h"

// constexpr uint16_t CRC::calculateCRC16(const void* buffer, size_t size)
// {
//     uint16_t crc = CRC16_PRESET;
//     auto data = reinterpret_cast<const uint8_t*>(buffer);
//     for(int i = 0; i < size; i++)
//     {
//         uint8_t cc = data[i];
//         uint8_t tmp = (crc >> 8) ^ cc;
//         crc = (crc << 8) ^ CRC16_TABLE[tmp];
//     }
//     return crc;
// }
// constexpr uint32_t CRC::calculateCRC32(const void* buffer, size_t size)
// {
//     uint32_t crc = CRC32_PRESET;
//     auto data = reinterpret_cast<const uint8_t*>(buffer);
//     for(int i = 0; i < size; i++)
//     {
//         uint8_t cc = data[i];
//         uint8_t tmp = (crc >> 8) ^ cc;
//         crc = (crc << 8) ^ CRC32_TABLE[tmp];
//     }ssh joseph
//     return crc;
// }
