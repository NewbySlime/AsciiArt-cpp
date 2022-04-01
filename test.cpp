#include "img.h"
#include "AsciiArt.h"

#include "conio.h"
#include "filesystem"
#include "sstream"

// minimum filename length (for frames)
#define MIN_NUM_LENGTH 4

int main(){
  std::cout << "Choose what to convert:\n1. Pictures\n2. Frames (pictures from video) (with filename as number starting with 0000 (as like blender rendered pictures))" << std::endl;
  std::cout << "\nFiles extensions currently supported: " << SupportedFileTypeStr << std::endl;

  int option = _getch();

  switch(option){
    break; case '1':{
      std::string datapath;
      std::cout << "Enter path to file: " << std::flush;
      std::cin >> datapath;
      BmpOpener bmp;
      int errcode = bmp.Open(datapath);

      if(errcode != ERR_BMP_SUCCESS){
        std::cout << "Error opening file: ";
        switch(errcode){
          break; case ERR_BMP_WRONGID:
            std::cout << "Not a bmp file." << std::endl;
          
          break; default:
            std::cout << "errcode: " << errcode << std::endl;
        }

        return errcode;
      }

      ImageData imgdat = bmp.GetImageData();
      AsciiArt::SetImage(imgdat);
    }


    // maybe add some extension checking
    break; case '2':{
      std::string filepath;
      std::cout << "Enter path to first frame picture: " << std::flush;
      std::cin >> filepath;

      float framerate = 0;
      std::cout << "Enter framerate (can be float): " << std::flush;
      std::cin >> framerate;

      std::size_t extensionOffset = filepath.find_last_of('.');
      std::size_t nameOffset = filepath.find_last_of('/');

      std::string extensionName = filepath.substr(extensionOffset+1);
      std::string folderPath = filepath.substr(0, nameOffset);
      std::string firstFileName = filepath.substr(nameOffset+1, extensionOffset);
      
      std::size_t imgdataslen = 0;
      ImageData **imgdatas = (ImageData**)malloc(0);
      BitmapFileData currentBitmap;

      size_t framecount = 0;
      std::stringstream strnumi(firstFileName);
      strnumi >> framecount;

      while(true){
        std::stringstream ss("");
        ss << framecount;
        
        std::string newFilename = ss.str();
        if(newFilename.length() < MIN_NUM_LENGTH){
          std::string filler(MIN_NUM_LENGTH-newFilename.length(), '0');
          newFilename = filler+newFilename;
        }

        std::ifstream ifs{};
        std::string newfilename = folderPath+"/"+newFilename+"."+extensionName;
        std::cout << "Loading " << newfilename << std::endl;

        ifs.open(newfilename);

        if(ifs.fail())
          break;

        int errcode = BmpOpener::Open(ifs, currentBitmap);
        if(errcode != 0){
          std::cout << "Failed opening a file, errcode: " << errcode << std::endl;
        }else{
          imgdatas = (ImageData**)realloc(imgdatas, sizeof(ImageData*)*(++imgdataslen));
          ImageData *newImage = new ImageData{currentBitmap};
          imgdatas[imgdataslen-1] = newImage;
        }
        
        framecount++;
      }

      AsciiArt::DrawImage(imgdatas, imgdataslen, framerate);
    }

    break; default:
      std::cout << "Wrong option." << std::endl;
  }
}