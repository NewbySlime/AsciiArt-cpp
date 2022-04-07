#include "algorithms.h"

#include "stdint.h"
#include "vector"
#include "map"
#include "math.h"

#include "iostream"

using namespace std;

template<typename t_num> bool algorithm_GetBit(t_num data, uint8_t index){
  return (data >> index) & 0b1;
}

template<typename t_num> bool algorithm_GetBitR(t_num data, uint8_t index){
  return (data >> (8-index)) & 0b1;
}

template<typename num> num algorithm_GetBitsR(char *data){
  num res = 0;
  for(int i_c = 0; i_c < sizeof(num); i_c++){
    uint8_t newval = 0;
    for(int i = 0; i < 8; i++)
      newval |= ((data[i_c] >> i) & 0b1) << (7-i);
    
    res |= newval << (((sizeof(num)-1)-i_c) * 8);
  }
  
  return res;
}

uint16_t algorithm_GetBitsBetweenR(char *data, uint8_t endAt, uint8_t numberofbits){
  uint32_t byte2 = algorithm_GetBitsR<uint32_t>(data);
  return (byte2 >> (32-endAt-numberofbits)) & (0xffff >> (16-numberofbits));
}

uint16_t algorithm_ReverseBits(uint16_t val, uint8_t bitcount){
  uint16_t newval = algorithm_GetBitsR<uint16_t>(reinterpret_cast<char*>(&val));
  return newval >> (16-bitcount);
}

// PNG
const int __slidingWindowSizeArr[] = {32000};
const int __lookaheadWindowSizeArr[] = {258};

const int __compressionTypeCount = sizeof(__slidingWindowSizeArr)/sizeof(int);


const uint16_t _StartCodeLengthCode = 16;
const uint16_t _StopCodeLengthCode = 18+1;
const uint16_t _StartLengthCode = 257;
const uint16_t _StopLengthCode = 285-1;
const uint8_t _StartDistanceCode = 0;
const uint8_t _StopDistanceCode = 29+1;
const uint16_t _StartLength = 3;
const uint16_t _StartDistance = 1;

const int16_t _Lengthcode[] = {
  257,
  265,
  269,
  273,
  277,
  281,
  -285,
  286
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
  28,
  30
};


// use reversed binary data
class StaticHuffmanTree{
  private:
    // extra bits and start length
    vector<pair<uint8_t, uint16_t>> distanceCodes;
    vector<pair<uint8_t, int16_t>> lengthCodes;

  public:
    StaticHuffmanTree(){
      int i = _StartLengthCode, index = 0, num = _StartLength;
      while(index+1 < (sizeof(_Lengthcode)/sizeof(_Lengthcode[0]))){
        if(_Lengthcode[index] < 0){
          lengthCodes.push_back(pair<uint8_t, uint16_t>{0, num});
          num += 1;
          i++;
        }
        else{
          int _n = (int)pow(2, index);
          for(int n = 0; n < (_Lengthcode[index+1]-_Lengthcode[index]); n++){
            lengthCodes.push_back(pair<uint8_t, uint16_t>{index, num});
          printf("code %d, extrabits %d, startlen %d\n", i, index, num);
            num += _n;
            i++;
          }
        }

        index++;
      }

      i = _StartDistanceCode; index = 0; num = _StartDistance;
      while(index+1 < (sizeof(_Distancecode)/sizeof(_Distancecode[0]))){
        int _n = (int)pow(2, index);
        for(int n = 0; n < (_Distancecode[index+1]-_Distancecode[index]); n++){
          distanceCodes.push_back(pair<uint8_t, int16_t>{index, num});
          num += _n;
          i++;
        }

        index++;
      }
    }

    // if the code is length codes or distance codes,
    // it will return the extra bits.
    // for getting length, use GetExtra
    // -- first pair --
    // return value >= 0, means the literal
    // return value -1, means end of block
    // return value < -2, means code length -(val+2)
    // -- second pair --
    // return how many bits are used
    pair<int16_t, uint8_t> GetLeaf(uint16_t bits){
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
        // magic number 23 is subtraction of 280 and 257
        return pair<int16_t, uint8_t>{-(code - 0b11000000 + 23+ 2 + _StartLengthCode), bitcount};
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
        
        return pair<int16_t, uint8_t>{-(code - 0b0000001 + 2 + _StartLengthCode), bitcount};
      }

