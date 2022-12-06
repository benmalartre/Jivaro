#ifndef JVR_UTILS_PREFS_H
#define JVR_UTILS_PREFS_H

#include "../common.h"
#include <stdio.h>
#include <string.h>

#include <pxr/base/js/json.h>
#include <pxr/base/js/value.h>
#include <pxr/base/js/utils.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>


JVR_NAMESPACE_OPEN_SCOPE

struct PreferencesItem {
  pxr::TfToken            _key;
  pxr::VtValue            _value;
};

class Preferences {
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