
#include "../common.h"
#include "../graph/input.h"
#include "../graph/output.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../graph/connectableAPI.h"
#include "../graph/stage.h"
#include "../ui/node.h"
#include "../ui/graph.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/vt/value.h>


JVR_NAMESPACE_OPEN_SCOPE

// main entry point
void
TestStageUI(
  GraphEditorUI* ui, 
  const std::vector<UsdStageRefPtr>& stages
);

void 
RecurseStagePrim(
  GraphEditorUI* ui, 
  NodeUI* parent,
  const UsdPrim& prim, 
  int stageIndex, 
  int& nodeIndex
);

void 
DrawStageUI(GraphEditorUI* ui);

JVR_NAMESPACE_CLOSE_SCOPE