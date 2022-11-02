#include <iostream>
#include <string>
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
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/imaging/hd/renderPassState.h>
#include <pxr/imaging/LoFi/debugCodes.h>
#include <pxr/usd/usdAnimX/fileFormat.h>
#include <pxr/usd/usdAnimX/types.h>
#include <pxr/usd/usdAnimX/desc.h>
#include <pxr/usd/usdAnimX/data.h>  
#include <pxr/usd/usdAnimX/keyframe.h>

#include "../utils/files.h"
#include "../utils/prefs.h"
#include "../ui/fileBrowser.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../ui/graphEditor.h"
#include "../ui/timeline.h"
#include "../ui/demo.h"
#include "../ui/toolbar.h"
#include "../ui/explorer.h"
#include "../ui/layers.h"
#include "../ui/layerEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/curveEditor.h"
#include "../command/command.h"
#include "../command/delegate.h"
#include "../command/manager.h"
#include "../command/router.h"
#include "../command/block.h"
#include "../app/application.h"
#include "../app/modal.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"

JVR_NAMESPACE_OPEN_SCOPE

Application* APPLICATION = nullptr;
const char* Application::APPLICATION_NAME = "Jivaro";

// constructor
//----------------------------------------------------------------------------
Application::Application(unsigned width, unsigned height):
  _mainWindow(nullptr), _activeWindow(nullptr), 
  _viewport(nullptr), _execute(false)
{  
  _workspace = new Workspace();
  _mainWindow = CreateStandardWindow(width, height);
  _mainWindow->Init();
  _time.Init(1, 101, 24);
  
};

Application::Application(bool fullscreen):
  _mainWindow(nullptr), _activeWindow(nullptr), 
  _viewport(nullptr), _execute(false)
{
  _workspace = new Workspace();
  _mainWindow = CreateFullScreenWindow();
  _mainWindow->Init();
  _time.Init(1, 101, 24);
};

// destructor
//----------------------------------------------------------------------------
Application::~Application()
{
  if(_mainWindow) delete _mainWindow;
  if(_workspace) delete _workspace;
};

// create full screen window
//----------------------------------------------------------------------------
Window*
Application::CreateFullScreenWindow()
{
  Window* window = Window::CreateFullScreenWindow();
  return AddWindow(window);
}

// create child window
//----------------------------------------------------------------------------
Window*
Application::CreateChildWindow(
  int x, int y, int width, int height, Window* parent,
  const std::string& name, bool decorated)
{
  Window* window =
    Window::CreateChildWindow(x, y, width, height,
      parent->GetGlfwWindow(), name, decorated);
  return AddWindow(window);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Application::CreateStandardWindow(int width, int height)
{
  Window* window = Window::CreateStandardWindow(width, height);
  return AddWindow(window);
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

// browse for file
//----------------------------------------------------------------------------
std::string
Application::BrowseFile(int x, int y, const char* folder, const char* filters[], 
  const int numFilters, const char* name)
{
  std::string result = 
    "/Users/malartrebenjamin/Documents/RnD/Jivaro/assets/Kitchen_set 3/Kitchen_set.usd";
  
  ModalFileBrowser browser(x, y, "Open", ModalFileBrowser::Mode::OPEN);
  browser.Loop();
  if(browser.GetStatus() == BaseModal::Status::OK) {
    result = browser.GetResult();
  }
  browser.Term();  

  return result;
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
  pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::CreateAnonymous("shot.usda");
  pxr::SdfLayerRefPtr geomLayer = pxr::SdfLayer::CreateAnonymous("geom.usda");
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
  animLayer->Export("C:/Users/graph/Documents/bmal/src/Amnesie/assets/test.animx");
  curveEditor->SetLayer(pxr::SdfLayerHandle(animLayer));
  stage->SetEditTarget(geomLayer);

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


Mesh* MakeColoredPolygonSoup(pxr::UsdStageRefPtr& stage, 
  const pxr::TfToken& path)
{
  Mesh* mesh = new Mesh();
  //mesh->PolygonSoup(65535);
  pxr::GfMatrix4f space(1.f);
  mesh->TriangularGrid2D(10.f, 6.f, space, 0.2f);
  mesh->Randomize(0.05f);

  pxr::UsdGeomMesh polygonSoup = 
    pxr::UsdGeomMesh::Define(stage, pxr::SdfPath(path));
  polygonSoup.CreatePointsAttr(pxr::VtValue(mesh->GetPositions()));
  polygonSoup.CreateNormalsAttr(pxr::VtValue(mesh->GetNormals()));
  polygonSoup.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh->GetFaceConnects()));
  polygonSoup.CreateFaceVertexCountsAttr(pxr::VtValue(mesh->GetFaceCounts()));
  pxr::VtArray<pxr::GfVec3f> colors(1);

  polygonSoup.CreateDisplayColorAttr(pxr::VtValue(mesh->GetDisplayColor()));
  pxr::UsdGeomPrimvar displayColorPrimvar = polygonSoup.GetDisplayColorPrimvar();
  GeomInterpolation colorInterpolation = mesh->GetDisplayColorInterpolation();

  switch(colorInterpolation) {
    case GeomInterpolationConstant:
      displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->constant);
      break;
    case GeomInterpolationUniform:
      displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->uniform);
      break;
    case GeomInterpolationVertex:
      displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->vertex);
      break;
    case GeomInterpolationVarying:
      displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
      break;
    case GeomInterpolationFaceVarying:
      displayColorPrimvar.SetInterpolation(pxr::UsdGeomTokens->faceVarying);
      break;
    default:
      break;
  }

  polygonSoup.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));
  return mesh;
}