      return pair<int16_t, uint8_t>{0,0};
    }

    // return extra bits, for the distance, use GetExtra
    uint8_t GetDistanceExtraBits(uint16_t code){
      if(code > 29)
        return 0;

      return distanceCodes[code].first;
    }

    // return extra bits, for the full length, use GetExtra
    uint8_t GetLengthExtraBits(uint16_t code){
      if(code < 257 || code > 285)
        return 0;

      return lengthCodes[code-_StartLengthCode].first;
    }

    // can be used to get full length or distance
    // extraBits is the extra bits provided in the code
    // for extraBits, don't forget to revert back to the original binary form (not reversed)
    uint16_t GetExtra(uint16_t code, uint16_t extraBits){
      // distance
      if(code >= 0 && code <= 29)
        return distanceCodes[code].second + (extraBits & (0xffff >> 16-distanceCodes[code].first));

      // length
      else if(code >= 257 && code <= 285)
        return lengthCodes[code-_StartLengthCode].second + (extraBits & (0xff >> 8-lengthCodes[code-_StartLengthCode].first));

      return 0;
    }
};

struct tree{
  public:
    struct leaf{
      public:
        bool leafReady = false;
        uint16_t codenum = 0;

        leaf *right = NULL, *left = NULL;

        leaf(){}

        ~leaf(){
          if(right != NULL)
            delete right;
          
          if(left != NULL)
            delete left;
        }

        void SetLeaf(uint16_t code, uint16_t val, uint8_t bitLength){
          if(bitLength == 0){
            codenum = val;
            leafReady = true;
            return;
          }

          leaf *newleaf = new leaf{};
          if(algorithm_GetBit<uint16_t>(code, bitLength))
            right = newleaf;
          else
            left = newleaf;
          
          newleaf->SetLeaf(code, val, bitLength-1);
        }
    };

    leaf *firstleaf = NULL, *currentleaf = NULL;

    tree(){
      firstleaf = new leaf{};
    }

    ~tree(){
      delete firstleaf;
    }

    // bitCountOfTheCodes already sorted based on index
    void SetCode(uint8_t *bitCountOfTheCodes, uint16_t arraylength){
      vector<uint16_t> bitlengthcount{}, nextcode{};
      uint8_t maxBitsLength = 0;

      for(int i = 0; i < arraylength; i++){
        uint8_t currentBitLength = bitCountOfTheCodes[i];
        if(currentBitLength == 0)
          continue;

        if(currentBitLength > maxBitsLength)
          maxBitsLength = currentBitLength;

        if(bitlengthcount.size() > currentBitLength)
          bitlengthcount.insert(bitlengthcount.end(), i-bitlengthcount.size(), 0);

        bitlengthcount[currentBitLength-1]++;
      }

      nextcode.resize(maxBitsLength, 0);

      uint16_t code = 0;
      bitlengthcount[0] = 0;
      for(int bitsl = 1; bitsl <= maxBitsLength; bitsl++){
        code = (code + bitlengthcount[bitsl-1]) << 1;
        nextcode[bitsl-1] = code;
      }

      for(int i = 0; i < arraylength; i++){
        int currentBitLength = bitCountOfTheCodes[i];
        if(currentBitLength != 0){
          // assign the code
          firstleaf->SetLeaf(nextcode[currentBitLength-1], i, currentBitLength);
          nextcode[currentBitLength-1]++;
        }
      }
    }

    // this will iterate each leaf using the bits
    // return NULL if still branches,
    // return actual leaf pointer if it has codenum 
    leaf* GetCode(bool bit){
      if(bit)
        currentleaf = currentleaf->right;
      else
        currentleaf = currentleaf->left;

      if(currentleaf == NULL){
        currentleaf = firstleaf;
        return NULL;
      }
      
      if(currentleaf->leafReady){
        leaf *result = currentleaf;
        currentleaf = firstleaf;
        return result;
      }

      return NULL;
    }
};

// pair of base length and extrabits count
pair<uint16_t, uint8_t> _codeLengthCode[] {
  pair<uint16_t, uint8_t>(3, 2),
  pair<uint16_t, uint8_t>(3, 3),
  pair<uint16_t, uint8_t>(11, 7)
};

StaticHuffmanTree _StaticHuffmanTree{};
class HuffmanTree{
  private:
    tree _codeLengthTree, _litlenTree, _distTree;

  public:
    HuffmanTree(){}

    // the array length has to be 19 or _StopCodeLengthCode
    void SetCodeLength(uint8_t *bitCountOfTheCodes){
      _codeLengthTree.SetCode(bitCountOfTheCodes, _StopCodeLengthCode);
    }

    // the array length has to be 284 or _StopLengthCode
    void SetLitLen(uint8_t *litlenBitCount){
      _litlenTree.SetCode(litlenBitCount, _StopLengthCode);
    }

