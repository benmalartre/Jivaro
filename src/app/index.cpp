#include <iostream>
#include <string>

#include "../app/index.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE


// constructor
//----------------------------------------------------------------------------
Index::Index()
  : _execute(false)
  , _exec(nullptr)
{  
  // scene indices
  _sceneIndexBases = HdMergingSceneIndex::New();
  _finalSceneIndex = HdMergingSceneIndex::New();
  _editableSceneIndex = _sceneIndexBases;
  SetCurrentSceneIndex(_editableSceneIndex);

  UsdImagingCreateSceneIndicesInfo info;
  info.displayUnloadedPrimsWithBounds = false;
  const UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);

  _stageSceneIndex = sceneIndices.stageSceneIndex;

  AddSceneIndexBase(sceneIndices.finalSceneIndex);  
};

// destructor
//----------------------------------------------------------------------------
Index::~Index()
{
  std::cout << "destroy Scene Index Execution" << _exec << std::endl;
  if(_exec) delete _exec;
};

void
Index::SetStage(UsdStageRefPtr& stage)
{
  _stage = stage;
  _stageSceneIndex->SetStage(_stage);
  _stageSceneIndex->SetTime(UsdTimeCode::Default());
}

void
Index::Update(const float time)
{
  if(_stageSceneIndex) {
    _stageSceneIndex->ApplyPendingUpdates();
    _stageSceneIndex->SetTime(time);
  }
}

void 
Index::SetExec(Execution* exec)
{
  if(_exec)delete _exec;
  _exec = exec;
}

void 
Index::InitExec()
{
  if(!_exec)return;

  Time* time = Time::Get();
  time->SetActiveTime(time->GetStartTime());

  _exec->InitExec(_stage);

  _execSceneIndex = ExecSceneIndex::New(_sceneIndexBases);
  _execSceneIndex->SetExec(_exec);
  SetCurrentSceneIndex(_execSceneIndex);
  _execSceneIndex->UpdateExec();
}


void
Index::UpdateExec(float time)
{
  _exec->UpdateExec(_stage, time);
  _execSceneIndex->UpdateExec();
}

void
Index::TerminateExec()
{
  _finalSceneIndex->RemoveInputScene(_execSceneIndex);
  SetCurrentSceneIndex(_sceneIndexBases);
  if(_exec)_exec->TerminateExec(_stage);
  _execute = false;
  _execSceneIndex = nullptr;
  NewSceneNotice().Send();
}

void 
Index::SendExecViewEvent(const ViewEventData &data)
{
  _exec->ViewEvent(&data);
}


// execution
void 
Index::ToggleExec() 
{
  _execute = 1 - _execute; 
  if (_execute)InitExec();
  else TerminateExec();
};

void 
Index::SetExec(bool state) 
{ 
  _execute = state; 
};

bool 
Index::GetExec() 
{ 
  return _execute; 
};


// ---------------------------------------------------------------------------------------------
// Scene Indices
//----------------------------------------------------------------------------------------------
void 
Index::AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex)
{
  _sceneIndexBases->AddInputScene(sceneIndex, SdfPath::AbsoluteRootPath());
}

HdSceneIndexBaseRefPtr 
Index::GetEditableSceneIndex()
{
  return _editableSceneIndex;
}

void 
Index::SetCurrentSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
{
  if(_editableSceneIndex)
    _finalSceneIndex->RemoveInputScene(_editableSceneIndex);
  _editableSceneIndex = sceneIndex;
  _finalSceneIndex->AddInputScene(sceneIndex,
                                  SdfPath::AbsoluteRootPath());
}


HdSceneIndexBaseRefPtr 
Index::GetFinalSceneIndex()
{
    return _finalSceneIndex;
}

HdSceneIndexPrim 
Index::GetPrim(SdfPath primPath)
{
  return _finalSceneIndex->GetPrim(primPath);
}


JVR_NAMESPACE_CLOSE_SCOPE
