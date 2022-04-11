#include "algorithms.h"

#include "stdint.h"
#include "vector"
#include "map"
#include "math.h"

#include "log.h"
#include "iostream"

using namespace std;

template<typename t_num> bool algorithm_GetBit(t_num data, uint8_t index){
  return (data >> index) & 0b1;
}

template bool algorithm_GetBit(char, uint8_t);
template bool algorithm_GetBit(unsigned char, uint8_t);


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
      PRINTLOG("Setting up static Huffman tree...\n\nLength codes,");

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
            PRINTLOG("\tCode %d, extrabits %d, baseLength %d\n", i, index, num);
            num += _n;
            i++;
          }
        }

        index++;
      }

      PRINTLOG("\n\nDistance codes,\n");

      i = _StartDistanceCode; index = 0; num = _StartDistance;
      while(index+1 < (sizeof(_Distancecode)/sizeof(_Distancecode[0]))){
        int _n = (int)pow(2, index);
        for(int n = 0; n < (_Distancecode[index+1]-_Distancecode[index]); n++){
          distanceCodes.push_back(pair<uint8_t, int16_t>{index, num});
          PRINTLOG("\tCode %d, extrabits %d, baseDistance %d\n", i, index, num);
          num += _n;
          i++;
        }

        index++;
      }

      PRINTLOG("\nDone setting up static tree.\n\n");
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

          leaf *currleaf;
          if(algorithm_GetBit<uint16_t>(code, bitLength-1)){
            PRINTLOG("1");
            if(right == NULL)
              right = new leaf{};
            
            currleaf = right;
          }
          else{
            PRINTLOG("0");
            if(left == NULL)
              left = new leaf{};

            currleaf = left;
          }

          currleaf->SetLeaf(code, val, bitLength-1);
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
      PRINTLOG("\nSetting up tree...\n");
      vector<uint16_t> bitlengthcount{}, nextcode{};
      uint8_t maxBitsLength = 0;

      for(int i = 0; i < arraylength; i++){
        uint8_t currentBitLength = bitCountOfTheCodes[i];
        if(currentBitLength == 0)
          continue;

        if(currentBitLength > maxBitsLength)
          maxBitsLength = currentBitLength;

        if(bitlengthcount.size() < currentBitLength)
          bitlengthcount.insert(bitlengthcount.end(), currentBitLength-bitlengthcount.size(), 0);

        bitlengthcount[currentBitLength-1]++;
      }

      nextcode.resize(maxBitsLength, 0);

      uint16_t code = 0;
      bitlengthcount[0] = 0;
      for(int bitsl = 1; bitsl <= maxBitsLength; bitsl++){
        // added ifs here, not from the rfc specified
        if(bitlengthcount[bitsl-1] != 0){
          code = (code + bitlengthcount[bitsl-1]) << 1;
          nextcode[bitsl-1] = code;
        }
      }

      for(int i = 0; i < arraylength; i++){
        int currentBitLength = bitCountOfTheCodes[i];
        if(currentBitLength != 0){
          // assign the code
          PRINTLOG("\tcode val: %d, codewords: ", i);
          firstleaf->SetLeaf(nextcode[currentBitLength-1], i, currentBitLength);
          PRINTLOG("\n");
          nextcode[currentBitLength-1]++;
        }
      }

      currentleaf = firstleaf;
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
    enum TreeType{
      CodeLength,
      Litlen,
      Distance
    };

    HuffmanTree(){}


    // for CodeLength, the array length has to be 19 or _StopCodeLengthCode
    // for Literal/Length, the array length has to be 284 or _StopLengthCode
    // for Distance, the array length has to be 30 or _StopDistanceCode
    void SetCode(uint8_t *bitCountArr, TreeType type){
      switch(type){
        break; case CodeLength:
          _codeLengthTree.SetCode(bitCountArr, _StopCodeLengthCode);
        
        break; case Litlen:
          _litlenTree.SetCode(bitCountArr, _StopLengthCode);
        
        break; case Distance:
          _distTree.SetCode(bitCountArr, _StopDistanceCode);
      }
    }

    const tree::leaf* GetLeaf(bool bit, TreeType type){
      switch(type){
        break; case CodeLength:
          return _codeLengthTree.GetCode(bit);
        
        break; case Litlen:
          return _litlenTree.GetCode(bit);
        
        break; case Distance:
          return _distTree.GetCode(bit);
      }

      return NULL;
    }

    // this will iterate each leaf using bits supplied
    // -- first pair --
    // return value > 0, means the CodeLength/Literal/Distance (val+1)
    // return 0, if still needs to be supplied
    // return value < 0, means the length -(val)
    // -- second pair --
    // return how many extra bits
    pair<int16_t, uint8_t> GetCode(bool bit, TreeType type){
      const tree::leaf* targetLeaf = GetLeaf(bit, type);
      if(targetLeaf != NULL){
        switch(type){
          break; case CodeLength:
            if(targetLeaf->codenum >= _StartCodeLengthCode)
              return pair<int16_t, uint8_t>{-targetLeaf->codenum, _codeLengthCode[targetLeaf->codenum-_StartCodeLengthCode].second};

          break; case Litlen:
            if(targetLeaf->codenum >= _StartLengthCode){
              uint16_t code = targetLeaf->codenum;
              uint8_t extrabits = _StaticHuffmanTree.GetLengthExtraBits(code);
              return pair<int16_t, uint8_t>{-code, extrabits};
            }

          break; case Distance:
            return pair<int16_t, uint8_t>{targetLeaf->codenum+1, _StaticHuffmanTree.GetDistanceExtraBits(targetLeaf->codenum)};
        }

        return pair<int16_t, uint8_t>{targetLeaf->codenum+1, 0};
      }

      return pair<int16_t, uint8_t>{0,0};
    }

    // return the actual length based on the code
    uint16_t GetExtraCodeLength(uint16_t code, uint8_t extraBits){
      if(code < _StartCodeLengthCode || code >= _StopCodeLengthCode)
        return 0;

      code -= _StartCodeLengthCode;
      return _codeLengthCode[code].first + (extraBits & (0xff >> 8-_codeLengthCode[code].second));
    }

    // return full length or distance based on the code
    uint16_t GetExtraLenDist(uint16_t code, uint16_t extrabits){
      return _StaticHuffmanTree.GetExtra(code, extrabits);
    }
};


