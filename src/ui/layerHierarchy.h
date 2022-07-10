#ifndef JVR_UI_LAYER_HIERARCHY_H
#define JVR_UI_LAYER_HIERARCHY_H

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/head.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>

JVR_NAMESPACE_OPEN_SCOPE

class LayerHierarchyUI : public HeadedUI
{
public:
  LayerHierarchyUI(View* parent);
  ~LayerHierarchyUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

private:
  pxr::SdfLayerRefPtr    _layer;
  pxr::SdfPrimSpecHandle _prim;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_LAYER_HIERARCHY_H