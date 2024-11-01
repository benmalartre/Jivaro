#ifndef JVR_APPLICATION_LAYOUT_H
#define JVR_APPLICATION_LAYOUT_H
#include <map>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Window;

enum WindowLayout {
  WINDOW_LAYOUT_BASE,
  WINDOW_LAYOUT_RAW,
  WINDOW_LAYOUT_STANDARD,
  WINDOW_LAYOUT_RANDOM
};

class Layout {
public:
  static void Layout::BaseLayout(Window* window);
  static void Layout::RandomLayout(Window* window);
  static void Layout::StandardLayout(Window* window);
  static void Layout::RawLayout(Window* window);
};

JVR_NAMESPACE_CLOSE_SCOPE


#endif //JVR_APPLICATION_LAYOUT_H