Mesh* MakeOpenVDBSphere(pxr::UsdStageRefPtr& stage, const pxr::TfToken& path)
{
  Mesh* mesh = new Mesh();
  /*
  mesh->OpenVDBSphere(6.66, pxr::GfVec3f(3.f, 7.f, 4.f));

  pxr::SdfPath path(path);
  pxr::UsdGeomMesh vdbSphere = pxr::UsdGeomMesh::Define(stage, path);
  vdbSphere.CreatePointsAttr(pxr::VtValue(mesh->GetPositions()));
  vdbSphere.CreateNormalsAttr(pxr::VtValue(mesh->GetNormals()));
  vdbSphere.CreateFaceVertexIndicesAttr(pxr::VtValue(mesh->GetFaceConnects()));
  vdbSphere.CreateFaceVertexCountsAttr(pxr::VtValue(mesh->GetFaceCounts()));

  vdbSphere.CreateSubdivisionSchemeAttr(pxr::VtValue(pxr::UsdGeomTokens->none));

  std::cout << "CREATED OPENVDB SPHERE !!!" << std::endl;
  */ 
 
  return mesh;
}

// init application
//----------------------------------------------------------------------------
void 
Application::Init()
{
  for(auto& tool: _tools)tool.second.Init();
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
    //"C:/Users/graph/Documents/bmal/src/USD_ASSETS/Kitchen_set/Kitchen_set.usd";
    //"E:/Projects/RnD/USD_BUILD/assets/Contour/JackTurbulized.usda";
    //"E:/Projects/RnD/USD/extras/usd/examples/usdGeomExamples/basisCurves.usda";
    //"E:/Projects/RnD/USD_BUILD/assets/maneki_anim.usd";
    "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usda";
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

    // setup notifications
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::SelectionChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::NewSceneCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::SceneChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::AttributeChangedCallback);
  pxr::TfNotice::Register(TfCreateWeakPtr(this), &Application::UndoStackNoticeCallback);

  // create window
  _mainWindow->SetGLContext();
  int width, height;
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &width, &height);
  View* mainView = _mainWindow->SplitView(
    _mainWindow->GetMainView(), 0.5, true, View::LFIXED, 22);
  View* bottomView = _mainWindow->SplitView(
    mainView->GetRight(), 0.9, true, false);
  //bottomView->Split(0.9, true, true);
  View* timelineView = bottomView->GetRight();
  timelineView->SetTabed(false);
  View* centralView = _mainWindow->SplitView(
    bottomView->GetLeft(), 0.6, true);
  View* middleView = centralView->GetLeft();
  View* topView = mainView->GetLeft();

  _mainWindow->SplitView(middleView, 0.9, false);
  
  View* workingView = _mainWindow->SplitView(
    middleView->GetLeft(), 0.15, false);
  View* propertyView = middleView->GetRight();
  View* leftTopView = _mainWindow->SplitView(
    workingView->GetLeft(), 0.1, false, View::LFIXED, 32);
  View* toolView = leftTopView->GetLeft();
  toolView->SetTabed(false);
  View* stageView = leftTopView->GetRight();
  _mainWindow->SplitView(stageView, 0.25, true);
  View* layersView = stageView->GetLeft();
  View* explorerView = stageView->GetRight();


  View* viewportView = workingView->GetRight();  
  View* graphView = centralView->GetRight();

  _mainWindow->Resize(width, height);
  
  
  GraphEditorUI* graph = new GraphEditorUI(graphView);
  
  //CurveEditorUI* editor = new CurveEditorUI(graphView);
  _viewport = new ViewportUI(viewportView);
  _timeline = new TimelineUI(timelineView);
  MenuUI* menu = new MenuUI(topView);
  ToolbarUI* verticalToolbar = new ToolbarUI(toolView, true);
  _explorer = new ExplorerUI(explorerView);
  _layers =  new LayersUI(layersView);
  //new LayerHierarchyUI(layersView, "fuck");
  //_property = new PropertyUI(propertyView, "Property");
  //new DemoUI(propertyView);
  
  std::cout << "PREFERENCES : " << GetPreferences().GetRootFolder() << std::endl;
  //_stage = TestAnimXFromFile(filename, editor);
  //pxr::UsdStageRefPtr stage = TestAnimX(editor);
  //_scene->GetRootStage()->GetRootLayer()->InsertSubLayerPath(stage->GetRootLayer()->GetIdentifier());

  /*
  // Create the layer to populate.
  std::string shotFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/test.usda";
  std::string animFilePath = "E:/Projects/RnD/USD_BUILD/assets/AnimX/anim.animx";
  //pxr::SdfLayerRefPtr baseLayer = pxr::SdfLayer::FindOrOpen(shotFilePath);
  
  // Create a UsdStage with that root layer.
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Create(shotFilePath);
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(100);
  
  pxr::UsdGeomCube cube =
    pxr::UsdGeomCube::Define(stage, pxr::SdfPath("/Cube"));
    

  stage->GetRootLayer()->Save();

  // we use Sdf, a lower level library, to obtain the 'anim' layer.
  pxr::SdfLayerRefPtr animLayer = pxr::SdfLayer::FindOrOpen(animFilePath);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;

  stage->SetEditTarget(animLayer);
  std::cout << "HAS LOCAL LAYER : " << stage->HasLocalLayer(animLayer) << std::endl;
  */
  
  /*
  // Create a mesh for the group.
        UsdGeomMesh mesh =
            UsdGeomMesh::Define(stage, SdfPath("/" + group.name));*/
  
  //_stage = pxr::UsdStage::CreateNew("test_stage");
  //_stage = pxr::UsdStage::Open(filename);

  //_stage = pxr::UsdStage::CreateNew("test.usda", pxr::TfNullPtr);
  //_stage = pxr::UsdStage::CreateInMemory();

  //_mesh = MakeColoredPolygonSoup(_scene->GetCurrentStage(), pxr::TfToken("/polygon_soup"));
  //Mesh* vdbMesh = MakeOpenVDBSphere(_stage, pxr::TfToken("/openvdb_sphere"));
