#include <iostream>
#include <string>

#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/popup.h"
#include "../command/manager.h"
#include "../geometry/scene.h"
#include "../app/model.h"
#include "../app/notice.h"
#include "../app/handle.h"
#include "../app/engine.h"
#include "../app/selection.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/time.h"
#include "../app/commands.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/application.h"

#include "../tests/grid.h"
#include "../tests/raycast.h"
#include "../tests/particles.h"
#include "../tests/pbd.h"
#include "../tests/hair.h"
#include "../tests/bvh.h"
#include "../tests/points.h"
#include "../tests/instancer.h"
#include "../tests/velocity.h"
#include "../tests/pendulum.h"
#include "../tests/geodesic.h"
#include "../tests/push.h"


JVR_NAMESPACE_OPEN_SCOPE


// constructor
//----------------------------------------------------------------------------
Model::Model()
  : _execute(false)
  , _activeEngine(nullptr)
  , _exec(nullptr)
{  
  // scene indices
  _sceneIndexBases = HdMergingSceneIndex::New();
  _finalSceneIndex = HdMergingSceneIndex::New();
  _editableSceneIndex = _sceneIndexBases;
  SetEditableSceneIndex(_editableSceneIndex);

  UsdImagingCreateSceneIndicesInfo info;
  info.displayUnloadedPrimsWithBounds = true;
  const UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);
  _stageSceneIndex = sceneIndices.stageSceneIndex;
  AddSceneIndexBase(_stageSceneIndex);
};

// destructor
//----------------------------------------------------------------------------
Model::~Model()
{
};

bool
Model::_IsAnyEngineDirty()
{
  for (auto& engine : _engines) {
    if (engine->IsDirty())return true;
  }
  return false;
}

void
Model::SetStage(UsdStageRefPtr& stage)
{
  _stageCache.Insert(stage);
  _stage = stage;
  _layer = stage->GetRootLayer();
    
  _stageSceneIndex->SetStage(_stage);
  _stageSceneIndex->SetTime(UsdTimeCode::Default());

}


void 
Model::_SetEmptyStage()
{
  _stage = UsdStage::CreateInMemory();
  UsdGeomSetStageUpAxis(_stage, UsdGeomTokens->y);

  _rootLayer = _stage->GetRootLayer();
  _sessionLayer = _stage->GetSessionLayer();

  _stage->SetEditTarget(_sessionLayer);
  _stageSceneIndex->SetStage(_stage);
  _stageSceneIndex->SetTime(UsdTimeCode::Default());
}

void 
Model::_LoadUsdStage(const std::string usdFilePath)
{
  _rootLayer = SdfLayer::FindOrOpen(usdFilePath);
  _sessionLayer = SdfLayer::CreateAnonymous();
  _stage = UsdStage::Open(_rootLayer, _sessionLayer);
  _stage->SetEditTarget(_sessionLayer);
  _stageSceneIndex->SetStage(_stage);
  _stageSceneIndex->SetTime(UsdTimeCode::Default());
}


// init application
//----------------------------------------------------------------------------
void 
Model::Init()
{
  // scene indices
  _sceneIndexBases = HdMergingSceneIndex::New();
  _finalSceneIndex = HdMergingSceneIndex::New();
  _editableSceneIndex = _sceneIndexBases;
  SetEditableSceneIndex(_editableSceneIndex);

  UsdImagingCreateSceneIndicesInfo info;
  info.displayUnloadedPrimsWithBounds = true;
  const UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);
  _stageSceneIndex = sceneIndices.stageSceneIndex;
  AddSceneIndexBase(_stageSceneIndex);
  
}

void
Model::Update(const float time)
{
  if(_stageSceneIndex) {
    _stageSceneIndex->ApplyPendingUpdates();
    _stageSceneIndex->SetTime(time);
  }
}



