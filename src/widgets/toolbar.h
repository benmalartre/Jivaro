#ifndef AMN_WIDGET_TOOLBAR_H
#define AMN_WIDGET_TOOLBAR_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/ui.h"
#include "../utils/utils.h"
#include "../widgets/icon.h"
#include <vector>

AMN_NAMESPACE_OPEN_SCOPE

class ToolbarUI : BaseUI
{
public:
  ToolbarUI(View* parent, const std::string& name);
  ~ToolbarUI()         override;

  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  bool Draw()      override;

private:
  pxr::GfVec3f              _color;
  std::vector<IconButtonUI> _icons;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGET_TOOLBAR_H