#ifndef MEM_H_
#define MEM_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"
#include <stdlib.h>
#include <string.h>

//--------------------------------------- Public Constants -----------------------------------------

#define MEMORY_ALIGNMENT	4		                ///< Must be a power of 2
//#define ALLOW_UNALIGNED_ADDRESSING
//#define SYSTEM_IS_BIG_ENDIAN

#define memAlloc(bytes)             malloc(bytes)
#define memRealloc(ptr, bytes)      realloc(ptr, bytes)
#define memFree(ptr)                free(ptr)
#define memCopy(dst, src, size)	    memcpy(dst, src, size)
#define memSet(dst, val, size)	    memset(dst, val, size)
#define memCompare(b1, b2, size)	memcmp(b1, b2, size)
    
#define SWAP_16(x) ((x << 8) | ((x >> 8) & 0xff))
#define SWAP_32(x) SWAP_16(x >> 16) | SWAP_16(x & 0xffff) << 16 
#define SWAP_64(x) SWAP_32(x >> 32) | SWAP_32(x & 0xffffffff) << 32 

#ifdef SYSTEM_IS_BIG_ENDIAN
#define LITTLE_ENDIAN_16(x) SWAP_16(x)
#define LITTLE_ENDIAN_32(x) SWAP_32(x)
#define LITTLE_ENDIAN_64(x) SWAP_64(x)
#else
#define LITTLE_ENDIAN_16(x) x
#define LITTLE_ENDIAN_32(x) x
#define LITTLE_ENDIAN_64(x) x
#endif

#ifdef ALLOW_UNALIGNED_ADDRESSING
#define MEM_COPY_16BIT(dst, src) (*((uint16_t*)dst) = *((uint16_t*)src))
#define MEM_COPY_32BIT(dst, src) (*((uint32_t*)dst) = *((uint32_t*)src))
#define MEM_COPY_48BIT(dst, src) (*((uint32_t*)dst) = *((uint32_t*)src)); *((uint16_t*)dst+4) = *((uint16_t*)src+4)
#define MEM_COPY_64BIT(dst, src) (*((uint64_t*)dst) = *((uint64_t*)src))
#else
#define MEM_COPY_16BIT(dst, src) *((uint8_t*)dst+0) = *((uint8_t*)src+0); *((uint8_t*)dst+1) = *((uint8_t*)src+1)
#define MEM_COPY_32BIT(dst, src) *((uint8_t*)dst+0) = *((uint8_t*)src+0); *((uint8_t*)dst+1) = *((uint8_t*)src+1); \
                                 *((uint8_t*)dst+2) = *((uint8_t*)src+2); *((uint8_t*)dst+3) = *((uint8_t*)src+3)
#define MEM_COPY_48BIT(dst, src) *((uint8_t*)dst+0) = *((uint8_t*)src+0); *((uint8_t*)dst+1) = *((uint8_t*)src+1); \
                                 *((uint8_t*)dst+2) = *((uint8_t*)src+2); *((uint8_t*)dst+3) = *((uint8_t*)src+3); \
								 *((uint8_t*)dst+4) = *((uint8_t*)src+4); *((uint8_t*)dst+5) = *((uint8_t*)src+5)
#define MEM_COPY_64BIT(dst, src) *((uint8_t*)dst+0) = *((uint8_t*)src+0); *((uint8_t*)dst+1) = *((uint8_t*)src+1); \
                                 *((uint8_t*)dst+2) = *((uint8_t*)src+2); *((uint8_t*)dst+3) = *((uint8_t*)src+3); \
                                 *((uint8_t*)dst+4) = *((uint8_t*)src+4); *((uint8_t*)dst+5) = *((uint8_t*)src+5); \
                                 *((uint8_t*)dst+6) = *((uint8_t*)src+6); *((uint8_t*)dst+7) = *((uint8_t*)src+7)
#endif

//----------------------------------------- Public Types ------------------------------------------

//---------------------------------- Public Function Prototypes -----------------------------------

void memInitialise(void);
void memPack16Bit(uint8_t** dst, uint16_t value);
void memPack32Bit(uint8_t** dst, uint32_t value);
void memPack48Bit(uint8_t** dst, uint64_t value);
void memPack64Bit(uint8_t** dst, uint64_t value);
void memPackFloat32(uint8_t** dst, real_t value);
void memPackDouble64(uint8_t** dst, real_t value);
uint16_t memGet16Bit(const uint8_t** data);
uint32_t memGet32Bit(const uint8_t** data);
uint64_t memGet48Bit(const uint8_t** data);
uint64_t memGet64Bit(const uint8_t** data);
real_t memGetFloat32(const uint8_t** data);
real_t memGetDouble64(const uint8_t** data);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
