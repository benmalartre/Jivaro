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
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "../common.h"
#include "../app/handle.h"
#include "../app/selection.h"
#include "../command/inverse.h"

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Base class
//==================================================================================
class Command {
  public:
    Command(bool undoable) : _undoable(undoable) {};
    virtual void Do() = 0;
protected:
  bool        _undoable;
  UndoInverse _inverse;

public:
  friend UndoRouter;
};

//==================================================================================
// Open Scene
//==================================================================================
class OpenSceneCommand : public Command {
public:
  OpenSceneCommand(const std::string& filename);
  ~OpenSceneCommand() {};
  void Do() override {};

};

//==================================================================================
// New Scene
//==================================================================================
class NewSceneCommand : public Command {
public:
  NewSceneCommand();
  ~NewSceneCommand() {};
  void Do() override {};

};

//==================================================================================
// Save Layer
//==================================================================================
class SaveLayerCommand : public Command {
public:
  SaveLayerCommand(pxr::SdfLayerRefPtr layer);
  ~SaveLayerCommand() {};
  void Do() override {};
};

//==================================================================================
// Save Layer As
//==================================================================================
class SaveLayerAsCommand : public Command {
public:
  SaveLayerAsCommand(pxr::SdfLayerRefPtr layer, const std::string& path);
  ~SaveLayerAsCommand() {};
  void Do() override {};
};

//==================================================================================
// Save Layer
//==================================================================================
class ReloadLayerCommand : public Command {
public:
  ReloadLayerCommand(pxr::SdfLayerRefPtr layer);
  ~ReloadLayerCommand() {};
  void Do() override {};
private:
  pxr::SdfLayerRefPtr _layer;
};

//==================================================================================
// Layer Text Edit
//==================================================================================
class LayerTextEditCommand : public Command {
public:
  LayerTextEditCommand(pxr::SdfLayerRefPtr layer, const std::string& newText);
  ~LayerTextEditCommand() {};
  void Do() override;
private:
  pxr::SdfLayerRefPtr _layer;
  std::string         _oldText;
  std::string         _newText;
};

//==================================================================================
// Create new prim
//==================================================================================
class CreatePrimCommand : public Command {
public:
  CreatePrimCommand(pxr::SdfLayerRefPtr layer, const std::string& primName);
  CreatePrimCommand(pxr::SdfPrimSpecHandle primSpec, const std::string& primName);
  ~CreatePrimCommand() {};
  void Do() override;

};

//==================================================================================
// Duplicate prim
//==================================================================================
class DuplicatePrimCommand : public Command {
public:
  DuplicatePrimCommand(pxr::UsdStageRefPtr stage, const pxr::SdfPath& path);
  ~DuplicatePrimCommand() {};
  void Do() override;

};

//==================================================================================
// Delete prim
//==================================================================================
class DeletePrimCommand : public Command {
public:
  DeletePrimCommand(pxr::UsdStageRefPtr stage, const pxr::SdfPathVector& path);
  ~DeletePrimCommand() {};
  void Do() override;

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
  void Do() override;

private:
  std::vector<Selection::Item> _previous;
};

//==================================================================================
// Translate 
//==================================================================================
class TranslateCommand : public Command {
public:
  TranslateCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    const pxr::UsdTimeCode& timeCode=pxr::UsdTimeCode::Default());
  ~TranslateCommand() {};
  void Do() override;
};

//==================================================================================
// Rotate 
//==================================================================================
class RotateCommand : public Command {
public:
  RotateCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    const pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~RotateCommand() {};
  void Do() override;
};

//==================================================================================
// Scale 
//==================================================================================
class ScaleCommand : public Command {
public:
  ScaleCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets, 
    const pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~ScaleCommand() {};
  void Do() override;
};

//==================================================================================
// Pivot 
//==================================================================================
class PivotCommand : public Command {
public:
  PivotCommand(pxr::UsdStageRefPtr stage, const HandleTargetDescList& targets,
    const pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default());
  ~PivotCommand() {};
  void Do() override;

};

//==================================================================================
// Show/Hide
//==================================================================================
class ShowHideCommand : public Command {
public:
  enum Mode {
    SHOW,
    HIDE,
    TOGGLE
  };

  ShowHideCommand(pxr::SdfPathVector& paths, Mode mode);
  ~ShowHideCommand() {};
  void Do() override;

};

//==================================================================================
// Activate/Deactivate
//==================================================================================
class ActivateCommand : public Command {
public:
  enum Mode {
    ACTIVATE,
    DEACTIVATE,
    TOGGLE
  };

  ActivateCommand(pxr::SdfPathVector& paths, Mode mode);
  ~ActivateCommand() {};
  void Do() override;
};

//==================================================================================
// Set Attribute
//==================================================================================
class SetAttributeCommand : public Command {
public:
  SetAttributeCommand(pxr::UsdAttributeVector& paths, 
    const pxr::VtValue& value, const pxr::UsdTimeCode& timeCode);
  ~SetAttributeCommand() {};
  void Do() override;
};

//==================================================================================
// Usd Generic Command
//==================================================================================
class UsdGenericCommand : public Command {
public:
  UsdGenericCommand();
  ~UsdGenericCommand() {};
  void Do() override;
};

//==================================================================================
// Create Node Command
//==================================================================================
class CreateNodeCommand : public Command {
public:
  CreateNodeCommand(const std::string& name, const pxr::SdfPath& path);
  ~CreateNodeCommand() {};
  void Do() override;
private:
  std::string   _name;
  pxr::SdfPath  _path;
};

//==================================================================================
// Move Node Command
//==================================================================================
class MoveNodeCommand : public Command {
public:
  MoveNodeCommand(const pxr::SdfPathVector& paths, const pxr::GfVec2f& offset);
  ~MoveNodeCommand() {};
  void Do() override;
private:
  pxr::SdfPathVector                _nodes;
  pxr::GfVec2f                      _offset;
};

//==================================================================================
// Expend Node Command
//==================================================================================
class ExpendNodeCommand : public Command {
public:
  ExpendNodeCommand(const pxr::SdfPathVector& nodes, const pxr::TfToken& state);
  ~ExpendNodeCommand() {};
  void Do() override;
private:
  pxr::SdfPathVector                _nodes;
};

//==================================================================================
// Connect Node Command
//==================================================================================
class ConnectNodeCommand : public Command {
public:
  ConnectNodeCommand(const pxr::SdfPath& source, const pxr::SdfPath& destination);
  ~ConnectNodeCommand() {};
  void Do() override;
private:
  pxr::SdfPath   _source;
  pxr::SdfPath   _destination;
};




JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_COMMAND_H