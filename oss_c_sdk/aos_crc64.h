#ifndef LIBAOS_CRC_H
#define LIBAOS_CRC_H

#include "aos_define.h"

AOS_CPP_START

uint64_t aos_crc64(uint64_t crc, void *buf, size_t len);
uint64_t aos_crc64_combine(uint64_t crc1, uint64_t crc2, uintmax_t len2);
uint32_t aos_crc32(uint32_t crc, const void *buf, size_t bufLen);

AOS_CPP_END

#endif
