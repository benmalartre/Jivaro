#ifndef JVR_UI_ATTRIBUTES_H
#define JVR_UI_ATTRIBUTES_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>

JVR_NAMESPACE_OPEN_SCOPE

class AttributeEditorUI : public BaseUI
{
public:
  AttributeEditorUI(View* parent);
  ~AttributeEditorUI()         override;
  bool Draw()      override;

protected:
  pxr::SdfPath _GetPrimToDisplay();
  void _AppendDisplayColorAttr(pxr::SdfPath primPath);
  void _AppendDataSourceAttrs(pxr::HdContainerDataSourceHandle containerDataSource);
  void _AppendAllPrimAttrs(pxr::SdfPath primPath);

private:
  static ImGuiWindowFlags _flags;
  SdfPath                 _path;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_DEBUG_H