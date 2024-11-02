#include "../utils/files.h"

JVR_NAMESPACE_OPEN_SCOPE

bool FileExists(const std::string& name){
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

bool DirectoryExists(const std::string& path)
{
  struct stat info;
  
  if( stat( path.c_str(), &info ) != 0 ) return false;
  else if( info.st_mode & S_IFDIR ) return true;
  else return false;
}

bool CreateDirectory(const std::string& path)
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
      nError = CreateDirectoryA((LPCSTR )currentPath.c_str(), (LPSECURITY_ATTRIBUTES) NULL);
#else
      nError = mkdir(currentPath.c_str(),nMode);
#endif
      if (nError != 0) return false;
    }
    currentPath += SEPARATOR;
  }
  return true;
}

int GetFileSize(const std::string& filePath)
{
  struct stat results;
  
  if (stat((const char*)filePath.c_str(), &results) == 0)
    return results.st_size;
  else
    return -1;
}

std::string GetFileName(const std::string& filePath)
{
  return SplitString(filePath, SEPARATOR).back();
}

size_t NumFilesInDirectory(const char* path)
{
  /*
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
  */

  return 0;
}

static bool _IsHiddenFile(const char* filename)
{
  if(strcmp(filename, "..") == 0) return false;
  return (filename[0] == '.' || filename[strlen(filename)-1] == '~');
}

size_t GetVolumes(std::vector<EntryInfo>& entries)
{
#ifdef _WIN32
  entries.clear();
  TCHAR szDrive[] = _T(" A:");
  DWORD uDriveMask = GetLogicalDrives();

  if (uDriveMask != 0)
  {
    while (uDriveMask)
    {
      // Use the bitwise AND, 1�"available, 0-not available
      if (uDriveMask & 1) {
        entries.push_back({
          (std::string)(const char*)szDrive,
          EntryInfo::Type::FOLDER,
          false
          });
      }
      // increment, check next drive
      ++szDrive[1];
      // shift the bitmask binary right
      uDriveMask >>= 1;
    }
  }
#else
  GetEntriesInDirectory(SEPARATOR, entries);
#endif
  return entries.size();
}

size_t GetEntriesInDirectory(const char* path, std::vector<EntryInfo>& entries, 
  bool ignoreCurrent, bool ignoreParent)
{
  entries.clear();
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (path)) != NULL) 
  {
    // print all the files and directories within directory
    while ((ent = readdir (dir)) != NULL) 
    {
      if(ent->d_type == DT_REG) {
        entries.push_back({
          (std::string)ent->d_name,
          EntryInfo::Type::FILE,
          _IsHiddenFile(ent->d_name)
        });
      } else if (ent->d_type == DT_DIR) {
        if (ignoreCurrent && strcmp(ent->d_name, ".") == 0) continue;
        if (ignoreParent && strcmp(ent->d_name, "..") == 0) continue;
        entries.push_back({
         (std::string)ent->d_name,
         EntryInfo::Type::FOLDER,
         _IsHiddenFile(ent->d_name)
         });
      }
    }
    closedir (dir);
    return entries.size();
  } 
  return 0;
}

size_t GetFilesInDirectory(const char* path, std::vector<std::string>& filenames)
{
  filenames.clear();
#ifdef _WIN32
  std::string search_path = std::string(path) + "/*.*";
  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        !strncmp(fd.cFileName, ".DS_Store", 9)) continue;
      filenames.push_back(fd.cFileName);
    } while (::FindNextFile(hFind, &fd));
    ::FindClose(hFind);
  }
  return 0;
#else
  DIR *dir;
  struct dirent *ent;
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
      filenames.push_back((std::string)ent->d_name);
    }
    closedir (dir);
    return filenames.size();
  } 
  else 
  {
    // could not open directory
    std::cerr << "Could Not Open Directory : " << path << std::endl;
    return EXIT_FAILURE;
  }
  
  return 0;
#endif
}

std::string GetInstallationFolder()
{
  std::string exePath = ArchGetExecutablePath();
  std::vector<std::string> tokens = SplitString(exePath, SEPARATOR);
  tokens.pop_back();
  return JoinString(tokens, SEPARATOR);
}
    

File::File(const std::string& fileName)
{
  if(FileExists(fileName))
    path = fileName;
}

void File::_CreatePath(const std::string& directory, 
  const std::string& name, const std::string& extension)
{
  if(EndsWithString(name, extension)) path = directory+SEPARATOR+name;
  else path = directory+SEPARATOR+name+extension;
}

bool File::Open(FILE_MODE mode)
{
  file = new std::fstream();
  if(mode == FILE_WRITE)
    file->open(path.c_str(), std::ios::out | std::ios::trunc);
  else if(mode == FILE_READ)
    file->open(path.c_str(), std::ios::in);
  if(file->is_open()) return true;
  else return false;
}
    
bool File::Close()
{
  if(file->is_open()) file->close();
  return true;
}
    
void File::Write(const std::string& s)
{
  *file << s << "\n";
}

uint64_t File::GetFileLength()
{
  if(!file->good()) return 0;
  file->seekg(0,std::ios::end);
  unsigned long len = file->tellg();
  file->seekg(std::ios::beg);
  
  return len;
}

std::string File::Read()
{
  if (file->is_open())
  {
    file->seekg(0);
    std::string line;
    while ( getline (*file,line) )
    {
      content+=line+"\n";
    }
    file->close();
  }
  return content;
}
    
std::string File::ReadAll()
{
  file->seekg(0, std::ios::end);
  content.reserve(file->tellg());
  file->seekg(0, std::ios::beg);
  
  content.assign((std::istreambuf_iterator<char>(*file)),
                  std::istreambuf_iterator<char>());
  return content;
}

JVR_NAMESPACE_CLOSE_SCOPE

