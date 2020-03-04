
#include "../default.h"
#include "../graph/input.h"
#include "../graph/output.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../graph/connectableAPI.h"
#include "../graph/stage.h"
#include "../widgets/node.h"
#include "../widgets/graph.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/vt/value.h>


AMN_NAMESPACE_OPEN_SCOPE

// main entry point
void
TestStageUI(AmnGraphUI* ui, const std::vector<pxr::UsdStageRefPtr>& stages);

void 
RecurseStagePrim(AmnGraphUI* ui, const pxr::UsdPrim& prim, int stageIndex);


AMN_NAMESPACE_CLOSE_SCOPE