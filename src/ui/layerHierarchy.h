#ifndef JVR_UI_LAYER_HIERARCHY_H
#define JVR_UI_LAYER_HIERARCHY_H

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/head.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>

PXR_NAMESPACE_OPEN_SCOPE

class LayerHierarchyUI : public HeadedUI
{
public:
  LayerHierarchyUI(View* parent, const std::string& name);
  ~LayerHierarchyUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

private:
  SdfLayerRefPtr    _layer;
  SdfPrimSpecHandle _prim;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_LAYER_HIERARCHY_H