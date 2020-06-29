#ifndef AMN_WIDGET_TOOLBAR_H
#define AMN_WIDGET_TOOLBAR_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include "../widgets/icon.h"
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

AMN_NAMESPACE_OPEN_SCOPE

// callback prototype
typedef void(*ToolbarPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);
struct ToolbarItem {
  BaseUI*                     ui;
  std::string                 label;
  std::string                 shortcut;
  bool                        selected;
  bool                        enabled;

  pxr::VtArray<pxr::VtValue>  args;
  ToolbarPressedFunc          func;

  ToolbarItem(BaseUI* ui, const std::string lbl, const std::string sht, bool sel,
    bool enb, ToolbarPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());

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
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGET_TOOLBAR_H