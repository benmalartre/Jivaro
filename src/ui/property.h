#ifndef AMN_WIDGETS_PROPERTY_H
#define AMN_WIDGETS_PROPERTY_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

class PropertyUI : public BaseUI
{
public:
  PropertyUI(View* parent, const std::string& name);
  ~PropertyUI()         override;

  void SetPrim(const pxr::UsdPrim& prim);
  pxr::UsdPrim& GetPrim() { return _prim; };

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

private:
  pxr::GfVec3f  _color;
  pxr::UsdPrim  _prim;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGETS_PROPERTY_H