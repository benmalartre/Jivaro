#include "../command/command.h"
#include "../app/application.h"
#include "../app/notice.h"
#include <pxr/usd/usdGeom/mesh.h>

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Open Scene
//==================================================================================
OpenSceneCommand::OpenSceneCommand(const std::string& filename)
  : Command(false)
  , _filename(filename)
{
}

void OpenSceneCommand::Execute()
{
  GetApplication()->OpenScene(_filename);
  NewSceneNotice().Send();
}


//==================================================================================
// Create Prim
//==================================================================================
CreatePrimCommand::CreatePrimCommand(pxr::UsdStageRefPtr stage, const std::string& name) 
  : Command(true)
  , _parent()
  , _stage(stage)
  , _name(name)
{
}

CreatePrimCommand::CreatePrimCommand(pxr::UsdPrim parent, const std::string& name)
  : Command(true)
  , _parent(parent)
  , _stage()
  , _name(name)
{
}

void CreatePrimCommand::Execute() 
{
  Redo();
}

void CreatePrimCommand::Undo()
{
  if (!_prim) return;
  if (_stage) {
    _stage->RemovePrim(_prim.GetPath());
  } else {
    _prim.GetStage()->RemovePrim(_prim.GetPath());
  }
  SceneChangedNotice().Send();
}

void CreatePrimCommand::Redo() {
  if (!_stage && !_parent) return;
  if (_stage) {
    _prim = _stage->DefinePrim(pxr::SdfPath::AbsoluteRootPath().AppendChild(_name));
  }
  else if(_parent) {
    _prim = _parent.GetStage()->DefinePrim(_parent.GetPath().AppendChild(_name));
  }
  SceneChangedNotice().Send();
}

JVR_NAMESPACE_CLOSE_SCOPE