    // the array length has to be 30 or _StopDistanceCode
    void SetDist(uint8_t *distBitCount){
      _distTree.SetCode(distBitCount, _StopDistanceCode);
    }

    const tree::leaf* GetLeafCodeLength(bool bit){
      return _codeLengthTree.GetCode(bit);
    }

    const tree::leaf* GetLeafLitLen(bool bit){
      return _litlenTree.GetCode(bit);
    }

    const tree::leaf* GetLeafDist(bool bit){
      return _distTree.GetCode(bit);
    }

    // this will iterate each leaf using bits supplied
    // -- first pair --
    // return value >= 0, means the CodeLength (val+1)
    // return 0, if still needs to be iterated
    // return value < 0, means the length -(val)
    // -- second pair --
    // return how many extra bits
    pair<int16_t, uint8_t> GetCodeLength(bool bit){
      const tree::leaf* targetLeaf = GetLeafCodeLength(bit);
      if(targetLeaf != NULL){
        if(targetLeaf->codenum >= _StartCodeLengthCode)
          return pair<int16_t, uint8_t>{-targetLeaf->codenum, _codeLengthCode[targetLeaf->codenum-_StartCodeLengthCode].second};

        return pair<int16_t, uint8_t>{targetLeaf->codenum+1, 0};
      }

      return pair<int16_t, uint8_t>{};
    }

    // this will iterate each leaf using bits supplied
    // -- first pair --
    // return >= 0, means the literal (val+1)
    // return 0, if still needs to be iterated
    // returm < 0, means the length -(val)
    // -- second pair --
    // return how many -extra bits
    pair<int16_t, uint8_t> GetLitLen(bool bit){
      const tree::leaf* targetLeaf = GetLeafLitLen(bit);
      if(targetLeaf != NULL){
        if(targetLeaf->codenum >= _StartLengthCode){
          uint16_t code = targetLeaf->codenum;
          uint8_t extrabits = _StaticHuffmanTree.GetLengthExtraBits(code);
          return pair<int16_t, uint8_t>{-code, extrabits};
        }

        return pair<int16_t, uint8_t>{targetLeaf->codenum+1, 0};
      }

      return pair<int16_t, uint8_t>{0,0};
    }

    // this will iterate each leaf using bits supplied
    // -- first pair --
    // return the distance code (val+1)
    // return 0, if still needs to be iterated
    // -- second pair --
    // return how many extra bits
    pair<int16_t, uint8_t> GetDist(bool bit){
      const tree::leaf* targetLeaf = GetLeafDist(bit);
      if(targetLeaf != NULL)
        return pair<int16_t, uint8_t>{targetLeaf->codenum, _StaticHuffmanTree.GetLengthExtraBits(targetLeaf->codenum+1)};

      return pair<int16_t, uint8_t>{};
    }

    // return the actual length based on the code
    uint16_t GetExtraCodeLength(uint16_t code, uint8_t extraBits){
      if(code >= _StartCodeLengthCode && code < _StopCodeLengthCode){
        code -= _StartCodeLengthCode;
        return _codeLengthCode[code].first + (extraBits & (0xff >> 8-_codeLengthCode[code].second));
      }

      return 0;
    }

    // return full length or distance based on the code
    uint16_t GetExtraLenDist(uint16_t code, uint16_t extrabits){
      return _StaticHuffmanTree.GetExtra(code, extrabits);
    }
};


