#pragma once

#include "../default.h"
#include "../ui.h"
#include "../utils.h"
#include <pxr/usd/usd/prim.h>

namespace AMN {
  class DummyUI : public UI
  {
  public:
    DummyUI(View* parent, const std::string& name);
    ~DummyUI()         override;
    void OnKeyboard()  override;
    void OnMouseMove() override;
    void OnClick()     override;
    void OnEnter()     override;
    void OnLeave()     override;
    void OnDraw()      override;

    void FillBackground();
  private:
    pxr::GfVec3f _color;

  };

} // namespace AMN
