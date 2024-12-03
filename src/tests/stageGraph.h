
#include "../common.h"
#include "../graph/input.h"
#include "../graph/output.h"
#include "../graph/graph.h"
#include "../graph/connectableAPI.h"
#include "../graph/stage.h"
#include "../ui/node.h"
#include "../ui/graph.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/vt/value.h>


JVR_NAMESPACE_OPEN_SCOPE

// add node to graph test
GraphNode _TestAddNode(UsdStageRefPtr stage, const SdfPath& path);

// create input test
GraphInput _TestAddInput(GraphNode& node, 
                              const std::string& name, 
                              const GraphAttributeType& type,
                              const SdfValueTypeName& valueType);

// create output test
GraphOutput _TestAddOutput(GraphNode& node, 
                                const std::string& name, 
                                const GraphAttributeType& type,
                                const SdfValueTypeName& valueType);

// create simple graph
GraphNode _CreateNodeAtPosition(UsdPrim prim, const GfVec2i& pos);

// main entry point
void TestScene(const std::string& result);

JVR_NAMESPACE_CLOSE_SCOPE