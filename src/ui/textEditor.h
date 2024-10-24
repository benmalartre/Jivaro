#ifndef JVR_UI_TEXT_EDITOR_H
#define JVR_UI_TEXT_EDITOR_H

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/ui.h"
#include <pxr/usd/sdf/layer.h>

JVR_NAMESPACE_OPEN_SCOPE

class TextEditorUI : public BaseUI
{
public:
  TextEditorUI(View* parent);
  ~TextEditorUI()  override;

  bool Draw()      override;

private:
  SdfLayerRefPtr           _layer;
  static ImGuiWindowFlags       _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TEXT_EDITOR_H