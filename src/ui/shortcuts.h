#ifndef JVR_UI_SHORTCUTS_H
#define JVR_UI_SHORTCUTS_H

#include "../imgui/imgui.h"
#include "../command/block.h"

JVR_NAMESPACE_OPEN_SCOPE

// Boiler plate code for adding a shortcut, mainly to avoid writing the same code multiple time
// Only configurable at compile time, so it should change in the future.

template <typename... Args> 
inline bool KeyPressed(ImGuiKey key, Args... others) 
{
  return KeyPressed(key) && KeyPressed(others...);
}
template <> inline bool KeyPressed(ImGuiKey key) 
{ 
  return ImGui::IsKeyDown(key); 
}


template <typename CommandT, ImGuiKey... Keys, typename... Args> 
inline void AddShortcut(Args &&...args) 
{
  static bool KeyPressedOnce = false;
  if (KeyPressed(Keys...)) {
    if (!KeyPressedOnce) {
      UndoBlock block;
      CommandT(args...);
      KeyPressedOnce = true;
    }
  } else {
      KeyPressedOnce = false;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_SHORTCUTS_H