pair<char*, size_t> algorithm_DEFLATE_decompress(char *data, size_t datasize, int compressionType){
  const int SlidingWindowSize = __slidingWindowSizeArr[compressionType];

  const int reserveStorage = 256;
  size_t currentResultLength = reserveStorage, actualResultLength = 0;
  char *result = (char*)malloc(sizeof(char)*currentResultLength);

  bool keepLooping = true;
  size_t offsetbits = 0;
  while(keepLooping){
    keepLooping = !algorithm_GetBit(data[offsetbits/8], 0);
    cout << "Keeplooping " << keepLooping << endl;

    uint8_t HuffmanCodingType = (data[offsetbits/8] >> 1) & 0b11;
    cout << "HuffmanCodingType: " << (int)HuffmanCodingType << endl;
    offsetbits += 3;

    switch(HuffmanCodingType){
      // no compression
      break; case 0b00:{
        // skipping some bits
        offsetbits = offsetbits+(8-(offsetbits%8));
        uint16_t len = *reinterpret_cast<uint16_t*>(data+(offsetbits/8));
        
        // also skipping nlen
        offsetbits += sizeof(uint16_t)*8*2;
        actualResultLength += len;
        if(actualResultLength >= currentResultLength)
          result = (char*)realloc(result, ((actualResultLength/reserveStorage)+1)*reserveStorage);

        memcpy(result+(actualResultLength-len), data+(offsetbits/8), len);
        offsetbits += len*8;
        cout << "actualResultLength: " << actualResultLength << endl;
        cout << "offsetbits: " << offsetbits << endl;
      }

      // fixed Huffman coding
      break; case 0b01:{
        while(true){
          cout << "offset: " << offsetbits << endl;
          uint16_t bitsBetweenOffset = algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 16);
          auto res = _StaticHuffmanTree.GetLeaf(bitsBetweenOffset);
          offsetbits += res.second;

          cout << "code: " << hex << bitsBetweenOffset << dec << endl;
          
          // value is literal
          if(res.first >= 0){
            result[actualResultLength++] = res.first & 0xff;
            if(actualResultLength >= currentResultLength){
              currentResultLength += reserveStorage;
              result = (char*)realloc(result, sizeof(char) * currentResultLength);
            }

            cout << "literal: " << (res.first & 0xff) << endl;
          }

          // end of block
          else if(res.first == -1){
            cout << "end of block" << endl;
            offsetbits += res.second;
            break;
          }
          
          // value is the lenght code
          else{
            uint16_t length, distance;
            uint16_t lengthcode = -2-res.first;
            cout << lengthcode << endl;
            
            uint8_t extrabitscount = _StaticHuffmanTree.GetLengthExtraBits(lengthcode);
            length = _StaticHuffmanTree.GetExtra(lengthcode,

              // might need to optimize
              algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabitscount), extrabitscount));

            offsetbits += extrabitscount;
            uint8_t distancecode = algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5);

            cout << (int)distancecode << endl;

            offsetbits += 5;
            extrabitscount = _StaticHuffmanTree.GetDistanceExtraBits(distancecode);
            distance = _StaticHuffmanTree.GetExtra(distancecode,

              // might need to optimize
              algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabitscount), extrabitscount));

            offsetbits += extrabitscount;

            cout << "length: " << length << " distance: " << distance << endl;
            // copying using length and distance
            for(int i_l = 0; i_l < length; i_l++){
              result[actualResultLength] = result[actualResultLength-distance];

              actualResultLength++;
              if(actualResultLength >= currentResultLength-1){
                currentResultLength += reserveStorage;
                result = (char*)realloc(result, sizeof(char) * currentResultLength);
              }
            }
          }
        }
      }

      // dynamic Huffman coding
      break; case 0b10:{
        uint16_t litlenmax = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5), 5);
        offsetbits += 5;

        uint16_t distmax = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5), 5);
        offsetbits += 5;

        uint16_t numofcodelen = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 4), 4) + 4;
        offsetbits += 4;

        const uint8_t __ReorderedIndex[] {
          16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
        };

        uint8_t *codeLengthArr = (uint8_t*)calloc(_StopCodeLengthCode, 1);
        for(int i = 0; i < numofcodelen; i++){
          codeLengthArr[__ReorderedIndex[i]] = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 3), 3);
          offsetbits += 3;
        }

        HuffmanTree ht{};
        ht.SetCodeLength(codeLengthArr);

        size_t maxoffsetbits = datasize*8;

        // code length parts
        uint8_t *litlenArr = (uint8_t*)calloc(_StopLengthCode, 1);
        uint16_t offsetlitlen = 0;
        while(offsetbits < maxoffsetbits){
          bool currentBit = algorithm_GetBitR(data[offsetbits/8], offsetbits%8);
          auto currentPair = ht.GetCodeLength(currentBit);
          if(currentPair.first != 0){
            if(currentPair.first > 0){
              uint16_t val = currentPair.first-1;
              litlenArr[offsetlitlen++] = val;
            }
            else{
              uint16_t val = -currentPair.first;
              uint8_t extrabits = currentPair.second;
              uint16_t length = ht.GetExtraCodeLength(val,
                algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabits), extrabits)
              );

              uint8_t copyCode = 

              if(val == 16){
                uint8_t previousBitLength = litlenArr[offsetlitlen-1];
                for(int i = 0; i < length; i++)
                  litlenArr[offsetlitlen++] = previousBitLength;

              }

              else{
                for(int i = 0; i < length; i++)
                  litlenArr[offsetlitlen++] = 0;

              }
            }
          }

          offsetbits++;
        }

        free(codeLengthArr);
      }
      
      //reserved
      break; default:{
        continue;
      }
    }
  }

  if(actualResultLength < currentResultLength)
    result = (char*)realloc(result, sizeof(char) * actualResultLength);
  
  return pair<char*, size_t>{result, actualResultLength};
}