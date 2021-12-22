#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/ui.h"
#include "../ui/utils.h"



PXR_NAMESPACE_OPEN_SCOPE

class Command;
// callback prototype
typedef void(*MenuPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);

struct MenuItem {
  View* view;
  std::string                 label;
  std::string                 shortcut;
  bool                        selected;
  bool                        enabled;

  std::vector<MenuItem>       items;
  pxr::VtArray<pxr::VtValue>  args;
  MenuPressedFunc             func;

  MenuItem(View* view, const std::string lbl, const std::string sht, bool sel,
    bool enb, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());
  MenuItem& AddItem(View* view, const std::string lbl, const std::string sht, bool sel,
    bool enb, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());

  bool Draw();
};

class MenuUI : public BaseUI
{
public:
  MenuUI(View* parent);
  ~MenuUI();

  // overrides
  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  bool Draw() override;

  MenuItem& AddItem(View* view, const std::string label, const std::string shortcut, bool selected,
    bool enabled, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());
private:
  std::vector<MenuItem>   _items;
  MenuItem* _current;
  static ImGuiWindowFlags _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H