void 
Model::InitExec()
{
  //_exec = new TestPendulum();
  //_exec = new TestVelocity();
  //_exec = new TestPoints();
  //_exec = new TestGrid();
  //_exec = new TestParticles();
  //_exec = new TestInstancer();
  //_exec = new TestRaycast();
  _exec = new TestPBD();
  //_exec = new TestPush();
  //_exec = new TestHair();
  //_exec = new TestGeodesic();
  //_exec = new TestBVH();
  _exec->InitExec(_stage);

  for(auto& engine: _engines) {
    engine->InitExec(_exec->GetScene());
  }
  
}

void
Model::UpdateExec(float time)
{
  _exec->UpdateExec(_stage, time);

  for (auto& engine : _engines) {
    engine->UpdateExec(time);
  }
  
}

void
Model::TerminateExec()
{
  for (auto& engine : _engines) {
    engine->TerminateExec();
  }
  _exec->TerminateExec(_stage);
  delete _exec;
  _exec = NULL;  
  _execute = false;
  NewSceneNotice().Send();
}

void 
Model::SendExecViewEvent(const ViewEventData *data)
{
  _exec->ViewEvent(data);
}




// ---------------------------------------------------------------------------------------------
// Scene Indices
//----------------------------------------------------------------------------------------------
void 
Model::AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex)
{
  _sceneIndexBases->AddInputScene(sceneIndex, SdfPath::AbsoluteRootPath());
}

HdSceneIndexBaseRefPtr 
Model::GetEditableSceneIndex()
{
  return _editableSceneIndex;
}

void 
Model::SetEditableSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
{
  std::cout << "Set Editable Scene Index" << sceneIndex << std::endl;

  if(_editableSceneIndex)
    _finalSceneIndex->RemoveInputScene(_editableSceneIndex);
  _editableSceneIndex = sceneIndex;
  _finalSceneIndex->AddInputScene(_editableSceneIndex,
                                  SdfPath::AbsoluteRootPath());

  std::cout << "Set Editable Scene Index done!" << sceneIndex << std::endl;
}

HdSceneIndexBaseRefPtr 
Model::GetFinalSceneIndex()
{
    return _finalSceneIndex;
}

HdSceneIndexPrim 
Model::GetPrim(SdfPath primPath)
{
  return _finalSceneIndex->GetPrim(primPath);
}

UsdPrim 
Model::GetUsdPrim(SdfPath path)
{
  return _stage->GetPrimAtPath(path);
}

UsdPrimRange 
Model::GetAllPrims()
{
  return _stage->Traverse();
}


// ---------------------------------------------------------------------------------------------
// Engines
//----------------------------------------------------------------------------------------------
void 
Model::AddEngine(Engine* engine)
{
  _engines.push_back(engine);
  _activeEngine = engine;
}

void 
Model::RemoveEngine(Engine* engine)
{
  if (engine == _activeEngine)  _activeEngine = NULL;
  for (size_t i = 0; i < _engines.size(); ++i) {
    if (engine == _engines[i]) {
      _engines.erase(_engines.begin() + i);
      break;
    }
  }
}

void 
Model::DirtyAllEngines()
{
  for (auto& engine : _engines) {
    engine->SetDirty(true);
  }
}

Engine* Model::GetActiveEngine()
{
  return _activeEngine;
}

void 
Model::SetActiveEngine(Engine* engine) 
{
  _activeEngine = engine;
}


void 
Model::AddCommand(std::shared_ptr<Command> command)
{
  CommandManager::Get()->AddCommand(command);
  CommandManager::Get()->ExecuteCommands();
}

// execution
void 
Model::ToggleExec() 
{
  _execute = 1 - _execute; 
  if (_execute)InitExec();
  else TerminateExec();
  DirtyAllEngines();
};

void 
Model::SetExec(bool state) 
{ 
  _execute = state; 
};

bool 
Model::GetExec() 
{ 
  return _execute; 
};

