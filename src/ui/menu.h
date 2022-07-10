#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/ui.h"
#include "../ui/utils.h"



JVR_NAMESPACE_OPEN_SCOPE

class Command;
class MenuUI;
// callback prototype
typedef void(*MenuPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);

struct MenuItem {
  MenuUI*                     ui;
  std::string                 label;
  std::string                 shortcut;
  bool                        selected;
  bool                        enabled;

  std::vector<MenuItem>       items;
  pxr::VtArray<pxr::VtValue>  args;
  MenuPressedFunc             func;

  MenuItem(MenuUI* ui, const std::string lbl, const std::string sht, bool sel,
    bool enb, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());
  MenuItem& AddItem(MenuUI* ui, const std::string lbl, const std::string sht, bool sel,
    bool enb, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());

  void Draw();
  pxr::GfVec2i GetSize();
  pxr::GfVec2i GetPos();
  
};

class MenuUI : public BaseUI
{
public:
  MenuUI(View* parent);
  ~MenuUI();

  bool Draw() override;

  MenuItem& AddItem(const std::string label, const std::string shortcut, bool selected,
    bool enabled, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());

  friend MenuItem;

private:
  std::vector<MenuItem>   _items;
  MenuItem*               _current;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H