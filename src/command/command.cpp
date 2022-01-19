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
// Selection
//==================================================================================
SelectCommand::SelectCommand(Selection::Type type, 
  const pxr::SdfPathVector& paths, int mode)
  : Command(true)
  , _paths(paths)
  , _type(type)
  , _mode(mode)
{
}

void SelectCommand::Execute()
{
  Selection* selection = GetApplication()->GetSelection();
  _previous = selection->GetItems();
  Redo();
}

void SelectCommand::Undo()
{
  Selection* selection = GetApplication()->GetSelection();
  selection->SetItems(_previous);
  SelectionChangedNotice().Send();
}

void SelectCommand::Redo() {
  Selection* selection = GetApplication()->GetSelection();
  switch (_mode) {
  case 0:
    selection->Clear();
    for (auto& path : _paths) selection->AddItem(path);
    break;
  case 1:
    for (auto& path : _paths) selection->AddItem(path);
    break;
  case 2:
    for (auto& path : _paths) selection->RemoveItem(path);
    break;
  case 3:
    for (auto& path : _paths) selection->ToggleItem(path);
    break;
  }
  SelectionChangedNotice().Send();
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

    _origin.push_back(pxr::GfVec3f(target.previous.translation));
    _translate.push_back(pxr::GfVec3f(xformMatrix.GetRow3(3)));
    _prims.push_back(xformable.GetPrim());
    target.previous.translation = _translate.back();
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
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetTranslate(_translate[i], _time);
  }
  SceneChangedNotice().Send();
}

//==================================================================================
// Rotate
//==================================================================================
RotateCommand::RotateCommand(pxr::UsdStageRefPtr stage, const pxr::GfMatrix4f& matrix,
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
    const GfVec3d xAxis = target.parent.GetRow3(0);
    const GfVec3d yAxis = target.parent.GetRow3(1);
    const GfVec3d zAxis = target.parent.GetRow3(2);

    // Get latest rotation values to give a hint to the decompose function
    HandleTargetXformVectors vectors;
    api.GetXformVectors(&vectors.translation, &vectors.rotation, &vectors.scale,
      &vectors.pivot, &vectors.rotOrder, _time);
    double thetaTw = GfDegreesToRadians(vectors.rotation[0]);
    double thetaFB = GfDegreesToRadians(vectors.rotation[1]);
    double thetaLR = GfDegreesToRadians(vectors.rotation[2]);
    double thetaSw = 0.0;

    // Decompose the matrix in angle values
    GfRotation::DecomposeRotation(xformMatrix, xAxis, yAxis, zAxis, 1.0,
      &thetaTw, &thetaFB, &thetaLR, &thetaSw, true);
    const GfVec3f rotation =
      GfVec3f(GfRadiansToDegrees(thetaTw), GfRadiansToDegrees(thetaFB), GfRadiansToDegrees(thetaLR));
    api.SetRotate(rotation, vectors.rotOrder, _time);

    _origin.push_back(target.previous.rotation);
    _rotation.push_back(rotation);
    _prims.push_back(xformable.GetPrim());
    _rotOrder.push_back(vectors.rotOrder);
    target.previous.rotation = _rotation.back();
  }
}

void RotateCommand::Execute()
{
  Redo();
}


void RotateCommand::Undo()
{
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    api.SetRotate(_origin[i], _rotOrder[i], _time);
  }
  SceneChangedNotice().Send();
}

void RotateCommand::Redo() {
  for (size_t i = 0; i < _prims.size(); ++i) {
    pxr::UsdGeomXformCommonAPI api(_prims[i]);
    if (!api) {
      _EnsureXformCommonAPI(_prims[i], _time);
    }
    api.SetRotate(_rotation[i],_rotOrder[i], _time);
  }
  SceneChangedNotice().Send();
}

PXR_NAMESPACE_CLOSE_SCOPE