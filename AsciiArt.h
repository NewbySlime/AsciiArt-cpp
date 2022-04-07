#ifndef ASCIIART_HEADER
#define ASCIIART_HEADER

#include "img.h"

#ifdef _WIN32
  #include "windows.h"
#endif

class AsciiArt{
  private:
    #ifdef _WIN32
      static CONSOLE_SCREEN_BUFFER_INFO GetTerminalData();
    #endif
    // returned values are literal char based on ascii density
    // if the image has alpha, it will be returned with the alpha "ca" (char - alpha)
    static char* ProcessImage(ImageData &img);

  public:
    static void SetImage(ImageData &img);
    static void DrawImage(ImageData **img, int imgnum, float fps);
    static void SafeStopDrawing();

};

#endif
