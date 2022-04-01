#include "AsciiArt.h"
#include "math.h"

#include "thread"
#include "chrono"

#define DefaultImageScaleY 1
#define DefaultImageScaleX 2

using namespace std;

#define iomoveup_r(n) printf("\x1b[%dF", n)

const char* AsciiRangeChar = "@#W$9876543210?!abc;:+=-,._                      ";
const int AsciiRangeCharLen = 50;

CONSOLE_SCREEN_BUFFER_INFO AsciiArt::GetTerminalData(){
  CONSOLE_SCREEN_BUFFER_INFO bufinfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufinfo);
  return bufinfo;
}

void AsciiArt::SetImage(ImageData &img){
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo = GetTerminalData();

  int newHeight = (int)img.Height*DefaultImageScaleY;
  int newWidth = (int)img.Width*DefaultImageScaleX;

  int termsize_x = consoleInfo.srWindow.Right-consoleInfo.srWindow.Left;
  int termsize_y = consoleInfo.srWindow.Bottom-consoleInfo.srWindow.Top-1;

  float ImageScale = 1;

  if(newHeight > termsize_y || newWidth > termsize_x)
    ImageScale = min((float)termsize_y/newHeight, (float)termsize_x/newWidth);

  newHeight = (int)newHeight*ImageScale;
  newWidth = (int)newWidth*ImageScale;

  ImageData newimg = img.ResizeImage(newWidth, newHeight);
  char *asciibuf = ProcessImage(newimg);

  for(int i = newHeight-1; i >= 0; i--){
    fwrite(asciibuf+(i*newWidth), sizeof(char), newWidth, stdout);
    fwrite("\n", sizeof(char), 1, stdout);
  }
}

// this will use the first ImageData as the information about the image
void AsciiArt::DrawImage(ImageData **img, int imgnum, float fps){
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo = GetTerminalData();

  int newHeight = (int)img[0]->Height*DefaultImageScaleY;
  int newWidth = (int)img[0]->Width*DefaultImageScaleX;

  int termsize_x = consoleInfo.srWindow.Right-consoleInfo.srWindow.Left;
  int termsize_y = consoleInfo.srWindow.Bottom-consoleInfo.srWindow.Top-1;

  float ImageScale = 1;

  if(newHeight > termsize_y || newWidth > termsize_x)
    ImageScale = min((float)termsize_y/newHeight, (float)termsize_x/newWidth);

  newHeight = (int)newHeight*ImageScale;
  newWidth = (int)newWidth*ImageScale;

  long DeltaDurationPerFrameus = (long)(1000000.0f/fps);
  auto startTime = chrono::high_resolution_clock::now();

  for(int i = 0; i < imgnum; i++){
    auto startFrameSec = chrono::high_resolution_clock::now();

    ImageData newimg = img[i]->ResizeImage(newWidth, newHeight);
    char *asciibuf = ProcessImage(newimg);

    for(int i = newHeight-1; i >= 0; i--){
      fwrite(asciibuf+(i*newWidth), sizeof(char), newWidth, stdout);
      fwrite("\n", sizeof(char), 1, stdout);
    }

    auto stopFrameSec = chrono::high_resolution_clock::now();
    auto currentTime = chrono::duration_cast<chrono::milliseconds>(stopFrameSec-startTime);

    float time = currentTime.count()/1000.f;
    
    printf("Time: %.3f, Frame %d of %d", time, i+1, imgnum);

    if(i < imgnum-1)
      iomoveup_r(newHeight);

    stopFrameSec = chrono::high_resolution_clock::now();
    auto CurrentDuration = chrono::duration_cast<chrono::microseconds>(stopFrameSec-startFrameSec);
    long delta = DeltaDurationPerFrameus-CurrentDuration.count();

    this_thread::sleep_for(chrono::microseconds(delta));
  }
}


char* AsciiArt::ProcessImage(ImageData &img){
  // plus one for the null ending
  int reslen = (img.Width*img.Height)+1;
  char *res = new char[reslen];

  int offset = 0;
  int res_offset = 0;
  while(offset < img.PixelDataSize && res_offset < reslen){
    // averaging rbg to get grayscal
    int ascii = (int)round((float)(
      img.PixelData[offset++] +
      img.PixelData[offset++] +
      img.PixelData[offset++]
    ) / 3);

    // then get the ascii range
    res[res_offset++] = AsciiRangeChar[AsciiRangeCharLen-(int)abs((float)ascii/0xff*AsciiRangeCharLen)];

    // adding alpha color (if used)
    img.isAlphaUsed? res[res_offset++] = img.PixelData[offset++]: 0;
  }

  res[reslen-1] = '\0';
  return res;
}