#ifndef AMN_WIDGET_TOOLBAR_H
#define AMN_WIDGET_TOOLBAR_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "../ui/ui.h"
#include "../ui/icon.h"
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

AMN_NAMESPACE_OPEN_SCOPE

// callback prototype
//typedef void(*ToolbarPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);
struct ToolbarItem {
  BaseUI*                     ui;
  short                       tool;
  std::string                 label;
  std::string                 shortcut;
  bool                        toggable;
  bool                        enabled;

  pxr::VtArray<pxr::VtValue>  args;
  IconPressedFunc             func;
  Icon*                       icon;

  ToolbarItem(BaseUI* ui, short tool, const std::string lbl, 
    const std::string sht, Icon* icon, bool sel, bool enb, 
    IconPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());

  bool Draw();
};

class ToolbarUI : BaseUI
{
public:
  ToolbarUI(View* parent, const std::string& name);
  ~ToolbarUI() override;

  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  bool Draw() override;

private:
  pxr::GfVec3f              _color;
  std::vector<ToolbarItem>  _items;
  ToolbarItem*              _current;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGET_TOOLBAR_H