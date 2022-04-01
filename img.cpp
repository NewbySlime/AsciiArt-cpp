#include "img.h"
#include "math.h"
#include "vectormath.h"
#include "sstream"

using namespace std;


void GetData(istream &bis){

}

template<typename type> void GetData(istream &bis, type* data){
  char *databyte = reinterpret_cast<char*>(data);
  bis.read(databyte, sizeof(type));
}

template<typename t1, typename... t2> void GetData(istream &bis, t1* data, t2*... datas){
  GetData(bis, data);
  GetData(bis, datas...);
}

size_t GetData(char *buf){
  return 0;
}

// return byte offset
template<typename type> size_t GetData(char *buf, type* data){
  memcpy(data, buf, sizeof(type));
  return sizeof(type);
}

template<typename t1, typename... t2> size_t GetData(char *buf, t1* data, t2*... datas){
  size_t currentOffset = GetData(buf, data);
  currentOffset += GetData(buf+currentOffset, datas...);
  return currentOffset;
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
    return ERR_BMP_WRONGID;
  
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


  return ERR_BMP_SUCCESS;
}


ImageData BmpOpener::GetImageData(){
  return bmpdat;
}