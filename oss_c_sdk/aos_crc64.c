/* aos_crc64.c -- compute CRC-64
 * Copyright (C) 2013 Mark Adler
 * Version 1.4  16 Dec 2013  Mark Adler
 */

/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler
  madler@alumni.caltech.edu
 */

/* Compute CRC-64 in the manner of xz, using the ECMA-182 polynomial,
   bit-reversed, with one's complement pre and post processing.  Provide a
   means to combine separately computed CRC-64's. */

/* Version history:
   1.0  13 Dec 2013  First version
   1.1  13 Dec 2013  Fix comments in test code
   1.2  14 Dec 2013  Determine endianess at run time
   1.3  15 Dec 2013  Add eight-byte processing for big endian as well
                     Make use of the pthread library optional
   1.4  16 Dec 2013  Make once variable volatile for limited thread protection
 */

#include "aos_crc64.h"

/* 64-bit CRC polynomial with these coefficients, but reversed:
    64, 62, 57, 55, 54, 53, 52, 47, 46, 45, 40, 39, 38, 37, 35, 33, 32,
    31, 29, 27, 24, 23, 22, 21, 19, 17, 13, 12, 10, 9, 7, 4, 1, 0 */
#define POLY UINT64_C(0xc96c5795d7870f42)

/* Tables for CRC calculation -- filled in by initialization functions that are
   called once.  These could be replaced by constant tables generated in the
   same way.  There are two tables, one for each endianess.  Since these are
   static, i.e. local, one should be compiled out of existence if the compiler
   can evaluate the endianess check in crc64() at compile time. */
static uint64_t crc64_little_table[8][256];
static uint64_t crc64_big_table[8][256];

/* Fill in the CRC-64 constants table. */
static void crc64_init(uint64_t table[][256])
{
    unsigned n, k;
    uint64_t crc;

    /* generate CRC-64's for all single byte sequences */
    for (n = 0; n < 256; n++) {
        crc = n;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? POLY ^ (crc >> 1) : crc >> 1;
        table[0][n] = crc;
    }

    /* generate CRC-64's for those followed by 1 to 7 zeros */
    for (n = 0; n < 256; n++) {
        crc = table[0][n];
        for (k = 1; k < 8; k++) {
            crc = table[0][crc & 0xff] ^ (crc >> 8);
            table[k][n] = crc;
        }
    }
}

/* This function is called once to initialize the CRC-64 table for use on a
   little-endian architecture. */
static void crc64_little_init(void)
{
    crc64_init(crc64_little_table);
}

/* Reverse the bytes in a 64-bit word. */
static APR_INLINE uint64_t rev8(uint64_t a)
{
    uint64_t m;

    m = UINT64_C(0xff00ff00ff00ff);
    a = ((a >> 8) & m) | (a & m) << 8;
    m = UINT64_C(0xffff0000ffff);
    a = ((a >> 16) & m) | (a & m) << 16;
    return a >> 32 | a << 32;
}

/* This function is called once to initialize the CRC-64 table for use on a
   big-endian architecture. */
static void crc64_big_init(void)
{
    unsigned k, n;

    crc64_init(crc64_big_table);
    for (k = 0; k < 8; k++)
        for (n = 0; n < 256; n++)
            crc64_big_table[k][n] = rev8(crc64_big_table[k][n]);
}

/* Run the init() function exactly once.  If pthread.h is not included, then
   this macro will use a simple static state variable for the purpose, which is
   not thread-safe.  The init function must be of the type void init(void). */
#ifdef PTHREAD_ONCE_INIT
#  define ONCE(init) \
    do { \
        static pthread_once_t once = PTHREAD_ONCE_INIT; \
        pthread_once(&once, init); \
    } while (0)
#else
#  define ONCE(init) \
    do { \
        static volatile int once = 1; \
        if (once) { \
            if (once++ == 1) { \
                init(); \
                once = 0; \
            } \
            else \
                while (once) \
                    ; \
        } \
    } while (0)
#endif

/* Calculate a CRC-64 eight bytes at a time on a little-endian architecture. */
static APR_INLINE uint64_t crc64_little(uint64_t crc, void *buf, size_t len)
{
    unsigned char *next = buf;

    ONCE(crc64_little_init);
    crc = ~crc;
    while (len && ((uintptr_t)next & 7) != 0) {
        crc = crc64_little_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }
    while (len >= 8) {
        crc ^= *(uint64_t *)next;
        crc = crc64_little_table[7][crc & 0xff] ^
              crc64_little_table[6][(crc >> 8) & 0xff] ^
              crc64_little_table[5][(crc >> 16) & 0xff] ^
              crc64_little_table[4][(crc >> 24) & 0xff] ^
              crc64_little_table[3][(crc >> 32) & 0xff] ^
              crc64_little_table[2][(crc >> 40) & 0xff] ^
              crc64_little_table[1][(crc >> 48) & 0xff] ^
              crc64_little_table[0][crc >> 56];
        next += 8;
        len -= 8;
    }
    while (len) {
        crc = crc64_little_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }
    return ~crc;
}

