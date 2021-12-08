#ifndef AMN_UI_PROPERTY_H
#define AMN_UI_PROPERTY_H

#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


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
  pxr::GfVec3f            _color;
  pxr::UsdPrim            _prim;
  static ImGuiWindowFlags _flags;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_PROPERTY_H