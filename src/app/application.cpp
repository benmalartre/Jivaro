#include "../utils/nfd/include/nfd.h"
#include "../utils/files.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../ui/graph.h"
#include "../ui/timeline.h"
#include "../ui/dummy.h"
#include "../ui/toolbar.h"
#include "../ui/explorer.h"
#include "../ui/property.h"
#include "../ui/curveEditor.h"

#include <pxr/base/tf/debug.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/LoFi/debugCodes.h>
#include <pxr/usd/usdAnimX/fileFormat.h>
#include <pxr/usd/usdAnimX/types.h>
#include <pxr/usd/usdAnimX/desc.h>
#include <pxr/usd/usdAnimX/data.h>  
#include <pxr/usd/usdAnimX/keyframe.h>
#include "../tests/stageGraph.h"
#include "../tests/stageUI.h"

#include "application.h"
#include "notice.h"
#include "engine.h"

#include <iostream>

AMN_NAMESPACE_OPEN_SCOPE

Application* AMN_APPLICATION = nullptr;
const char* Application::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
Application::Application(unsigned width, unsigned height):
  _mainWindow(NULL),_stage(nullptr)
{  
  _mainWindow = CreateStandardWindow(width, height);
  _mainWindow->Init(this);
  _time.Init(1, 101, 24);
};

Application::Application(bool fullscreen):
  _mainWindow(NULL),_stage(nullptr)
{
  _mainWindow = CreateFullScreenWindow();
  _mainWindow->Init(this);
  _time.Init(1, 101, 24);
};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  if(_mainWindow) delete _mainWindow;
};

// create full screen window
//----------------------------------------------------------------------------
Window*
Application::CreateFullScreenWindow()
{
  return Window::CreateFullScreenWindow();
}

// create child window
//----------------------------------------------------------------------------
Window*
Application::CreateChildWindow(int width, int height, Window* parent)
{
  return Window::CreateChildWindow(width, height, parent->GetGlfwWindow());
}

// create standard window
//----------------------------------------------------------------------------
Window*
Application::CreateStandardWindow(int width, int height)
{
  return Window::CreateStandardWindow(width, height);
}

void _RecurseSplitView(View* view, int depth, bool horizontal)
{
  if(depth < 3)
  {
    view->Split(0.5, horizontal, false);
    _RecurseSplitView(view->GetLeft(), depth + 1, horizontal);
    _RecurseSplitView(view->GetRight(), depth + 1, horizontal);
    view->SetPerc(0.5);
  }
}

static
pxr::UsdStageRefPtr
TestAnimXFromFile(const std::string& filename, CurveEditorUI* curveEditor)
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  pxr::SdfLayerHandleVector layers = stage->GetLayerStack();
  for (auto& layer : layers) {
    if (layer->GetFileFormat()->GetFormatId() == pxr::UsdAnimXFileFormatTokens->Id) {
      curveEditor->SetLayer(layer);
      break;
    }
  }
  return stage;
}

