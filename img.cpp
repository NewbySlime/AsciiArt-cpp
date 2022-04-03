#include "img.h"
#include "math.h"
#include "vectormath.h"
#include "sstream"
#include "algorithms.h"

using namespace std;


size_t GetData(istream &bis){
  return 0;
}

template<typename type> size_t GetData(istream &bis, type* data){
  char *databyte = reinterpret_cast<char*>(data);
  bis.read(databyte, sizeof(type));
  return sizeof(type);
}

template<typename t1, typename... t2> size_t GetData(istream &bis, t1* data, t2*... datas){
  return GetData(bis, data) +
  GetData(bis, datas...);
}


constexpr unsigned int CharsToUint32(char *buf){
  uint32_t res = (buf[0] << (8*3)) | (buf[1] << (8*2)) | (buf[2] << 8) | buf[3];
  return res;
}


// ** ImageData Class ** //
ImageData::ImageData(){}

ImageData::ImageData(const ImageData &imgdat){
  Width = imgdat.Width;
  Height = imgdat.Height;
  PixelDataSize = imgdat.PixelDataSize;
  PixelData = new unsigned char[imgdat.PixelDataSize];
  memcpy(PixelData, imgdat.PixelData, PixelDataSize);
}

ImageData::ImageData(const BitmapFileData &bmp){
  Width = bmp.Width;
  Height = bmp.Height;
  PixelDataSize = (int)ceil((double)bmp.ColorDepth*Width*Height/8);
  PixelData = new unsigned char[PixelDataSize];

  int widthbytes = (int)ceil((float)Width*bmp.ColorDepth/8);
  int padding = (widthbytes%4);
  padding = padding != 0? 4-padding: 0;

  switch(bmp.ColorDepth){
    break; case COLORD_RGBA8:
      isAlphaUsed = true;

    case COLORD_RGB8:{
      size_t offset = 0, offsetPixel = 0;
      int i = 0;
      while(offset < bmp.ImageSize && offsetPixel < PixelDataSize){
        memcpy(PixelData+offsetPixel, bmp.PixelData+offset, widthbytes);
        i++;
        offset += widthbytes + padding;
        offsetPixel += widthbytes;
      }
    }
  }
}

ImageData::ImageData(const PNGFileData &png){

}

ImageData::~ImageData(){
  if(PixelData != NULL)
    delete[] PixelData;
}

ImageData ImageData::ResizeImage(int w, int h){
  return ResizeImage(*this,w,h);
}

// reusing: http://www.tech-algorithm.com/articles/bilinear-image-scaling/
ImageData ImageData::ResizeImage(ImageData &img, int w, int h){
  ImageData newimg;
  int manyColors = 3+img.isAlphaUsed;
  size_t newImgDataSize = w*h*manyColors;
  unsigned char *newImgData = new unsigned char[w*h*(3+img.isAlphaUsed)];
  float x_ratio = (float)(img.Width-1)/w;
  float y_ratio = (float)(img.Height-1)/h;
  vec4<float> A, B, C, D;
  
  for(int i = 0; i < h; i++)
    for(int o = 0; o < w; o++){
      int x_i = (int)(x_ratio*o);
      int y_i = (int)(y_ratio*i);
      float x_diff = x_ratio*o - x_i;
      float y_diff = y_ratio*i - y_i;

      int index = x_i+(y_i*img.Width);
      A = vec4<float>(img.PixelData+(index*manyColors), img.isAlphaUsed);
      B = vec4<float>(img.PixelData+((index+1)*manyColors), img.isAlphaUsed);
      C = vec4<float>(img.PixelData+((index+img.Width)*manyColors), img.isAlphaUsed);
      D = vec4<float>(img.PixelData+((index+img.Width+1)*manyColors), img.isAlphaUsed);

      vec4<float> result =
        A*(1-x_diff)*(1-y_diff) +
        B*(x_diff)*(1-y_diff)   +
        C*(y_diff)*(1-x_diff)   +
        D*(x_diff*y_diff)
      ;

      result.CopyToMemory(newImgData+((o+i*w)*manyColors), img.isAlphaUsed);
    }
  
  newimg.Height = h;
  newimg.Width = w;
  newimg.isAlphaUsed = img.isAlphaUsed;
  newimg.PixelData = newImgData;
  newimg.PixelDataSize = newImgDataSize;

  return newimg;
}


// ** BmpOpener Class ** //
BmpOpener::BmpOpener(){

}

int BmpOpener::Open(string fp, uint16_t id){
  return BmpOpener::Open(fp, bmpdat);
}

int BmpOpener::Open(string fp, BitmapFileData& bmps, uint16_t idField){
  ifstream ifs;
  ifs.open(fp, ifs.binary);

  return Open(ifs, bmps, idField);
}

