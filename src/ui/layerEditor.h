#ifndef JVR_UI_LAYER_EDITOR_H
#define JVR_UI_LAYER_EDITOR_H

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/ui.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>

JVR_NAMESPACE_OPEN_SCOPE

class LayerEditorUI : public BaseUI
{
public:
  LayerEditorUI(View* parent);
  ~LayerEditorUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;

private:
  SdfLayerRefPtr           _layer;
  SdfPrimSpecHandle        _prim;
  static ImGuiWindowFlags       _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_LAYER_EDITOR_H