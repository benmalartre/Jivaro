#include "../command/command.h"
#include "../app/application.h"
#include "../app/notice.h"
#include <pxr/usd/usdGeom/mesh.h>

PXR_NAMESPACE_OPEN_SCOPE

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


//==================================================================================
// Translate
//==================================================================================
TranslateCommand::TranslateCommand(pxr::UsdStageRefPtr stage, const pxr::GfMatrix4f& matrix,
  std::vector<HandleTargetDesc>& targets, pxr::UsdTimeCode& timeCode)
  : Command(true)
{
  _time = timeCode;
  pxr::UsdGeomXformCache xformCache(timeCode);
  for (auto& target : targets) {
    pxr::UsdGeomXformable xformable(stage->GetPrimAtPath(target.path));
    pxr::GfMatrix4f invParentMatrix(
      xformCache.GetParentToWorldTransform(xformable.GetPrim()).GetInverse());
    pxr::GfMatrix4d xformMatrix((target.offset * matrix) * invParentMatrix);


    pxr::UsdGeomXformCommonAPI api(xformable.GetPrim());
    pxr::GfVec3d translation;
    pxr::GfVec3f rotation;
    pxr::GfVec3f scale;
    pxr::GfVec3f pivot;
    pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;

    api.GetXformVectors(&translation, &rotation, &scale, &pivot, &rotOrder, timeCode);
    _origin.push_back(pxr::GfVec3f(translation));
    _translate.push_back(pxr::GfVec3f(xformMatrix.GetRow3(3)));
    _prims.push_back(xformable.GetPrim());
  }
}

void TranslateCommand::Execute()
{
  Redo();
}

static void _EnsureXformCommonAPI(pxr::UsdPrim& prim, pxr::UsdTimeCode& timeCode)
{
  pxr::GfVec3d translation;
  pxr::GfVec3f rotation;
  pxr::GfVec3f scale;
  pxr::GfVec3f pivot;
  pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;
  pxr::UsdGeomXformCommonAPI api(prim);
  api.GetXformVectors(&translation, &rotation, &scale, &pivot, &rotOrder, timeCode);
  pxr::UsdGeomXformable xformable(prim);
  xformable.ClearXformOpOrder();
  api.SetXformVectors(translation, rotation, scale, pivot, rotOrder, timeCode);
}

void TranslateCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetTranslate(_origin[i], _time);
  }
  SceneChangedNotice().Send();
}

void TranslateCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      std::cout << "FAIL CREATE API " << _prims[i].GetPath() << std::endl;
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetTranslate(_translate[i], _time);
  }
  SceneChangedNotice().Send();
}

PXR_NAMESPACE_CLOSE_SCOPE