/*
  for(size_t i=0; i< 12; ++i) {
    pxr::SdfPath path(pxr::TfToken("/cube_"+std::to_string(i)));
    pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(_stage, path);
    cube.AddTranslateOp().Set(pxr::GfVec3d(i * 3, 0, 0), pxr::UsdTimeCode::Default());
  }
*/
  //_stages.push_back(stage1);
  //TestStageUI(graph, _stages);

 
  _mainWindow->CollectLeaves();
 
  /*
  Window* childWindow = CreateChildWindow(200, 200, 400, 400, _mainWindow);
  childWindow->Init(this);

  _tools[childWindow] = Tool();
  _tools[childWindow].Init();
  
  _mainWindow->AddChild(childWindow);
  
  ViewportUI* viewport2 = new ViewportUI(childWindow->GetMainView());
  
  //DummyUI* dummy = new DummyUI(childWindow->GetMainView(), "Dummy");
  
  childWindow->CollectLeaves();
  */
 

}

void 
Application::Update()
{
  _time.ComputeFramerate(glfwGetTime());
  if(_time.IsPlaying()) {
    if(_time.PlayBack()) {
      if(_viewport)_viewport->GetEngine()->SetDirty(true);
    }
  }  
  if (_workspace->GetExecStage()) {
    _workspace->UpdateExec(GetTime().GetActiveTime());
  }
}

