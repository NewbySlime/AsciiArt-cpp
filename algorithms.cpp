#include "algorithms.h"

#include "stdint.h"
#include "utility"
#include "vector"
#include "math.h"

using namespace std;

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

uint16_t algorithm_GetBitsBetweenR(char *data, uint8_t endAt, uint8_t numberofbits){
  uint16_t byte2 = algorithm_GetBitsR(data);
  return (byte2 << endAt) & (0xffff >> (16-numberofbits));
}

// PNG
const int __slidingWindowSizeArr[] = {32000};
const int __lookaheadWindowSizeArr[] = {258};

const int __compressionTypeCount = sizeof(__slidingWindowSizeArr)/sizeof(int);


class HuffmanTree{
  public:
    HuffmanTree(){}

    uint16_t GetLeaf(uint16_t bits, uint8_t bitLength){

    }
};


const int16_t _StartLengthCode = 257;
const uint8_t _StartDistanceCode = 0;
const int16_t _MaxLengthCode = 285;
const uint8_t _MaxDistanceCode = 29;
const uint16_t _StartLength = 1;
const uint16_t _StartDistance = 3;

const int16_t _Lengthcode[] = {
  257,
  265,
  269,
  273,
  277,
  281,
  -285
};

const uint8_t _Distancecode[] = {
  0,
  4,
  6,
  8,
  10,
  12,
  14,
  16,
  18,
  20,
  22,
  24,
  26,
  28
};


// use reversed binary data
class StaticHuffmanTree{
  private:
    // extra bits and start length
    static vector<pair<uint8_t, uint16_t>> distanceCodes;
    static vector<pair<uint8_t, int16_t>> lengthCodes;

    static bool isReady;

  public:
    StaticHuffmanTree(){}

    // if the code is length codes or distance codes,
    // it will return the extra bits.
    // for getting length, use GetExtra
    // -- first pair --
    // return value -1, means end of block
    // return value < -2, means the extrabits -(val+2)
    // -- second pair --
    // return how many bits are used
    static pair<int16_t, uint8_t> GetLeaf(uint16_t bits){
      uint16_t head = bits >> (16-5);
      uint16_t bitcount = 0;
      uint16_t code = 0;
      
      // literal 144-255
      if(head >= 0b11001){
        bitcount = 9;
        code = bits >> (16-bitcount);
        return pair<int16_t, uint8_t>{(code - 0b110010000) + 144, bitcount};
      }

      //length 280-285
      else if(head >= 0b11000){
        bitcount = 8;
        code = bits >> (16-bitcount);
        return pair<int16_t, uint8_t>{-(lengthCodes[code - 0b11000000].first+2), bitcount};
      }

      // literal 0-143
      else if(head >= 0b00110){
        bitcount = 8;
        code = bits >> (16-bitcount);
        return pair<int16_t, uint8_t>((code - 0b00110000) + 0, bitcount);
      }

      // length 257-279 plus end of block
      else if(head >= 0b00000){
        bitcount = 7;
        code = bits >> (16-bitcount);

        if(code == 0)
          return pair<int16_t, uint8_t>{-1, 0};
        
        // magic number 23 is subtraction of 280 and 257
        return pair<int16_t, uint8_t>{-(lengthCodes[code - 0b0000001 + 23].first+2), bitcount};
      }

      return pair<int16_t, uint8_t>{0,0};
    }

    static int SetupClass(){
      if(isReady)
        return 0;

      int i = _StartLengthCode, index = 0, num = _StartLength;
      while(i <= _MaxLengthCode){
        int _n = (int)pow(2, index);
        for(int n = 0; n < _n; n++){
          lengthCodes.push_back(pair<uint8_t, uint16_t>{index, num});
          num += _n;
          i++;
        }

        index++;
      }

      int i = _StartDistanceCode, index = 0, num = _StartDistance;
      while(i <= _MaxDistanceCode){
        int _n = _Distancecode[index] >= 0 ? (int)pow(2, index): 1;
        for(int n = 0; n < _n; n++){
          distanceCodes.push_back(pair<uint8_t, int16_t>{index, num});
          num += _n;
          i++;
        }

        index++;
      }

      isReady = true;

      return 0;
    }

    // return extra bits, for the distance, use GetExtra
    static uint16_t GetDistance(uint8_t code){
      if(code > 29)
        return 0;

      return distanceCodes[code].first;
    }

    // can be used to get full length or distance
    // extraBits is the extra bits provided in the code
    // for extraBits, don't forget to revert back to the original binary form (not reversed)
    static uint16_t GetExtra(uint16_t code, uint8_t extraBits){
      // distance
      if(code >= 0 && code <= 29)
        return distanceCodes[code].second + (extraBits & (0xff >> 8-distanceCodes[code].first));

      // length
      else if(code >= 257 && code <= 285)
        return lengthCodes[code].second + (extraBits & (0xff >> 8-lengthCodes[code].first));

      return 0;
    }
};


// setting up StaticHuffmanTree class
int __res = StaticHuffmanTree::SetupClass();

char *algorithm_DEFLATE_decompress(char *data, size_t datasize, int compressionType){
  const int SlidingWindowSize = __slidingWindowSizeArr[compressionType];

  bool keepLooping = true;
  while(keepLooping){
    keepLooping = !algorithm_GetBit(data[0], 0);

    uint8_t HuffmanCodingType = (data[0] >> 1) & 0b11;
    size_t offsetbits = 3;

    switch(HuffmanCodingType){
      // no compression
      break; case 0b0:{

      }

      // fixed Huffman coding
      break; case 0b1:{
        auto res = StaticHuffmanTree::GetLeaf(algorithm_GetBitsR(data+(offsetbits/8)));
        
        // value is literal
        if(res.first >= 0){

        }

        // end of block
        else if(res.first == -1)
          continue;
        
        // value is the count of extra bits
        else{
          uint16_t length, distance;

          
        }
      }

      // dynamic Huffman coding
      break; case 0b10:{

      }
    }
  }
}