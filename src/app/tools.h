#ifndef JVR_APPLICATION_TOOLS_H
#define JVR_APPLICATION_TOOLS_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../ui/viewport.h"
#include "../app/handle.h"
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_OPEN_SCOPE

enum TOOLS
{
  TOOL_NONE,
  TOOL_SELECT,
  TOOL_TRANSLATE,
  TOOL_ROTATE,
  TOOL_SCALE,
  TOOL_BRUSH
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

  bool IsActive();
  bool IsInteracting();

  void Draw();
  void Select(bool lock);
  void Pick();
  void BeginUpdate();
  void Update();
  void EndUpdate();

private:
  TranslateHandle _translate;
  RotateHandle    _rotate;
  ScaleHandle     _scale;
  BrushHandle     _brush;
  bool            _interacting;
  ViewportUI*     _viewport;
  BaseHandle*     _active;
  short           _activeAxis;
  short           _hoveredAxis;
};

void InitializeTools();

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_HANDLE_H