void 
Application::AddEngine(Engine* engine)
{
  _engines.push_back(engine);
}

void 
Application::RemoveEngine(Engine* engine)
{
  for (size_t i = 0; i < _engines.size(); ++i) {
    if (engine == _engines[i]) {
      _engines.erase(_engines.begin() + i);
      break;
    }
  }
}

Window*
Application::AddWindow(Window* window)
{
  std::cout << "ADD WINDOW  " << window << std::endl;
  _tools[window] = Tool();
  std::cout << "TOOL SIZE = " << _tools.size() << std::endl;
  std::cout << "TOOL : " << &_tools[window] << std::endl;
  return window;
}

void
Application::RemoveWindow(Window* window)
{
  WindowToolsMapIt toolIt;

  for (toolIt = _tools.begin(); toolIt != _tools.end(); ++toolIt)
  {
    if (toolIt->first == window) {
      _tools.erase(toolIt);
      break;
    }
  }
}

static void _DirtyAllEngines(std::vector<Engine*>& engines)
{
  for (auto& engine : engines) {
    engine->SetDirty(true);
  }
}

void 
Application::SetActiveViewport(ViewportUI* viewport) 
{
  if (_viewport) {
    _viewport->GetView()->ClearFlag(View::TIMEVARYING);
  }
  _viewport = viewport;
  _viewport->GetView()->SetFlag(View::TIMEVARYING);
}

Tool*
Application::GetTools(Window* window)
{
  for (auto& tool : _tools) {
    if (tool.first == window)return &tool.second;
  }

  return NULL;
}

void 
Application::SetActiveTool(short t)
{
  for (auto& tool : _tools) {
    glfwMakeContextCurrent(tool.first->GetGlfwWindow());
    tool.second.SetActiveTool(t);
  }
}

void 
Application::SelectionChangedCallback(const SelectionChangedNotice& n)
{
  for (auto& engine : _engines) {
    if (!_selection.IsEmpty() && _selection.IsObject()) {
      engine->SetSelected(_selection.GetSelectedPrims());
    } else {
      engine->ClearSelected();
    }
  }
  for(auto& tool: _tools) tool.second.ResetSelection();
  _DirtyAllEngines(_engines);
  GetMainWindow()->ForceRedraw();
}

void 
Application::NewSceneCallback(const NewSceneNotice& n)
{
  _selection.Clear();
  _manager.Clear();
  _DirtyAllEngines(_engines);
}

void 
Application::SceneChangedCallback(const SceneChangedNotice& n)
{
  for(auto& tool: _tools)tool.second.ResetSelection();
  GetMainWindow()->ForceRedraw();
  _DirtyAllEngines(_engines);
}

void
Application::AttributeChangedCallback(const AttributeChangedNotice& n)
{
  for (auto& tool : _tools)tool.second.ResetSelection();
  GetMainWindow()->ForceRedraw();
  _DirtyAllEngines(_engines);
}

void
Application::UndoStackNoticeCallback(const UndoStackNotice& n)
{
  AddCommand(std::shared_ptr<UsdGenericCommand>(new UsdGenericCommand()));
}


void 
Application::AddCommand(std::shared_ptr<Command> command)
{
  _manager.AddCommand(command);
  _manager.ExecuteCommands();
  GetMainWindow()->ForceRedraw();
}

void 
Application::Undo()
{
  std::cout << " APPLICATION UNDO !!" << std::endl;
  _manager.Undo();
}

void 
Application::Redo()
{
  _manager.Redo();
}

void 
Application::Delete()
{
  Selection* selection = GetSelection();
  const pxr::SdfPathVector& paths = selection->GetSelectedPrims();
  selection->Clear();
  AddCommand(std::shared_ptr<DeletePrimCommand>(
    new DeletePrimCommand(GetWorkStage(), paths)));
}

void
Application::Duplicate()
{
  Selection* selection = GetSelection();
  if (!selection->IsEmpty()) {
    const Selection::Item& item = selection->GetItem(0);
    AddCommand(std::shared_ptr<DuplicatePrimCommand>(
      new DuplicatePrimCommand(GetWorkStage(), item.path)));
  }
}

