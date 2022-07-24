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

class MenuUI : public BaseUI
{
public:
  // callback prototype
  typedef void(*PressedFunc)(const pxr::VtArray<pxr::VtValue>& args);

  struct Item {
    MenuUI*                     ui;
    Item*                       parent;
    std::string                 label;
    std::string                 shortcut;
    bool                        selected;
    bool                        enabled;

    std::vector<Item>           items;
    pxr::VtArray<pxr::VtValue>  args;
    PressedFunc                 func;

    Item(MenuUI* ui, const std::string lbl, const std::string sht, bool sel,
      bool enb, PressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());
    Item& AddItem(MenuUI* ui, const std::string lbl, const std::string sht, bool sel,
      bool enb, PressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());
    Item(Item* parent, const std::string lbl, const std::string sht, bool sel,
      bool enb, PressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());
    Item& AddItem(Item* parent, const std::string lbl, const std::string sht, bool sel,
      bool enb, PressedFunc f = NULL, const pxr::VtArray<pxr::VtValue>& a = pxr::VtArray<pxr::VtValue>());

    bool Draw();
    pxr::GfVec2i ComputeSize();
    pxr::GfVec2i ComputePos();

  };

public:
  MenuUI(View* parent);
  ~MenuUI();

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void DirtyViewsUnderBox();

  Item& AddItem(const std::string label, const std::string shortcut, bool selected,
    bool enabled, PressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());


private:
  std::vector<Item>       _items;
  Item*                   _current;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H