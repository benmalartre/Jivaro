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

class Camera;
class Tool {
public:
  Tool();
  ~Tool();

  void Init();
  void SetActiveTool(short tool);
  void SetViewport(const pxr::GfVec4f& viewport);
  void SetCamera(Camera* camera);
  void ResetSelection();

  bool IsActive();
  bool IsInteracting();

  void Draw();
  void Select(float x, float y, float width, float height, bool lock);
  void Pick(float x, float y, float width, float height);
  void BeginUpdate(float x, float y, float width, float height);
  void Update(float x, float y, float width, float height);
  void EndUpdate(float x, float y, float width, float height);

private:
  pxr::GfVec4f    _viewport;
  /*
  TranslateHandle _translate;
  RotateHandle    _rotate;
  ScaleHandle     _scale;
  BrushHandle     _brush;
  */
  bool            _interacting;
  BaseHandle*     _active;
  short           _activeAxis;
  short           _hoveredAxis;
};

void InitializeTools();

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_HANDLE_H