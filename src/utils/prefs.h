#ifndef JVR_UTILS_PREFS_H
#define JVR_UTILS_PREFS_H

#include "../common.h"

#include <pxr/base/vt/dictionary.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>


JVR_NAMESPACE_OPEN_SCOPE

class Preferences {
public:
  Preferences();
  Preferences(Preferences const&) = delete;
  Preferences(Preferences&&) = delete;

  static Preferences& Get() {
    static Preferences prefs;
    return prefs;
  }

private:
  VtDictionary       _data;
  std::string             _root;
};

static Preferences& GetPreferences()
{
  static Preferences prefs;
  return prefs;
}


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_PREFS_H