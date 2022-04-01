#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <iostream>
#include <string>
#include <fstream>

#define SupportedFileTypeStr "BMP"

// ID field of the BMP
#define BMP_IDHEAD 0x4d42

// Compression method
#define BI_RGB 0

// DIB header size
#define _BITMAPINFOHEADER 40

// ColorDepth
#define COLORD_RGBA8 32
#define COLORD_RGB8 24

// Error codes when opening bmp files
#define ERR_BMP_DATANOTSUFFICIENT 1           // ifstream reached EOF before the data is supplied
#define ERR_BMP_SUCCESS 0
#define ERR_BMP_WRONGID -1
#define ERR_BMP_UNSUPPORTEDHEADERINFO -2
#define ERR_BMP_UNSUPPORTERCOMPRESSION -3

struct BitmapFileData;

class ImageData{
  public:
    int32_t Width, Height;
    size_t PixelDataSize;
    bool isAlphaUsed = false;
    
    // this will be converted to 8-bit-rgb or 8-bit-rgba colordepth
    unsigned char *PixelData = NULL;

    ImageData();
    ImageData(const BitmapFileData &bmp);
    ImageData(const ImageData &imgdat);

    ~ImageData();

    ImageData ResizeImage(int toWidth, int toHeight);
    static ImageData ResizeImage(ImageData &img, int toWidth, int toHeight);
};

struct BitmapFileData{
  public:
    uint32_t FileSize = 0, DataOffset = 0, Compression = 0, ImageSize = 0, PPMX = 0, PPMY = 0, ColorsUsed = 0, ColorsImportant = 0, PaddingLength = 0;

    int32_t Width = 0, Height = 0;
    
    uint16_t Planes = 0, ColorDepth = 0;

    // consists of rgba, and based on the ColorDepth
    // even if the bmp uses color palette, it will be converted to colors per pixel
    char *PixelData = NULL;
};

class BmpOpener{
  private:
    BitmapFileData bmpdat{};

  public:
    BmpOpener();

    // return error code if cannot get the image
    // and will deleting the last image data used
    // if idField is 0, it will read the header
    int Open(std::string filepath, uint16_t idField = 0);
    static int Open(std::string filepath, BitmapFileData& bmpstruct, uint16_t idField = 0);
    static int Open(std::ifstream &ifs, BitmapFileData &bmpstruct, uint16_t idField = 0);

    ImageData GetImageData();
};

#endif