static pxr::UsdPrim TestUsdShadeAPI()
{
  UndoBlock editBlock;
  pxr::UsdStageRefPtr stage = GetApplication()->GetWorkStage();

  const pxr::SdfPath GRAPH_PATH("/graph");
  const pxr::TfToken GET("get");
  const pxr::TfToken MUL("multiply");
  const pxr::TfToken SET("set");

  pxr::UsdExecGraph graph = pxr::UsdExecGraph::Define(stage, GRAPH_PATH);
  
  pxr::UsdExecNode get = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(GET));
  pxr::UsdUINodeGraphNodeAPI api(get);
  api.CreatePosAttr().Set(pxr::GfVec2f(0, 0));
  pxr::UsdExecInput inPrim = get.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  inPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput inAttr = get.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  inAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecOutput inVal = get.CreateOutput(pxr::TfToken("Value"), pxr::SdfValueTypeNames->Vector3f);
  inVal.Set(pxr::VtValue(pxr::GfVec3f(0.f)));

  pxr::UsdExecNode mul = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(MUL));
  api = pxr::UsdUINodeGraphNodeAPI(mul);
  api.CreatePosAttr().Set(pxr::GfVec2f(120, 0));
  pxr::UsdExecInput factor = mul.CreateInput(pxr::TfToken("Factor"), pxr::SdfValueTypeNames->Float);
  factor.Set(pxr::VtValue(1.f));
  pxr::UsdExecInput input = mul.CreateInput(pxr::TfToken("Input"), pxr::SdfValueTypeNames->Vector3f);
  pxr::UsdExecOutput output = mul.CreateOutput(pxr::TfToken("Output"), pxr::SdfValueTypeNames->Vector3f);

  pxr::UsdExecNode set = pxr::UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(SET));
  api = pxr::UsdUINodeGraphNodeAPI(set);
  api.CreatePosAttr().Set(pxr::GfVec2f(240, 0));
  pxr::UsdExecInput outPrim = set.CreateInput(pxr::TfToken("Primitive"), pxr::SdfValueTypeNames->Asset);
  outPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput outAttr = set.CreateInput(pxr::TfToken("Attribute"), pxr::SdfValueTypeNames->Token);
  outAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  pxr::UsdExecInput outVal0 = set.CreateInput(pxr::TfToken("Value0"), pxr::SdfValueTypeNames->Vector3f);
  outVal0.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal1 = set.CreateInput(pxr::TfToken("Value1"), pxr::SdfValueTypeNames->Vector3f);
  outVal1.Set(pxr::VtValue(pxr::GfVec3f(0.f)));
  pxr::UsdExecInput outVal2 = set.CreateInput(pxr::TfToken("Value2"), pxr::SdfValueTypeNames->Vector3f);
  outVal2.Set(pxr::VtValue(pxr::GfVec3f(0.f)));


  input.ConnectToSource(inVal);
  outVal0.ConnectToSource                                                                             (output);

  stage->SetDefaultPrim(graph.GetPrim());

  return graph.GetPrim();
}