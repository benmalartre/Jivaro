//--------------------------------------------------------------------------------
// FILES UTILS
//--------------------------------------------------------------------------------
#ifndef JVR_UTILS_FILES_H
#define JVR_UTILS_FILES_H

#include "../common.h"

#include "strings.h"
#include <stdint.h>
#include <fstream>
#include <pxr/base/arch/fileSystem.h>
#include <pxr/base/arch/systemInfo.h>

JVR_NAMESPACE_OPEN_SCOPE

#ifdef _WIN32
  #define SEPARATOR "\\"
  #include <windows.h>
  typedef int mode_t;
  #include "dirent.h"
  #include <tchar.h>
#else
  #define SEPARATOR "/"
  #include <dirent.h>
#endif

// entry info
//-----------------------------------------------------
struct EntryInfo
{
  enum Type {
    FOLDER,
    FILE
  };

  EntryInfo(const std::string& path, Type type, bool isHidden) 
    : path(path)
    , isHidden(isHidden)
    , type(type)
  {
  }
  std::string path;
  bool isHidden;
  Type type;
};

// file exists
//-----------------------------------------------------
bool FileExists(const std::string& path);

// directory exists
//-----------------------------------------------------
bool DirectoryExists(const std::string& path);

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
size_t NumFilesInDirectory(const char* path);

// get entries in directory
//-----------------------------------------------------
size_t GetVolumes(std::vector<EntryInfo>& entries);

// get entries in directory
//-----------------------------------------------------
size_t GetEntriesInDirectory(const char* path, std::vector<EntryInfo>& entries,
  bool ignoreCurrent=true, bool ignoreParent=false);

// get files in directory
//-----------------------------------------------------
size_t GetFilesInDirectory(const char* path, std::vector<std::string>& filenames);

// get installation folder
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
    const std::string& name, 
    const std::string& extension
  );
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_FILES_H