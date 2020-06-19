#ifndef AMN_WIDGETS_BROWSER_H
#define AMN_WIDGETS_BROWSER_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/ui.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

class BrowserUI : public BaseUI
{
public:
  BrowserUI(View* parent, const std::string& name);
  ~BrowserUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

  void FillBackground();
private:
  pxr::GfVec3f _color;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGETS_BROWSER_H