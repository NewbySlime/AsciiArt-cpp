#ifndef ALGORITHMS_HEADER
#define ALGORITHMS_HEADER

#include "stdint.h"
#include "utility"

#define ALGORITHM_DEFLATETYPE_PNG 0


template<typename datatype> bool algorithm_GetBit(datatype data, uint8_t index);
// reversed version of algorithm_GetBit
template<typename datatype> bool algorithm_GetBitR(datatype data, uint8_t index);

uint32_t algorithm_GetBitsR(char *data);

// uses 16-bit in case if the number of bits is above 8-bit
// end at -> 0bnnnnnnnn <- start at
// and using reversed bit
// return example, 0b000nnnnn
uint16_t algorithm_GetBitsBetweenR(char *data, uint8_t startAt, uint8_t numberOfBits);
// uses malloc to allocate array so it can use realloc
// return array to the result data and the length of the array
std::pair<char*, size_t> algorithm_DEFLATE_decompress(char *data, size_t datasize, int compressionType);

#endif