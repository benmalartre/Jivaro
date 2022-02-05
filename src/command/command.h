#ifndef JVR_COMMAND_COMMAND_H
#define JVR_COMMAND_COMMAND_H
#include <stack>
#include <iostream>
#include <memory>

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/variantSpec.h>
#include <pxr/usd/sdf/variantSetSpec.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/namespaceEdit.h>
#include <pxr/usd/sdf/valueTypeName.h>
#include <pxr/usd/sdf/path.h>

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "../common.h"
#include "../app/handle.h"
#include "../app/selection.h"

PXR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Base class
//==================================================================================
class Command {
  public:
    Command(bool undoable) : _undoable(undoable) {};
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() = 0;
protected:
  bool _undoable;
};

//==================================================================================
// Open Scene
//==================================================================================
class OpenSceneCommand : public Command {
public:
  OpenSceneCommand(const std::string& filename);
  ~OpenSceneCommand() {};
  void Execute() override;
  void Undo() override {};
  void Redo() override {};
protected:
  std::string _filename;
};

//==================================================================================
// Create new prim
//==================================================================================
class CreatePrimCommand : public Command {
public:
  CreatePrimCommand(pxr::UsdStageRefPtr stage, const std::string& primName);
  CreatePrimCommand(pxr::UsdPrim prim, const std::string& primName);
  ~CreatePrimCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
    pxr::UsdPrim          _prim;
    pxr::UsdPrim          _parent;
    pxr::UsdStageWeakPtr  _stage;
    pxr::TfToken          _name;
};

//==================================================================================
// Duplicate prim
//==================================================================================
class DuplicatePrimCommand : public Command {
public:
  DuplicatePrimCommand(pxr::UsdStageRefPtr stage, const pxr::SdfPath& path);
  ~DuplicatePrimCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  pxr::SdfPath          _sourcePath;
  pxr::SdfPath          _destinationPath;
  pxr::UsdStageWeakPtr  _stage;
};

//==================================================================================
// Select 
// mode : 0 = Set
//        1 = Add
//        2 = Remove
//        3 = Toggle
//==================================================================================
class SelectCommand : public Command {
public:
  enum Mode {
    SET,
    ADD,
    REMOVE,
    TOGGLE
  };
  SelectCommand(Selection::Type type, const pxr::SdfPathVector& paths, int mode);
  ~SelectCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  std::vector<pxr::SdfPath>         _paths;
  std::vector<std::vector<int>>     _indices;
  Selection::Type                   _type;
  int                               _mode;
  std::vector<Selection::Item>      _previous;
};

//==================================================================================
// Translate 
//==================================================================================
class TranslateCommand : public Command {
public:
  TranslateCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    pxr::UsdTimeCode& timeCode=pxr::UsdTimeCode::Default());
  ~TranslateCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  std::vector<pxr::UsdPrim>          _prims;
  std::vector<pxr::GfVec3d>          _translate;
  std::vector<pxr::GfVec3d>          _origin;
  pxr::UsdTimeCode                   _time;
};

//==================================================================================
// Rotate 
//==================================================================================
class RotateCommand : public Command {
public:
  RotateCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~RotateCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  std::vector<pxr::UsdPrim>          _prims;
  std::vector<pxr::GfVec3f>          _rotation;
  std::vector<pxr::GfVec3f>          _origin;
  std::vector<pxr::UsdGeomXformCommonAPI::RotationOrder> _rotOrder;
  pxr::UsdTimeCode                   _time;
};

//==================================================================================
// Scale 
//==================================================================================
class ScaleCommand : public Command {
public:
  ScaleCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets, 
    pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~ScaleCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  std::vector<pxr::UsdPrim>          _prims;
  std::vector<pxr::GfVec3f>          _scale;
  std::vector<pxr::GfVec3f>          _origin;
  pxr::UsdTimeCode                   _time;
};

//==================================================================================
// Pivot 
//==================================================================================
class PivotCommand : public Command {
public:
  PivotCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~PivotCommand() {};
  void Execute() override;
  void Undo() override;
  void Redo() override;
private:
  std::vector<pxr::UsdPrim>          _prims;
  std::vector<pxr::GfVec3f>          _pivot;
  std::vector<pxr::GfVec3f>          _origin;
  pxr::UsdTimeCode                   _time;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_COMMAND_H