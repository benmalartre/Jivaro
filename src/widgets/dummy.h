#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_OPEN_SCOPE

class DummyUI : public AmnUI
{
public:
  DummyUI(AmnView* parent, const std::string& name);
  ~DummyUI()         override;

  void Event()     override;
  void Draw()      override;

  void FillBackground();
private:
  pxr::GfVec3f _color;

};

PXR_NAMESPACE_CLOSE_SCOPE