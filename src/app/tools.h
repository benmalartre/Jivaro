#pragma once

#include "../default.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

enum TOOLS
{
  AMN_TOOL_NONE,
  AMN_TOOL_CAMERA,
  AMN_TOOL_TRANSLATE,
  AMN_TOOL_ROTATE,
  AMN_TOOL_SCALE,
  AMN_TOOL_DRAG
};

AMN_NAMESPACE_CLOSE_SCOPE
