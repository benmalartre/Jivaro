#include "stageGraph.h"

PXR_NAMESPACE_OPEN_SCOPE

// create simple graph
//----------------------------------------------------------------------------
GraphNode* CreateGraphNode()
{
  GraphNode* node = new GraphNode();
  return node;
}

GraphGraph* CreateGraphGraph()
{
  GraphGraph* graph = new GraphGraph();

  return graph;
}

// add node to graph test
//----------------------------------------------------------------------------
GraphNode _TestAddNode(UsdStageRefPtr stage, const SdfPath& path)
{
  GraphNode node = GraphNode::Define(stage, path);
  return node;
}

// create input test
//----------------------------------------------------------------------------
GraphInput _TestAddInput(GraphNode& node, 
                              const std::string& name, 
                              const GraphAttributeType& type,
                              const SdfValueTypeName& valueType)
{
  if(type == GraphAttributeType::Parameter)
  {
    GraphInput input = node.CreateInput(TfToken(name), valueType);
    return input;
  }
  return GraphInput();
}

// create output test
//----------------------------------------------------------------------------
GraphOutput _TestAddOutput(GraphNode& node, 
                                const std::string& name, 
                                const GraphAttributeType& type,
                                const SdfValueTypeName& valueType)
{
  if(type == GraphAttributeType::Parameter)
  {
    GraphOutput output = node.CreateOutput(TfToken(name), valueType);
    return output;
  }
  return GraphOutput();
}

GraphNode _CreateNodeAtPosition(UsdPrim prim, const GfVec2f& pos)
{
  UsdUINodeGraphNodeAPI api(prim);
  UsdAttribute attr  = api.CreatePosAttr();
  attr.Set(pos);
} 

GraphInput _AddInputPrim(GraphNode& node, UsdPrim& prim)
{
  UsdUINodeGraphNodeAPI api(prim);
  GraphInput input = _TestAddInput(node, 
                                        "Primitive", 
                                        GraphAttributeType::Input,
                                        SdfValueTypeNames->String);
  return input;
} 

void TestScene(const std::string& result)
{
  /*UsdStageRefPtr stage =
    UsdStage::CreateNew("/Users/benmalartre/Documents/RnD/amnesie/usd/test.usda");*/
  UsdStageRefPtr stage = UsdStage::CreateInMemory();

  //UsdUINodeGraphNodeAPI stageNodeApi(stage);

  std::string shotName = "SHA256";
  SdfPath graphPath("/"+shotName);

  GraphGraph graph = GraphGraph::Define(stage, graphPath);

  UsdGeomSphere sphere = 
    UsdGeomSphere::Define(stage, graphPath.AppendChild(TfToken("Sphere")));

  UsdAttribute radiusAttr = sphere.CreateRadiusAttr();
  UsdAttribute colorAttr = sphere.CreateDisplayColorAttr();
  radiusAttr.Set(32.0);
  
  VtArray<GfVec3f> colors;
  colors.push_back(GfVec3f(1.f,0.5f,0.f));
  colorAttr.Set(colors);

  GraphStage stageNode = 
    GraphStage::Define(stage, graphPath.AppendChild(TfToken("stage")));
  UsdAttribute fileNameAttr = stageNode.CreateFileNameAttr();
  fileNameAttr.Set("/Users/benmalartre/Documents/RnD/amnesie/usd/test.usda");
  UsdAttribute lifetimeAttr = stageNode.CreateLifetimeManagementAttr();
  lifetimeAttr.Set(GraphTokens->onDisk);  
  //stageNode.fileName = "/Users/benmalartre/Documents/RnD/amnesie/usd/test.usda";

  std::string fileName;
  fileNameAttr.Get(&fileName);
  std::cout << fileName.c_str() << std::endl;

  TfToken lifetime;
  lifetimeAttr.Get(&lifetime);
  std::cout << lifetime.GetText() << std::endl;

  SdfPath stagePath = stageNode.GetPath();
  GraphNode node1 = 
    _TestAddNode(stage, stagePath.AppendChild(TfToken("node1")));
  GraphInput input1 = 
    _TestAddInput(node1, "input1", 
      GraphAttributeType::Parameter, SdfValueTypeNames->Float);

  GraphOutput output1 = 
    _TestAddOutput(node1, "output1", 
      GraphAttributeType::Parameter, SdfValueTypeNames->Float);

  GraphNode node2 = 
    _TestAddNode(stage, stagePath.AppendChild(TfToken("node2")));
  GraphInput input2 = 
    _TestAddInput(node2, "input2", 
      GraphAttributeType::Parameter, SdfValueTypeNames->Float);

  GraphOutput output2 = 
    _TestAddOutput(node2, "output2", 
      GraphAttributeType::Parameter, SdfValueTypeNames->Float);

  input2.ConnectToSource(output1);

  UsdUINodeGraphNodeAPI nodeUI1(node1);
  UsdAttribute posAttr1 = nodeUI1.CreatePosAttr();
  posAttr1.Set(GfVec2f(120,60));
  
  UsdUINodeGraphNodeAPI nodeUI2(node2);
  UsdAttribute posAttr2 = nodeUI2.CreatePosAttr();
  posAttr2.Set(GfVec2f(240,60));

  stage->Export(result.c_str());
  /*
  stage->Save(); 
  std::cout << 
    "SAVED STAGE at /Users/benmalartre/Documents/RnD/amnesie/usd/test.usda" << 
      std::endl;*/
}

PXR_NAMESPACE_CLOSE_SCOPE