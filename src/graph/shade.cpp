static UsdPrim TestUsdShadeAPI()
{
  UndoBlock editBlock;
  UsdStageRefPtr stage = Application::Get()->GetWorkStage();

  const SdfPath GRAPH_PATH("/graph");
  const TfToken GET("get");
  const TfToken MUL("multiply");
  const TfToken SET("set");

  UsdExecGraph graph = UsdExecGraph::Define(stage, GRAPH_PATH);
  
  UsdExecNode get = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(GET));
  UsdUINodeGraphNodeAPI api(get);
  api.CreatePosAttr().Set(GfVec2f(0, 0));
  UsdExecInput inPrim = get.CreateInput(TfToken("Primitive"), SdfValueTypeNames->Asset);
  inPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  UsdExecInput inAttr = get.CreateInput(TfToken("Attribute"), SdfValueTypeNames->Token);
  inAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  UsdExecOutput inVal = get.CreateOutput(TfToken("Value"), SdfValueTypeNames->Vector3f);
  inVal.Set(VtValue(GfVec3f(0.f)));

  UsdExecNode mul = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(MUL));
  api = UsdUINodeGraphNodeAPI(mul);
  api.CreatePosAttr().Set(GfVec2f(120, 0));
  UsdExecInput factor = mul.CreateInput(TfToken("Factor"), SdfValueTypeNames->Float);
  factor.Set(VtValue(1.f));
  UsdExecInput input = mul.CreateInput(TfToken("Input"), SdfValueTypeNames->Vector3f);
  UsdExecOutput output = mul.CreateOutput(TfToken("Output"), SdfValueTypeNames->Vector3f);

  UsdExecNode set = UsdExecNode::Define(stage, GRAPH_PATH.AppendChild(SET));
  api = UsdUINodeGraphNodeAPI(set);
  api.CreatePosAttr().Set(GfVec2f(240, 0));
  UsdExecInput outPrim = set.CreateInput(TfToken("Primitive"), SdfValueTypeNames->Asset);
  outPrim.SetConnectability(UsdShadeTokens->interfaceOnly);
  UsdExecInput outAttr = set.CreateInput(TfToken("Attribute"), SdfValueTypeNames->Token);
  outAttr.SetConnectability(UsdShadeTokens->interfaceOnly);
  UsdExecInput outVal0 = set.CreateInput(TfToken("Value0"), SdfValueTypeNames->Vector3f);
  outVal0.Set(VtValue(GfVec3f(0.f)));
  UsdExecInput outVal1 = set.CreateInput(TfToken("Value1"), SdfValueTypeNames->Vector3f);
  outVal1.Set(VtValue(GfVec3f(0.f)));
  UsdExecInput outVal2 = set.CreateInput(TfToken("Value2"), SdfValueTypeNames->Vector3f);
  outVal2.Set(VtValue(GfVec3f(0.f)));


  input.ConnectToSource(inVal);
  outVal0.ConnectToSource                                                                             (output);

  stage->SetDefaultPrim(graph.GetPrim());

  return graph.GetPrim();
}