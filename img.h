#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <iostream>
#include <string>
#include <fstream>

#define SupportedFileTypeStr "BMP"

// ID field of the BMP
#define BMP_IDHEAD 0x4d42
// ID field of the PNG
#define PNG_IDHEAD 0x89504e470d0a1a0a

// Compression method
#define BI_RGB 0

// DIB header size
#define _BITMAPINFOHEADER 40

// ColorDepth
#define COLORD_RGBA8 32
#define COLORD_RGB8 24

#define PNG_COMPRESSION_DEFLATE 0

#define PNG_FILTER_DEFAULT 0

#define PNG_INTERLACE_NONE 0
#define PNG_INTERLACE_ADAM7 1


#define ERR_FILE_WRONGFILE -1
#define ERR_FILE_SUCCESS 0

// Error codes when opening bmp files
#define ERR_BMP_DATANOTSUFFICIENT 1           // ifstream reached EOF before the data is supplied
#define ERR_BMP_UNSUPPORTEDHEADERINFO -2
#define ERR_BMP_UNSUPPORTERCOMPRESSION -3


#define ERR_PNG_NOIENDCHUNK 1
#define ERR_PNG_UNSUPPORTEDCOMPRESSION -2
#define ERR_PNG_UNSUPPORTEDFILTER -3
#define ERR_PNG_UNSUPPORTEDINTERLACING -4
#define ERR_PNG_WRONGCOLORTYPE -5


struct BitmapFileData;
struct PNGFileData;

class ImageData{
  public:
    int32_t Width, Height;
    size_t PixelDataSize;
    bool isAlphaUsed = false;
    
    // this will be converted to 8-bit-rgb or 8-bit-rgba colordepth
    unsigned char *PixelData = NULL;

    ImageData();
    ImageData(const BitmapFileData &bmp);
    ImageData(const PNGFileData &png);
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

class ImageOpener{
  public:
    ImageOpener(){}
    virtual int Open(std::string filepath, uint16_t idField = 0){}
    virtual ImageData GetImageData(){}
};

class BmpOpener: public ImageOpener{
  private:
    BitmapFileData bmpdat{};

  public:
    BmpOpener();

    // return error code if cannot get the image
    // and will deleting the last image data used
    // if idField is 0, it will read the header
    int Open(std::string filepath, uint16_t idField = 0);
    static int Open(std::string filepath, BitmapFileData &bmpstruct, uint16_t idField = 0);
    static int Open(std::ifstream &ifs, BitmapFileData &bmpstruct, uint16_t idField = 0);

    ImageData GetImageData();
};

struct PNGFileData{
  public:
    uint32_t Width = 0, Height = 0;
    uint8_t BitDepth = 0, BitsPerChannel = 0, CompressionMethod = 0, FilterMethod = 0, ZLibFcheck = 0, InterlaceMethod = 0;

    bool Truecolor = false, Alpha = false, Palette = false;

    size_t CompressedBlockLen = 0;

    char *PaletteData = NULL;
    char *CompressedBlock = NULL;
};

class PNGOpener: public ImageOpener{
  private:
    PNGFileData pngdat{};

  public:
    PNGOpener();

    // same as BmpOpener
    int Open(std::string filepath, uint16_t idField = 0);
    static int Open(std::string filepath, PNGFileData &pngstruct, uint16_t idField = 0);
    static int Open(std::ifstream &ifs, PNGFileData &pngstruct, uint16_t idField = 0);

    ImageData GetImageData();
};

#endif