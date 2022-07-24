#include "../utils/prefs.h"
#include "../utils/files.h"
#include <iostream>

JVR_NAMESPACE_OPEN_SCOPE

Preferences::Preferences()
{
  _root = GetInstallationFolder() + SEPARATOR + "prefs";
  if (!DirectoryExists(_root))
  {
    CreateDirectory(_root);
  }
}
const std::string& Preferences::GetRootFolder()
{
  return _root;
}

JVR_NAMESPACE_CLOSE_SCOPE
