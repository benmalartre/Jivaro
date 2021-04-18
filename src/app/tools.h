#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "../ui/ui.h"
#include "../ui/viewport.h"
#include "../app/handle.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

enum TOOLS
{
  AMN_TOOL_NONE,
  AMN_TOOL_SELECT,
  AMN_TOOL_TRANSLATE,
  AMN_TOOL_ROTATE,
  AMN_TOOL_SCALE,
  AMN_TOOL_BRUSH,
  AMN_TOOL_OPEN,
  AMN_TOOL_SAVE
};

class ViewportUI;
class Tool {
public:
  Tool();
  ~Tool();

  void Init();
  void SetViewport(ViewportUI* viewport);
  void SetActiveTool(short tool);
  void ResetSelection();

  void Draw();
  void Select(bool lock);
  void Pick();
  void BeginUpdate();
  void Update();
  void EndUpdate();

private:
  TranslateHandle _translate;
  RotateHandle _rotate;
  ScaleHandle _scale;
  //BrushHandle* _brush;
  bool _interacting;
  ViewportUI* _viewport;

  BaseHandle* _active;
};

void AMNInitializeTools();

AMN_NAMESPACE_CLOSE_SCOPE
