//--------------------------------------------------------------------------------
// FILES UTILS
//--------------------------------------------------------------------------------
#pragma once

#include "../common.h"
#include <dirent.h>
#include "strings.h"
#include <fstream>
#include <pxr/base/arch/fileSystem.h>
#include <pxr/base/arch/systemInfo.h>

AMN_NAMESPACE_OPEN_SCOPE

#ifdef _WIN32
	#define SEPARATOR "\\"
  #include <windows.h>
  typedef int mode_t;
#else
	#define SEPARATOR "/"
#endif

// directory exists
//-----------------------------------------------------
bool DirectoryExists(std::string path);

// create directory
//-----------------------------------------------------
bool CreateDirectory(const std::string& path);

// get file size
//-----------------------------------------------------
int GetFileSize(const std::string& filePath);

// get file name
//-----------------------------------------------------
std::string GetFileName(const std::string& filePath);

// num files in directory
//-----------------------------------------------------
int NumFilesInDirectory(const char* path);

// get files in directory
//-----------------------------------------------------
int GetFilesInDirectory(const char* path, std::vector<std::string>& filenames);

// get installatio folder
//-----------------------------------------------------
std::string GetInstallationFolder();

//=====================================================
// File class
//=====================================================
// mode
//-----------------------------------------------------
enum FILE_MODE
{
    FILE_READ,
    FILE_WRITE
};
// header
//-----------------------------------------------------
#define SIZE_HEADER 66
#define SIZE_BUFFER 256

// class
//-----------------------------------------------------
class File
{
private:
  std::string path;
  std::fstream* file;
  std::string content;
  char* buffer;
    
public:
  File(){};
  File(const std::string& path);

  bool Open(FILE_MODE mode);
  bool Close();
  uint64_t GetFileLength();
  
  void SetPath(const std::string& in_path){path = in_path;};
  
  void Write(const std::string& s);
  std::string Read();
  std::string ReadAll();

  void _CreatePath(
    const std::string& directory, 
    const std::string& in_name, 
    const std::string& extension
  );
};

AMN_NAMESPACE_CLOSE_SCOPE