void 
Application::OpenScene(const std::string& filename)
{
  if(strlen(filename.c_str()) > 0) {    
    if (_workspace) delete _workspace;
    _workspace = new Workspace();
    _workspace->AddStageFromDisk(filename);
  }
}

void
Application::NewScene()
{
  if (_workspace) delete _workspace;
  _workspace = new Workspace();
}

void Application::SaveScene()
{
  GetWorkStage()->GetRootLayer()->Save(true);
}

void Application::SaveSceneAs(const std::string& filename)
{
  GetWorkStage()->GetRootLayer()->Save(true);
}

// execution
void 
Application::ToggleExec() 
{
  _execute = 1 - _execute; 
  if (_execute) {
    _workspace->AddExecStage();
    _DirtyAllEngines(_engines);
  }
  else {
    _workspace->RemoveExecStage();
    _DirtyAllEngines(_engines);
  }
};

void 
Application::SetExec(bool state) 
{ 
  _execute = state; 
};

bool 
Application::GetExec() 
{ 
  return _execute; 
};

// main loop
void 
Application::MainLoop()
{
  if (_mainWindow->IsIdle())return;
  _mainWindow->MainLoop();
}

// get stage for display
pxr::UsdStageRefPtr
Application::GetDisplayStage()
{
  return _workspace->GetDisplayStage();
}

// get stage for work
pxr::UsdStageRefPtr
Application::GetWorkStage()
{
  std::cout << "get works stage..." << std::endl;
  std::cout << "workspace : " << _workspace << std::endl;
  return _workspace->GetWorkStage();
}

// get current layer
pxr::SdfLayerRefPtr
Application::GetCurrentLayer()
{
  return _workspace->GetWorkLayer();
}

// selection
void 
Application::SetSelection(const pxr::SdfPathVector& selection)
{
  AddCommand(std::shared_ptr<SelectCommand>(
    new SelectCommand(Selection::PRIM, selection, SelectCommand::SET)));
}

void
Application::ToggleSelection(const pxr::SdfPathVector& selection)
{
  AddCommand(std::shared_ptr<SelectCommand>(
    new SelectCommand(Selection::PRIM, selection, SelectCommand::TOGGLE)));
}

void 
Application::AddToSelection(const pxr::SdfPathVector& paths)
{
  AddCommand(std::shared_ptr<SelectCommand>(
    new SelectCommand(Selection::PRIM, paths, SelectCommand::ADD)));
}

void 
Application::RemoveFromSelection(const pxr::SdfPathVector& paths)
{
  AddCommand(std::shared_ptr<SelectCommand>(
    new SelectCommand(Selection::PRIM, paths, SelectCommand::REMOVE)));
}

void 
Application::ClearSelection()
{
  AddCommand(std::shared_ptr<SelectCommand>(
    new SelectCommand(Selection::PRIM, {}, SelectCommand::SET)));
}

pxr::GfBBox3d
Application::GetStageBoundingBox()
{
  pxr::GfBBox3d bbox;
  pxr::TfTokenVector purposes = { pxr::UsdGeomTokens->default_ };
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_workspace->GetWorkStage()->GetPseudoRoot());
}

pxr::GfBBox3d 
Application::GetSelectionBoundingBox()
{
  pxr::GfBBox3d bbox;
  static pxr::TfTokenVector purposes = {
    pxr::UsdGeomTokens->default_,
    pxr::UsdGeomTokens->proxy,
    pxr::UsdGeomTokens->guide,
    pxr::UsdGeomTokens->render
  };
  pxr::UsdGeomBBoxCache bboxCache(
    pxr::UsdTimeCode(_time.GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const Selection::Item& item = _selection[n];
    if (item.type == Selection::Type::PRIM) {
      pxr::UsdPrim prim = _workspace->GetWorkStage()->GetPrimAtPath(item.path);
      
      if (prim.IsActive()) {
        const pxr::GfBBox3d primBBox = bboxCache.ComputeWorldBound(prim);
        bbox = bbox.Combine(bbox, pxr::GfBBox3d(primBBox.ComputeAlignedRange()));
      }
        
    }
    else if (item.type == Selection::Type::VERTEX) {

    }
    else if (item.type == Selection::Type::EDGE) {

    }
    else if (item.type == Selection::Type::FACE) {

    }
  }

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

JVR_NAMESPACE_CLOSE_SCOPE
