#include "algorithms.h"
#include "stdint.h"

bool algorithm_GetBit(char data, uint8_t index){
  return (data & (1 << index)) == 0;
}

bool algorithm_GetBitR(char data, uint8_t index){
  return (data & (1 << (8-index))) == 0;
}

uint16_t algorithm_GetBitsR(char *data){
  uint16_t byte2 = *reinterpret_cast<uint16_t*>(data);
  uint16_t res = 0;
  for(int i = 0; i < 16; i++)
    res |= ((byte2 >> i) & true) << (16-i);
  
  return res;
}

uint16_t algorithm_GetBitsBetween(char *data, uint8_t startAt, uint8_t numberofbits){
  uint16_t byte2 = *reinterpret_cast<uint16_t*>(data);
  return (byte2 >> startAt) & (0xffff >> (16-numberofbits));
}

uint16_t algorithm_GetBitsBetweenR(char *data, uint8_t startAt, uint8_t numberofbits){
  uint16_t byte2 = algorithm_GetBitsR(data);
  return (byte2 >> startAt) & (0xffff >> (16-numberofbits));
}

// PNG
const int __slidingWindowSizeArr[] = {32000};
const int __lookaheadWindowSizeArr[] = {258};

const int __compressionTypeCount = sizeof(__slidingWindowSizeArr)/sizeof(int);

char *algorithm_DEFLATE(char *data, size_t datasize, int compressionType){
  const int SlidingWindowSize = __slidingWindowSizeArr[compressionType], LookaheadWindowSize = __lookaheadWindowSizeArr[compressionType];

  bool keepLooping = true;
  while(keepLooping){
    keepLooping = !algorithm_GetBit(data[0], 0);

    uint8_t HuffmanCodingType = (data[0] >> 1) & 0b11; 

    switch(HuffmanCodingType){
      // no compression
      break; case 0b0:{

      }

      // fixed Huffman coding
      break; case 0b1:{

      }

      // dynamic Huffman coding
      break; case 0b10:{

      }
    }
  }
}