pair<char*, size_t> algorithm_DEFLATE_decompress(char *data, size_t datasize, int compressionType){
  PRINTLOG("\nStart deflating compressed data.")
  const int SlidingWindowSize = __slidingWindowSizeArr[compressionType];

  const int reserveStorage = 256;
  size_t currentResultLength = reserveStorage, actualResultLength = 0;
  char *result = (char*)malloc(sizeof(char)*currentResultLength);

  bool keepLooping = true;
  size_t offsetbits = 0;
  while(keepLooping){
    keepLooping = !algorithm_GetBit(data[offsetbits/8], 0);
    PRINTLOG("\nIs last block on the stream: %s\n", keepLooping? "FALSE": "TRUE");

    uint8_t HuffmanCodingType = (data[offsetbits/8] >> 1) & 0b11;
    offsetbits += 3;

    switch(HuffmanCodingType){
      // no compression
      break; case 0b00:{
        // skipping some bits
        offsetbits = offsetbits+(8-(offsetbits%8));
        uint16_t len = *reinterpret_cast<uint16_t*>(data+(offsetbits/8));
        PRINTLOG("\nNo Compression block,\nBlock size: %d\n", len);
        
        // also skipping nlen
        offsetbits += sizeof(uint16_t)*8*2;

        actualResultLength += len;
        if(actualResultLength >= currentResultLength)
          result = (char*)realloc(result, ((actualResultLength/reserveStorage)+1)*reserveStorage);

        memcpy(result+(actualResultLength-len), data+(offsetbits/8), len);
        offsetbits += len*8;
      }

      // fixed Huffman coding
      break; case 0b01:{
        PRINTLOG("\nStatic Huffman coding,");
        while(true){
          PRINTLOG("\nStarting index bit in byte 0x%X: %d\n", offsetbits/8, offsetbits%8);

          uint16_t bitsBetweenOffset = algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 16);
          auto res = _StaticHuffmanTree.GetLeaf(bitsBetweenOffset);
          offsetbits += res.second;

          PRINTLOG("\nUsing codewords: 0x%X, with bit length: %d\n", bitsBetweenOffset, res.second);
          
          // value is literal
          if(res.first >= 0){
            result[actualResultLength++] = res.first & 0xff;
            if(actualResultLength >= currentResultLength){
              currentResultLength += reserveStorage;
              result = (char*)realloc(result, sizeof(char) * currentResultLength);
            }

            PRINTLOG("\tUsing literal: %d\n", res.first & 0xff);
          }

          // end of block
          else if(res.first == -1){
            PRINTLOG("\tEnd of the block");
            break;
          }
          
          // value is the lenght code
          else{
            uint16_t length, distance;
            uint16_t lengthcode = -2-res.first;
            PRINTLOG("\tUsing length code: %d\n", lengthcode);
            
            uint8_t extrabitscount = _StaticHuffmanTree.GetLengthExtraBits(lengthcode);
            length = _StaticHuffmanTree.GetExtra(lengthcode,

              // might need to optimize
              algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabitscount), extrabitscount));

            offsetbits += extrabitscount;
            uint8_t distancecode = algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5);

            PRINTLOG("\tProcessed length: %d, extrabits: %d\n", length, extrabitscount);
            PRINTLOG("\tUsing distance code: %d\n", distancecode);

            offsetbits += 5;
            extrabitscount = _StaticHuffmanTree.GetDistanceExtraBits(distancecode);
            distance = _StaticHuffmanTree.GetExtra(distancecode,

              // might need to optimize
              algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabitscount), extrabitscount));

            offsetbits += extrabitscount;

            PRINTLOG("\tProcessed distance: %d, extrabits: %d\n", distance, extrabitscount);

            // copying using length and distance
            for(int i_l = 0; i_l < length; i_l++){
              result[actualResultLength] = result[actualResultLength-distance];

              actualResultLength++;
              if(actualResultLength >= currentResultLength){
                currentResultLength += reserveStorage;
                result = (char*)realloc(result, sizeof(char) * currentResultLength);
              }
            }
          }
        }
      }

      // dynamic Huffman coding
      break; case 0b10:{
        PRINTLOG("\nDynamic Huffman coding,\n");

        uint16_t litlenmax = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5), 5) + _StartLengthCode;
        offsetbits += 5;
        PRINTLOG("Litlen maximum code: %d\n", litlenmax);

        uint16_t distmax = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 5), 5) + 1;
        offsetbits += 5;
        PRINTLOG("Distance maximum code: %d\n", distmax);

        uint16_t numofcodelen = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 4), 4) + 4;
        offsetbits += 4;
        PRINTLOG("Code length maximum code: %d\n\n", numofcodelen);


        const uint8_t __ReorderedIndex[] {
          16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
        };

        uint8_t *codeLengthArr = (uint8_t*)calloc(_StopCodeLengthCode, 1);
        for(int i = 0; i < numofcodelen; i++){
          codeLengthArr[__ReorderedIndex[i]] = algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, 3), 3);
          offsetbits += 3;
        }

        HuffmanTree ht{};
        ht.SetCode(codeLengthArr, HuffmanTree::TreeType::CodeLength);
        free(codeLengthArr);

        size_t maxoffsetbits = datasize*8;

        uint8_t *litlenArr = (uint8_t*)calloc(_StopLengthCode, 1);
        uint8_t *distArr = (uint8_t*)calloc(_StopDistanceCode, 1);
        uint8_t *byteArrays[]{litlenArr, distArr};

        uint16_t offsets[]{0,0};
        uint16_t offsetsMax[]{litlenmax, distmax};

        static HuffmanTree::TreeType treetypesSet[]{
          HuffmanTree::TreeType::Litlen,
          HuffmanTree::TreeType::Distance
        },
        
        treetypesGet[]{
          HuffmanTree::TreeType::CodeLength,
          HuffmanTree::TreeType::CodeLength,
          HuffmanTree::TreeType::Litlen
        };

        #ifdef DO_LOG
        const char *_promptstr[]{
          "Getting litlen codewords...",
          "Getting distance codewords...",
          "Decoding literals..."
        };
        #endif

        PRINTLOG("\nDecoding Litlen and Dist, and getting literals...");

        // getting the litlen codes
        for(int i = 0; i < 3; i++){
          PRINTLOG("\n%s\nStarting at index 0x%X, bitoffset %d: ", _promptstr[i], offsetbits/8, offsetbits%8);
          while(offsetbits < maxoffsetbits && (treetypesGet[i] == HuffmanTree::Litlen || offsets[i] < offsetsMax[i])){
            bool currentBit = algorithm_GetBit(data[(offsetbits)/8], (offsetbits
            )%8);
            PRINTLOG("%s", currentBit? "1": "0");
            
            offsetbits++;
            auto currentPair = ht.GetCode(currentBit, treetypesGet[i]);
            if(currentPair.first != 0){
              PRINTLOG("\n\tCodeword found, value: %d\n", currentPair.first-1);

              if(currentPair.first > 0){
                if(treetypesGet[i] == HuffmanTree::Litlen){
                  // end of block
                  if((currentPair.first-1) == 256){
                    PRINTLOG("\tEnd of block\n");
                    break;
                  }

                  PRINTLOG("\tIs literal, value: %d, in index: %d\n", currentPair.first-1, actualResultLength);
                  result[actualResultLength++] = currentPair.first-1;

                  if(actualResultLength >= currentResultLength){
                    currentResultLength += reserveStorage;
                    result = (char*)realloc(result, sizeof(char) * currentResultLength);
                  }
                }
                else{
                  uint16_t val = currentPair.first-1;

                  PRINTLOG("\tCode %d has codewords length: %d\n", offsets[i], val);
                  byteArrays[i][offsets[i]++] = val;
                }
              }
              else{
                if(treetypesGet[i] == HuffmanTree::Litlen){
                  uint16_t length, distance;
                  length = ht.GetExtraLenDist(-(currentPair.first), 
                    algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, currentPair.second), currentPair.second)
                  );
                  PRINTLOG("\tIs literal length,\n\tLength: %d, extrabitscount: %d\n", length, currentPair.second);

                  offsetbits += currentPair.second;
                  
                  pair<int16_t, uint8_t> distancePair{};
                  while(offsetbits < maxoffsetbits && (distancePair = ht.GetCode(algorithm_GetBit(data[offsetbits/8], offsetbits%8), HuffmanTree::Distance)).first == 0)
                    offsetbits++;

                  offsetbits++;

                  distance = ht.GetExtraLenDist(distancePair.first-1,
                    algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, distancePair.second), distancePair.second)
                  );
                  PRINTLOG("\tDistance: %d, extrabitscount: %d\n", distance, distancePair.second);
                  
                  offsetbits += distancePair.second;

                  PRINTLOG("\tCopied content (in hex) from 0x%X: \n", actualResultLength);
                  // copying using length and distance
                  for(int i_l = 0; i_l < length; i_l++){
                    result[actualResultLength] = result[actualResultLength-distance];
                    if(i_l % 16)
                      PRINTLOG("\n\t\t");
                      
                    PRINTLOG("%X ", result[actualResultLength]);

                    actualResultLength++;
                    if(actualResultLength >= currentResultLength){
                      currentResultLength += reserveStorage;
                      result = (char*)realloc(result, sizeof(char) * currentResultLength);
                    }
                  }

                  PRINTLOG("\n\tEnded at index 0x%X\n", actualResultLength);
                }
                else{
                  uint16_t val = -currentPair.first;
                  uint8_t extrabits = currentPair.second;
                  uint16_t length = ht.GetExtraCodeLength(val,
                    algorithm_ReverseBits(algorithm_GetBitsBetweenR(data+(offsetbits/8), offsetbits%8, extrabits), extrabits)
                  );

                  offsetbits += extrabits;

                  uint8_t copyCode = val == 16? byteArrays[i][offsets[i]-1]: 0;

                  PRINTLOG("\tLength: %d, extrabitscount: %d\n\tCopyCode: %d\n", length, extrabits, copyCode);
                  
                  for(int i_count = 0; i_count < length && offsets[i] < offsetsMax[i]; i_count++)
                    byteArrays[i][offsets[i]++] = copyCode;
                }
              }

              PRINTLOG("\n");
            }
          }

          if(i < (sizeof(treetypesSet)/sizeof(treetypesSet[0]))){
            ht.SetCode(byteArrays[i], treetypesSet[i]);
            PRINTLOG("\n");
          }
        }

        free(litlenArr);
        free(distArr);
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
