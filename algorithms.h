#ifndef ALGORITHMS_HEADER
#define ALGORITHMS_HEADER

#include "stdint.h"

#define ALGORITHM_DEFLATETYPE_PNG 0


bool algorithm_GetBit(char data, uint8_t index);
// reversed version of algorithm_GetBit
bool algorithm_GetBitR(char data, uint8_t index);

uint16_t algorithm_GetBitsR(char *data);

// uses 16-bit in case if the number of bits is above 8-bit
// end at -> 0bnnnnnnnn <- start at
uint16_t algorithm_GetBitsBetween(char *data, uint8_t startAt, uint8_t numberOfBits);
// reversed version of algorithm_GetByteBetweenR
uint16_t algorithm_GetBitsBetweenR(char *data, uint8_t startAt, uint8_t numberOfBits);
char *algorithm_DEFLATE(char *data, size_t datasize, int compressionType);

#endif`