static
pxr::UsdStageRefPtr 
TestAnimX(CurveEditorUI* curveEditor)
{
  pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::CreateAnonymous("geom.usda");
  pxr::SdfLayerRefPtr geomLayer = pxr::SdfLayer::CreateAnonymous("rig.rig");
  pxr::SdfLayerRefPtr animLayer = pxr::SdfLayer::CreateAnonymous("anim.animx");

  rootLayer->InsertSubLayerPath(geomLayer->GetIdentifier());
  rootLayer->InsertSubLayerPath(animLayer->GetIdentifier());
     
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(rootLayer->GetIdentifier());
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(100);

  stage->SetEditTarget(geomLayer);
  std::cout << "SET METADATAS DONE..." << std::endl;

  pxr::SdfPath primPath("/Cube");
  pxr::UsdGeomCube cube =
    pxr::UsdGeomCube::Define(stage, primPath);

  stage->SetEditTarget(animLayer);
  //pxr::SdfFileFormatConstPtr fileFormat = animLayer->GetFileFormat();

  pxr::UsdAnimXFileFormatConstPtr fileFormat = 
    pxr::TfStatic_cast<pxr::UsdAnimXFileFormatConstPtr>(animLayer->GetFileFormat());
  pxr::SdfAbstractDataConstPtr datas = fileFormat->GetData(&(*animLayer));
  pxr::UsdAnimXDataPtr animXDatas = 
    pxr::TfDynamic_cast<pxr::UsdAnimXDataPtr>(
      pxr::TfConst_cast<pxr::SdfAbstractDataPtr>(datas));
  std::cout << "[ANIMX] DATAS : " << animXDatas->GetCurrentCount() << std::endl;

    
  std::cout << "[ANIMX] FILE FORMAT : " << fileFormat->GetFormatId() << std::endl;
  
  animXDatas->AddPrim(primPath);
  pxr::UsdAnimXOpDesc opDesc;
  opDesc.defaultValue = pxr::VtValue((double)1.0);
  opDesc.target = pxr::TfToken("size");
  opDesc.dataType = pxr::AnimXGetTokenFromSdfValueTypeName(opDesc.defaultValue.GetType());
  opDesc.name = pxr::TfToken("sizeOp");
  animXDatas->AddOp(primPath, opDesc);

  pxr::UsdAnimXCurveDesc curveDesc;
  curveDesc.name = pxr::TfToken("size");
  curveDesc.postInfinityType = pxr::UsdAnimXTokens->cycle;
  curveDesc.preInfinityType = pxr::UsdAnimXTokens->cycle;

  pxr::UsdAnimXKeyframeDesc keyframeDesc;
  keyframeDesc.time = 1;
  keyframeDesc.data[0] = 1.0;
  keyframeDesc.data[1] = (double)adsk::TangentType::Fixed;
  keyframeDesc.data[2] = 24.0;
  keyframeDesc.data[3] = 0.0;
  keyframeDesc.data[4] = (double)adsk::TangentType::Fixed;
  keyframeDesc.data[5] = 24.0;
  keyframeDesc.data[6] = 0.0;
  keyframeDesc.data[7] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 12;
  keyframeDesc.data[0] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 25;
  keyframeDesc.data[0] = 10.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 50;
  keyframeDesc.data[0] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  animXDatas->AddFCurve(primPath, opDesc.target, curveDesc);
  /*

  pxr::SdfPath propertyPath = pxr::SdfPath("/Cube").AppendProperty(pxr::TfToken("size"));
  animLayer->SetField(pxr::SdfPath("/Cube"), pxr::TfToken("size"), pxr::VtValue((double)32));
  pxr::UsdAnimXKeyframe keyframe;
  keyframe.time = 1;
  keyframe.value = 1;
  animLayer->SetTimeSample(
    propertyPath,
    keyframe.time, keyframe.GetAsSample());

  keyframe.time = 50;
  keyframe.value = 10;
  animLayer->SetTimeSample(
    propertyPath,
    keyframe.time, keyframe.GetAsSample());

  keyframe.time = 100;
  keyframe.value = 1;
  animLayer->SetTimeSample(
    propertyPath,
    keyframe.time, keyframe.GetAsSample());

  pxr::SdfLayerStateDelegateBasePtr animLayerStateDelegate =
    animLayer->GetStateDelegate();

  std::cout << "ANIMX LAYER STATE DELEGATE : " << animLayerStateDelegate << std::endl;
  */
  //pxr::SdfAbstractDataConstPtr datas = animLayer->_GetData();
  //animLayer->SetTimeSample()
  //stage->Reload();
  /*
  pxr::SdfAbstractDataConstPtr
    SdfFileFormat::_GetLayerData(const SdfLayer& layer)
  {
    return layer._GetData();
  }
    */
  //animLayer->Se
  //animLayer->
  curveEditor->SetLayer(pxr::SdfLayerHandle(animLayer));

  return stage;

  /*
  for (size_t i = 0; i < _filenames.size(); ++i)
  {
    std::string filename = _filenames[i];
    if (pxr::UsdStage::IsSupportedFile(filename) &&
      pxr::ArchGetFileLength(filename.c_str()) != -1)
    {
      pxr::SdfLayerRefPtr subLayer = SdfLayer::FindOrOpen(filename);
      _layers.push_back(subLayer);
      _rootLayer->InsertSubLayerPath(subLayer->GetIdentifier());
    }
  }
  

  //pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::FindOrOpen(_filename);
  //_stage = pxr::UsdStage::Open(rootLayer);
  _stage = pxr::UsdStage::Open(_rootLayer->GetIdentifier());
  _stage->SetEditTarget(_rootLayer);
  */
}