// get stage for display
UsdStageRefPtr
Model::GetDisplayStage()
{
  return _stage;
}

// get stage for work
UsdStageRefPtr
Model::GetWorkStage()
{
  return _stage;
}

// get current layer
SdfLayerRefPtr
Model::GetCurrentLayer()
{
  return _layer;
}

// selection
void 
Model::SetSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::SET);
}

void
Model::ToggleSelection(const SdfPathVector& selection)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, selection, SelectCommand::TOGGLE);
}

void 
Model::AddToSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::ADD);
}

void 
Model::RemoveFromSelection(const SdfPathVector& paths)
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, paths, SelectCommand::REMOVE);
}

void 
Model::ClearSelection()
{
  ADD_COMMAND(SelectCommand, Selection::PRIM, {}, SelectCommand::SET);
}

GfBBox3d
Model::GetStageBoundingBox()
{
  GfBBox3d bbox;
  TfTokenVector purposes = { UsdGeomTokens->default_ };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  return bboxCache.ComputeWorldBound(_stage->GetPseudoRoot());
}

GfBBox3d 
Model::GetSelectionBoundingBox()
{
  GfBBox3d bbox;
  static TfTokenVector purposes = {
    UsdGeomTokens->default_,
    UsdGeomTokens->proxy,
    UsdGeomTokens->guide,
    UsdGeomTokens->render
  };
  UsdGeomBBoxCache bboxCache(
    UsdTimeCode(Time::Get()->GetActiveTime()), purposes, false, false);
  for (size_t n = 0; n < _selection.GetNumSelectedItems(); ++n) {
    const Selection::Item& item = _selection[n];
    if (item.type == Selection::Type::PRIM) {
      UsdPrim prim = _stage->GetPrimAtPath(item.path);
      
      if (prim.IsActive()) {
        const GfBBox3d primBBox = bboxCache.ComputeWorldBound(prim);
        bbox = bbox.Combine(bbox, GfBBox3d(primBBox.ComputeAlignedRange()));
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
  UsdPrim& prim = _stage->GetPrimAtPath(SdfPath("/Cube"));
  if (!prim.IsValid()) {
    prim = UsdGeomCube::Define(_stage, SdfPath("/Cube")).GetPrim();
    UsdGeomCube cube(prim);
    cube.CreateSizeAttr().Set(VtValue(1.0));
   
    VtArray<TfToken> xformOpOrderTokens =
    { TfToken("xformOp:scale"), TfToken("xformOp:translate")};
    cube.CreateXformOpOrderAttr().Set(VtValue(xformOpOrderTokens));
   
  }
  UsdGeomCube cube(prim);

  bool resetXformStack = false;
  bool foundScaleOp = false;
  bool foundTranslateOp = false;
  std::vector<UsdGeomXformOp> xformOps = cube.GetOrderedXformOps(&resetXformStack);
  for (auto& xformOp : xformOps) {
 
    if (xformOp.GetName() == TfToken("xformOp:scale")) {
      GfRange3d bboxRange = bbox.GetRange();
      xformOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
      foundScaleOp = true;
    } else if(xformOp.GetName() == TfToken("xformOp:translate")) {
      GfVec3d center = bbox.ComputeCentroid();
      xformOp.Set(VtValue(center));
      foundTranslateOp = true;
    }
  }
  if (!foundScaleOp) {
    UsdGeomXformOp scaleOp = cube.AddScaleOp();
    GfRange3d bboxRange = bbox.GetRange();
    scaleOp.Set(VtValue(GfVec3f(bboxRange.GetMax() - bboxRange.GetMin())));
  }
  if (!foundTranslateOp) {
    UsdGeomXformOp translateOp = cube.AddTranslateOp();
    GfVec3d center = bbox.ComputeCentroid();
    translateOp.Set(VtValue(center));
  }
  */
  return bbox;
}

JVR_NAMESPACE_CLOSE_SCOPE