int BmpOpener::Open(std::ifstream &ifs, BitmapFileData &bmps, uint16_t idField){
  uint16_t idfield = 0;
  GetData(ifs, &idfield);

  if(idfield != BMP_IDHEAD)
    return ERR_FILE_WRONGFILE;
  
  GetData(ifs, &bmps.FileSize);
  
  // seeking after reserved bytes
  ifs.seekg((int)ifs.tellg()+4);
  GetData(ifs, &bmps.DataOffset);

  uint32_t infoSize = 0;
  GetData(ifs, &infoSize);

  // getting info header data based on the size of the info
  switch(infoSize){
    break; case _BITMAPINFOHEADER:{
      GetData(ifs,
        &bmps.Width,
        &bmps.Height,
        &bmps.Planes,
        &bmps.ColorDepth,
        &bmps.Compression,
        &bmps.ImageSize,
        &bmps.PPMX,
        &bmps.PPMY,
        &bmps.ColorsUsed,
        &bmps.ColorsImportant
      );
    }

    break; default:
      return ERR_BMP_UNSUPPORTEDHEADERINFO;
  }

  char *pixelData = NULL,
    *paletteData = NULL;

  pixelData = new char[bmps.ImageSize];
  ifs.read(pixelData, bmps.ImageSize);

  switch(bmps.Compression){
    break; case BI_RGB:
    
    break; default:
      return ERR_BMP_UNSUPPORTERCOMPRESSION;
  }


  if(bmps.ColorsUsed > 0){

  }
  else
    bmps.PixelData = pixelData;


  return ERR_FILE_SUCCESS;
}

ImageData BmpOpener::GetImageData(){
  return bmpdat;
}


// ** PNGOpener Class ** //
PNGOpener::PNGOpener(){

}

int PNGOpener::Open(std::string filepath, uint16_t idField = 0){
  return Open(filepath, pngdat, idField);
}

int PNGOpener::Open(std::string filepath, PNGFileData &pngstruct, uint16_t idField = 0){
  ifstream ifs;
  ifs.open(filepath);
  return Open(ifs, pngstruct, idField);
}

int PNGOpener::Open(std::ifstream &ifs, PNGFileData &pngstruct, uint16_t idField = 0){
  uint64_t fileSignature = 0;
  GetData(ifs, &fileSignature);

  if(fileSignature != PNG_IDHEAD)
    return ERR_FILE_WRONGFILE;

  while(true && ifs.eof()){
    uint32_t headerChunk = 0, chunkSize = 0, offset = 0;
    GetData(ifs, &headerChunk, &chunkSize);

    switch(headerChunk){
      break; case CharsToUint32("IHDR"):{
        uint8_t Colortype = 0;
        offset = GetData(ifs,
          &pngstruct.Width,
          &pngstruct.Height,
          &pngstruct.BitDepth,
          &Colortype,
          &pngstruct.CompressionMethod,
          &pngstruct.FilterMethod,
          &pngstruct.InterlaceMethod
        );

        pngstruct.BitsPerChannel = Colortype >> 3;
        pngstruct.Alpha = algorithm_GetBit(Colortype, 2);
        pngstruct.Truecolor = algorithm_GetBit(Colortype, 1);
        pngstruct.Palette = algorithm_GetBit(Colortype, 0);

        switch(pngstruct.CompressionMethod){
          break; case PNG_COMPRESSION_DEFLATE:
          break; default:
            return ERR_PNG_UNSUPPORTEDCOMPRESSION;
        }
        
        switch(pngstruct.FilterMethod){
          break; case PNG_FILTER_DEFAULT:
          break; default:
            return ERR_PNG_UNSUPPORTEDFILTER;
        }

        switch(pngstruct.InterlaceMethod){
          break; case PNG_INTERLACE_NONE:
          break; case PNG_INTERLACE_ADAM7:
          break; default:
            return ERR_PNG_UNSUPPORTEDINTERLACING;
        }
      }

      break; case CharsToUint32("IDAT"):{
        uint32_t CompressionMethod = 0; // ?

        offset = GetData(ifs,
          &CompressionMethod,
          &pngstruct.ZLibFcheck
        );

        pngstruct.CompressedBlockLen = chunkSize-offset-4;
        pngstruct.CompressedBlock = new char[pngstruct.CompressedBlockLen];
        ifs.read(pngstruct.CompressedBlock, pngstruct.CompressedBlockLen);

        offset += pngstruct.CompressedBlockLen;

        uint32_t ZLibChecksum = 0;
        offset += GetData(ifs, &ZLibChecksum);
      }

      break; case CharsToUint32("PLTE"):{
        if(!pngstruct.Palette)
          break;
      }

      break; case CharsToUint32("IEND"):
        return ERR_FILE_SUCCESS;

      break; default:{
        // cout << "Unsupported ancillary chunks: 0x" << hex << headerChunk << dec << endl;
      }
    }

    if(offset < headerChunk)
      ifs.seekg((size_t)ifs.tellg()+headerChunk-offset);

    uint32_t CRCValue = 0;
    GetData(ifs, &CRCValue);
  }

  return ERR_PNG_NOIENDCHUNK;
}

ImageData PNGOpener::GetImageData(){
  return pngdat;
}