// init application
//----------------------------------------------------------------------------
void 
Application::Init()
{
  //pxr::TfErrorMark mark;
  
  // If no error messages were logged, return success.
  
  /*
  if (mark.IsClean()) {
    std::cout << "HYDRA SCENE DELEGATE OK" << std::endl;
  }
  else {
    for (auto& error : mark)std::cout << error.GetErrorCodeAsString() << std::endl;
    std::cout << "HYDRA SCENE DELEGATE FAILED" << std::endl;
  }
  */
 #ifdef _WIN32
  std::string filename =
    //"E:/Projects/RnD/USD_BUILD/assets/animX/test.usda";
    "E:/Projects/RnD/USD_BUILD/assets/animX/layered_anim.usda";
    //"E:/Projects/RnD/USD_BUILD/assets/Contour/JackTurbulized.usda";
    //"E:/Projects/RnD/USD/extras/usd/examples/usdGeomExamples/basisCurves.usda";
    //"E:/Projects/RnD/USD_BUILD/assets/maneki_anim.usd";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemal.usda";
    //"/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/Kitchen_set.usd";
    //"/Users/benmalartre/Documents/RnD/amnesie/usd/result.usda";
#else
  std::string filename = 
    "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
#endif

  // build test scene
  //pxr::TestScene(filename);
  // create imaging gl engine
  //TfDebug::Enable(HD_MDI);
  //TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  //TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  //TfDebug::Enable(HDST_DUMP_SHADER_SOURCEFILE);
  //pxr::TfDebug::Enable(pxr::HD_DIRTY_LIST);
  //pxr::TfDebug::Enable(pxr::HD_COLLECTION_CHANGED);
  //pxr::TfDebug::Enable(pxr::LOFI_REGISTRY);

  // create window
  _mainWindow->SetGLContext();
  int width, height;
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &width, &height);
  View* mainView = _mainWindow->SplitView(_mainWindow->GetMainView(), 0.5, true, View::LFIXED, 65);
  View* bottomView = _mainWindow->SplitView(mainView->GetRight(), 0.9, true, false);
  
  //bottomView->Split(0.9, true, true);
  View* timelineView = bottomView->GetRight();
  View* centralView = _mainWindow->SplitView(bottomView->GetLeft(), 0.6, true);
  View* middleView = centralView->GetLeft();
  View* topView = _mainWindow->SplitView(mainView->GetLeft(), 0.5, true, View::LFIXED, 20);

  _mainWindow->SplitView(middleView, 0.9, false);
  
  View* workingView = _mainWindow->SplitView(middleView->GetLeft(), 0.15, false);
  View* propertyView = middleView->GetRight();
  View* explorerView = workingView->GetLeft();
  View* viewportView = workingView->GetRight();  
  View* graphView = centralView->GetRight();
  _mainWindow->Resize(width, height);

  GraphUI* graph = new GraphUI(graphView, "Graph", true);
  //CurveEditorUI* curveEditor = new CurveEditorUI(graphView);
  
  _viewport = new ViewportUI(viewportView, OPENGL);  
  _timeline = new TimelineUI(timelineView);

  MenuUI* menu = new MenuUI(topView->GetLeft());
  ToolbarUI* toolbar = new ToolbarUI(topView->GetRight(), "Toolbar");
  _explorer = new ExplorerUI(explorerView);

  _property = new PropertyUI(propertyView, "Property");

  //_stage = TestAnimXFromFile(filename, curveEditor);
  //_stage = TestAnimX(curveEditor);

  /*
  // Create the layer to populate.
  std::string shotFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/test.usda";
  std::string animFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/anim.animx";
  //pxr::SdfLayerRefPtr baseLayer = pxr::SdfLayer::FindOrOpen(shotFilePath);
  
  // Create a UsdStage with that root layer.
  _stage = pxr::UsdStage::Open(shotFilePath);
  _stage->SetStartTimeCode(1);
  _stage->SetEndTimeCode(100);
  
  pxr::UsdGeomCube cube =
    pxr::UsdGeomCube::Define(_stage, pxr::SdfPath("/Cube"));
    

  _stage->GetRootLayer()->Save();

  // we use Sdf, a lower level library, to obtain the 'anim' layer.
  pxr::SdfLayerRefPtr animLayer = pxr::SdfLayer::FindOrOpen(animFilePath);
  std::cout << "HAS LOCAL LAYER : " << _stage->HasLocalLayer(animLayer) << std::endl;

  _stage->SetEditTarget(animLayer);
  std::cout << "HAS LOCAL LAYER : " << _stage->HasLocalLayer(animLayer) << std::endl;
  
  // Create a mesh for the group.
        UsdGeomMesh mesh =
            UsdGeomMesh::Define(stage, SdfPath("/" + group.name));
  */
  _stage = pxr::UsdStage::Open(filename);
  //_stages.push_back(stage1);
  //TestStageUI(graph, _stages);
 
  _mainWindow->CollectLeaves();
  
 /*
  Window* childWindow = CreateChildWindow(400, 400, _mainWindow);
  childWindow->Init(this);
  
  _mainWindow->AddChild(childWindow);
  
  ViewportUI* viewport2 = new ViewportUI(childWindow->GetMainView(), LOFI);
  viewport2->Init();
  
  DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");
  
  childWindow->CollectLeaves();
 */
}

void Application::Update()
{
  
}


