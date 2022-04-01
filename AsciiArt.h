#ifndef ASCIIART_HEADER
#define ASCIIART_HEADER

#include "img.h"
#include "windows.h"

class AsciiArt{
  private:
    static CONSOLE_SCREEN_BUFFER_INFO GetTerminalData();
    // returned values are literal char based on ascii density
    // if the image has alpha, it will be returned with the alpha "ca" (char - alpha)
    static char* ProcessImage(ImageData &img);
    

  public:
    static void SetImage(ImageData &img);
    static void DrawImage(ImageData **img, int imgnum, float fps);

};

#endif