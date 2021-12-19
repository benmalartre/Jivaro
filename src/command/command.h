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

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

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
// Creating new prim
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
    pxr::UsdPrim        _prim;
    pxr::UsdPrim        _parent;
    pxr::UsdStageRefPtr _stage;
    pxr::TfToken        _name;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_COMMAND_H