/* Calculate a CRC-64 eight bytes at a time on a big-endian architecture. */
static APR_INLINE uint64_t crc64_big(uint64_t crc, void *buf, size_t len)
{
    unsigned char *next = buf;

    ONCE(crc64_big_init);
    crc = ~rev8(crc);
    while (len && ((uintptr_t)next & 7) != 0) {
        crc = crc64_big_table[0][(crc >> 56) ^ *next++] ^ (crc << 8);
        len--;
    }
    while (len >= 8) {
        crc ^= *(uint64_t *)next;
        crc = crc64_big_table[0][crc & 0xff] ^
              crc64_big_table[1][(crc >> 8) & 0xff] ^
              crc64_big_table[2][(crc >> 16) & 0xff] ^
              crc64_big_table[3][(crc >> 24) & 0xff] ^
              crc64_big_table[4][(crc >> 32) & 0xff] ^
              crc64_big_table[5][(crc >> 40) & 0xff] ^
              crc64_big_table[6][(crc >> 48) & 0xff] ^
              crc64_big_table[7][crc >> 56];
        next += 8;
        len -= 8;
    }
    while (len) {
        crc = crc64_big_table[0][(crc >> 56) ^ *next++] ^ (crc << 8);
        len--;
    }
    return ~rev8(crc);
}

/* Return the CRC-64 of buf[0..len-1] with initial crc, processing eight bytes
   at a time.  This selects one of two routines depending on the endianess of
   the architecture.  A good optimizing compiler will determine the endianess
   at compile time if it can, and get rid of the unused code and table.  If the
   endianess can be changed at run time, then this code will handle that as
   well, initializing and using two tables, if called upon to do so. */
uint64_t aos_crc64(uint64_t crc, void *buf, size_t len)
{
    uint64_t n = 1;

    return *(char *)&n ? crc64_little(crc, buf, len) :
                         crc64_big(crc, buf, len);
}

#define GF2_DIM 64      /* dimension of GF(2) vectors (length of CRC) */

static uint64_t gf2_matrix_times(uint64_t *mat, uint64_t vec)
{
    uint64_t sum;

    sum = 0;
    while (vec) {
        if (vec & 1)
            sum ^= *mat;
        vec >>= 1;
        mat++;
    }
    return sum;
}

static void gf2_matrix_square(uint64_t *square, uint64_t *mat)
{
    unsigned n;

    for (n = 0; n < GF2_DIM; n++)
        square[n] = gf2_matrix_times(mat, mat[n]);
}

/* Return the CRC-64 of two sequential blocks, where crc1 is the CRC-64 of the
   first block, crc2 is the CRC-64 of the second block, and len2 is the length
   of the second block. */
uint64_t aos_crc64_combine(uint64_t crc1, uint64_t crc2, uintmax_t len2)
{
    unsigned n;
    uint64_t row;
    uint64_t even[GF2_DIM];     /* even-power-of-two zeros operator */
    uint64_t odd[GF2_DIM];      /* odd-power-of-two zeros operator */

    /* degenerate case */
    if (len2 == 0)
        return crc1;

    /* put operator for one zero bit in odd */
    odd[0] = POLY;              /* CRC-64 polynomial */
    row = 1;
    for (n = 1; n < GF2_DIM; n++) {
        odd[n] = row;
        row <<= 1;
    }

    /* put operator for two zero bits in even */
    gf2_matrix_square(even, odd);

    /* put operator for four zero bits in odd */
    gf2_matrix_square(odd, even);

    /* apply len2 zeros to crc1 (first square will put the operator for one
       zero byte, eight zero bits, in even) */
    do {
        /* apply zeros operator for this bit of len2 */
        gf2_matrix_square(even, odd);
        if (len2 & 1)
            crc1 = gf2_matrix_times(even, crc1);
        len2 >>= 1;

        /* if no more bits set, then done */
        if (len2 == 0)
            break;

        /* another iteration of the loop with odd and even swapped */
        gf2_matrix_square(odd, even);
        if (len2 & 1)
            crc1 = gf2_matrix_times(odd, crc1);
        len2 >>= 1;

        /* if no more bits set, then done */
    } while (len2 != 0);

    /* return combined crc */
    crc1 ^= crc2;
    return crc1;
}

/*CRC32*/
static const uint32_t crc32Table[256] = {
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
    0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
    0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
    0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
    0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
    0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
    0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
    0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
    0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
    0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
    0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
    0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
    0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
    0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
    0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
    0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
    0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
    0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
    0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
    0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
    0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
    0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
    0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
    0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
    0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
    0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
    0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
    0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
    0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
    0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
    0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
    0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
    0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
    0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
    0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

uint32_t aos_crc32(uint32_t crc, const void *buf, size_t bufLen) 
{
    uint32_t crc32;
    unsigned char *byteBuf;
    size_t i;

    /** accumulate crc32 for buffer **/
    crc32 = crc ^ 0xFFFFFFFF;
    byteBuf = (unsigned char*)buf;
    for (i = 0; i < bufLen; i++) {
        crc32 = (crc32 >> 8) ^ crc32Table[(crc32 ^ byteBuf[i]) & 0xFF];
    }
    return crc32 ^ 0xFFFFFFFF;
}
