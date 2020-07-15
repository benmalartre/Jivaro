#ifndef AMN_UI_DUMMY_H
#define AMN_UI_DUMMY_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "ui.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

class DummyUI : public BaseUI
{
public:
  DummyUI(View* parent, const std::string& name);
  ~DummyUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

  void FillBackground();
  void Demo();
private:
  pxr::GfVec3f _color;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_DUMMY_H