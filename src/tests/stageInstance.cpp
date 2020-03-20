#include "stageInstance.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdGeomXform _AddInstanceableExternalReferenceUnderXform(
  UsdStageRefPtr stage,
  const SdfPath& path,
  const std::string& name,
  const std::string& filename
)
{
  UsdGeomXform referenceGrp = 
    UsdGeomXform::Define(stage, path.AppendChild(TfToken(name+"_grp")));

  SdfPath refGroupPath = referenceGrp.GetPath();
  UsdPrim reference = 
    stage->OverridePrim(refGroupPath.AppendChild(TfToken(name)));

  reference.GetReferences().AddReference(filename);
  return referenceGrp;
}

void TestScene(const std::string& result)
{
  /*UsdStageRefPtr stage =
    UsdStage::CreateNew("/Users/benmalartre/Documents/RnD/amnesie/usd/test.usda");*/
  UsdStageRefPtr stage = UsdStage::CreateInMemory();

  //UsdUINodeGraphNodeAPI stageNodeApi(stage);

  std::string shotName = "SHA257";
  SdfPath graphPath("/"+shotName);

  GraphGraph graph = GraphGraph::Define(stage, graphPath);
  
  /*
  UsdPrim refManeki = graphPath.AppendChild(TfToken("manekineko"));
  refSphere = refStage.OverridePrim('/refSphere')
print refStage.GetRootLayer().ExportToString()

  UsdGeomSphere sphere = 
    UsdGeomSphere::Define(stage, graphPath.AppendChild(TfToken("Sphere")));

  UsdAttribute radiusAttr = sphere.CreateRadiusAttr();
  UsdAttribute colorAttr = sphere.CreateDisplayColorAttr();
  radiusAttr.Set(32.0);
  
  VtArray<GfVec3f> colors;
  colors.push_back(GfVec3f(1.f,0.5f,0.f));
  colorAttr.Set(colors);
  */


  GraphStage stageNode = 
    GraphStage::Define(stage, graphPath.AppendChild(TfToken("stage")));
  UsdAttribute fileNameAttr = stageNode.CreateFileNameAttr();
  fileNameAttr.Set("/Users/benmalartre/Documents/RnD/amnesie/usd/instance.usda");
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

  UsdGeomXform maneki1Grp = 
    _AddInstanceableExternalReferenceUnderXform(
      stage,
      stagePath,
      "manekineko1",
      /*"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd"*/
      "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd"
    );

  UsdGeomXformOp translateManeki1 = maneki1Grp.AddTranslateOp();
  translateManeki1.Set(GfVec3d(-8, 0, 0));
  
  UsdGeomXform maneki2Grp = 
    _AddExternalReferenceUnderXform(
      stage,
      stagePath,
      "manekineko2",
      "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd"
    );
  
  UsdGeomXformOp translateManeki2 = maneki2Grp.AddTranslateOp();
  translateManeki2.Set(GfVec3d(8, 0, 0));

  GraphNode node1 = 
    _TestAddNode(stage, stagePath.AppendChild(TfToken("node1")));

  GraphInput input = 
  _TestAddInput(node1, 
                "input"+std::to_string(1), 
                GraphAttributeType::Parameter, 
                SdfValueTypeNames->Float);

  input = 
  _TestAddInput(node1, 
                "input"+std::to_string(2), 
                GraphAttributeType::Parameter, 
                SdfValueTypeNames->Bool);

  input = 
  _TestAddInput(node1, 
                "input"+std::to_string(3), 
                GraphAttributeType::Parameter, 
                SdfValueTypeNames->String);

  input = 
  _TestAddInput(node1, 
                "input"+std::to_string(4), 
                GraphAttributeType::Parameter, 
                SdfValueTypeNames->Int);

  input = 
  _TestAddInput(node1, 
                "input"+std::to_string(5), 
                GraphAttributeType::Parameter, 
                SdfValueTypeNames->Float3);

  input = 
  _TestAddInput(node1, 
                "input"+std::to_string(6), 
                GraphAttributeType::Input, 
                SdfValueTypeNames->TimeCode);

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