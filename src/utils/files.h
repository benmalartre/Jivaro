//--------------------------------------------------------------------------------
// FILES UTILS
//--------------------------------------------------------------------------------
#pragma once

#include "../default.h"
#include "strings.h"
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

// DIRECTORY EXISTS
//---------------------------------------------------------------------------------------
static bool DirectoryExists(std::string path)
{
    struct stat info;
    
    if( stat( path.c_str(), &info ) != 0 ) return false;
    else if( info.st_mode & S_IFDIR ) return true;
    else return false;
}

// CREATE DIRECTORY
//---------------------------------------------------------------------------------------
static bool CreateDirectory(const std::string& path)
{
    if(DirectoryExists(path)) return true;
    
    std::vector<std::string> splitted = SplitString(path, SEPARATOR);
    std::string currentPath = "";
    if(StartsWithString(path, SEPARATOR))currentPath = SEPARATOR;
    mode_t nMode = 0733; // UNIX style permissions
    int nError = 0;
    for(unsigned int i=0;i<splitted.size();i++)
    {
        currentPath += splitted[i];
        if(!DirectoryExists(currentPath))
        {
#ifdef _WIN32
			nError = CreateDirectoryA(currentPath.c_str(), NULL);
#else
            nError = mkdir(currentPath.c_str(),nMode);
#endif
            if (nError != 0) return false;
        }
        currentPath += SEPARATOR;
    }
    return true;
}

// GET FILE SIZE
//---------------------------------------------------------------------------------------
static int GetFileSize(const std::string& filePath)
{

    struct stat results;
    
    if (stat((const char*)filePath.c_str(), &results) == 0)
    return results.st_size;
    else
    return -1;
}

// NUM FILES IN DIRECTORY
//---------------------------------------------------------------------------------------
static int NumFilesInDirectory(const char* path)
{
  DIR *dir;
  struct dirent *ent;
  int num_files = 0;
  if ((dir = opendir (path)) != NULL) 
  {
    // print all the files and directories within directory
    while ((ent = readdir (dir)) != NULL) 
    {
      if(
        ! strncmp(ent->d_name, ".", 1) ||
        ! strncmp(ent->d_name, "..", 2) ||
        ! strncmp(ent->d_name, ".DS_Store", 9)
      ) continue;
      num_files++;
    }
    closedir (dir);
    return num_files;
  } 
  else 
  {
    // could not open directory
    std::cerr << "Could Not Open Directory : " << path << std::endl;
    return EXIT_FAILURE;
  }
}

// GET INSTALLATION FOLDER
//---------------------------------------------------------------------------------------
static std::string GetInstallationFolder()
{
  std::string exePath = pxr::ArchGetExecutablePath();
  std::vector<std::string> splitted = SplitString(exePath, SEPARATOR);
  splitted.pop_back();
  return JoinString(splitted, SEPARATOR);
  
}

AMN_NAMESPACE_CLOSE_SCOPE