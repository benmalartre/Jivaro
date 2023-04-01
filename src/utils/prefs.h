#ifndef JVR_UTILS_PREFS_H
#define JVR_UTILS_PREFS_H

#include "../common.h"

#include <pxr/base/vt/dictionary.h>


JVR_NAMESPACE_OPEN_SCOPE

class Preferences {
  using Catergorie = pxr::VtDictionary;

public:
  Preferences();
  const std::string& GetRootFolder();

private:
  std::string       _root;
};

static Preferences& GetPreferences()
{
  static Preferences prefs;
  return prefs;
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_PREFS_H