void Application::OpenScene(const std::string& filename)
{

  nfdchar_t *outPath = NULL;
  nfdresult_t result = NFD_OpenDialog( "usd,usdc,usda,usdz", NULL, &outPath );
  if ( result == NFD_OKAY )
  {
    std::cout << "NEW FILE : " << outPath << std::endl;
    _stage = pxr::UsdStage::Open(outPath);
    OnNewScene();
    std::cout << "SET PROPERTY PRIM : " << _stage->GetDefaultPrim().GetName() << std::endl;
    _property->SetPrim(_stage->GetDefaultPrim());
    free(outPath);
  }
  else if ( result == NFD_CANCEL )
  {
      puts("User pressed cancel.");
  }
  else 
  {
      printf("Error: %s\n", NFD_GetError() );
  }
}

// main loop
void 
Application::MainLoop()
{
  _mainWindow->MainLoop();
  /*
  if (_stage) {
    pxr::SdfLayerRefPtr flattened = _stage->Flatten();
    if (flattened)
      flattened->Export("E:/Projects/RnD/USD_BUILD/assets/animX/flattened.usda");
  }
  */
}

// selection
void 
Application::SetSelection(const pxr::SdfPathVector& selection)
{
  _selection.Clear();
  for (auto& selected : selection) {
    _selection.AddItem(selected);
  }
}

void 
Application::AddToSelection(const pxr::SdfPath& path)
{
  _selection.AddItem(path);
}

void 
Application::RemoveFromSelection(const pxr::SdfPath& path)
{
  _selection.RemoveItem(path);
}

void 
Application::ClearSelection()
{
  _selection.Clear();
}

pxr::GfBBox3d
Application::GetStageBoundingBox()
{
  pxr::GfBBox3d bbox;
  pxr::TfTokenVector purposes = { pxr::UsdGeomTokens->default_ };
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
}

pxr::GfBBox3d 
Application::GetSelectionBoundingBox()
{
  pxr::GfBBox3d bbox;
  pxr::TfTokenVector purposes = {pxr::UsdGeomTokens->default_};
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const SelectionItem& item = _selection[n];
    if (item.type == SelectionType::OBJECT) {
      pxr::UsdPrim prim = _stage->GetPrimAtPath(item.path);
      if (prim.IsActive() && !prim.IsInMaster())
        bbox = bbox.Combine(bbox, bboxCache.ComputeWorldBound(prim));
    }
    else if (item.type == SelectionType::COMPONENT) {

    }
  }

  std::cout << "BBOX : " << bbox.GetRange() << std::endl;
  /*
  pxr::UsdPrim& prim = _stage->GetPrimAtPath(pxr::SdfPath("/Cube"));
  if (!prim.IsValid()) {
    prim = pxr::UsdGeomCube::Define(_stage, pxr::SdfPath("/Cube")).GetPrim();
    pxr::UsdGeomCube cube(prim);
    cube.CreateSizeAttr().Set(pxr::VtValue(1.0));
   
    pxr::VtArray<pxr::TfToken> xformOpOrderTokens =
    { pxr::TfToken("xformOp:scale"), pxr::TfToken("xformOp:translate")};
    cube.CreateXformOpOrderAttr().Set(pxr::VtValue(xformOpOrderTokens));
   
  }
  pxr::UsdGeomCube cube(prim);

  bool resetXformStack = false;
  bool foundScaleOp = false;
  bool foundTranslateOp = false;
  std::vector<pxr::UsdGeomXformOp> xformOps = cube.GetOrderedXformOps(&resetXformStack);
  for (auto& xformOp : xformOps) {
 
    if (xformOp.GetName() == pxr::TfToken("xformOp:scale")) {
      pxr::GfRange3d bboxRange = bbox.GetRange();
      xformOp.Set(pxr::VtValue(pxr::GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
      foundScaleOp = true;
    } else if(xformOp.GetName() == pxr::TfToken("xformOp:translate")) {
      pxr::GfVec3d center = bbox.ComputeCentroid();
      xformOp.Set(pxr::VtValue(center));
      foundTranslateOp = true;
    }
  }
  if (!foundScaleOp) {
    pxr::UsdGeomXformOp scaleOp = cube.AddScaleOp();
    pxr::GfRange3d bboxRange = bbox.GetRange();
    scaleOp.Set(pxr::VtValue(pxr::GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
  }
  if (!foundTranslateOp) {
    pxr::UsdGeomXformOp translateOp = cube.AddTranslateOp();
    pxr::GfVec3d center = bbox.ComputeCentroid();
    translateOp.Set(pxr::VtValue(center));
  }
  */
  return bbox;
}

AMN_NAMESPACE_CLOSE_SCOPE
