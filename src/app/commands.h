#ifndef JVR_APPLICATION_COMMANDS_H
#define JVR_APPLICATION_COMMANDS_H
#include <stack>
#include <iostream>
#include <memory>

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>

#include "../command/command.h"
#include "../command/inverse.h"
#include "../command/block.h"

#include "../app/selection.h"

JVR_NAMESPACE_OPEN_SCOPE

struct ManipTargetDesc;
using ManipTargetDescList = std::vector<ManipTargetDesc>;

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
  NewSceneCommand(const std::string& filename);
  ~NewSceneCommand() {};
  void Do() override {};

};

//==================================================================================
// Save Layer
//==================================================================================
class SaveLayerCommand : public Command {
public:
  SaveLayerCommand(SdfLayerHandle layer);
  ~SaveLayerCommand() {};
  void Do() override {};
};

//==================================================================================
// Save Layer As
//==================================================================================
class SaveLayerAsCommand : public Command {
public:
  SaveLayerAsCommand(SdfLayerHandle layer, const std::string& path);
  ~SaveLayerAsCommand() {};
  void Do() override {};
};

//==================================================================================
// Reload Layer
//==================================================================================
class ReloadLayerCommand : public Command {
public:
  ReloadLayerCommand(SdfLayerHandle layer);
  ~ReloadLayerCommand() {};
  void Do() override {};
private:
  SdfLayerRefPtr _layer;
};

//==================================================================================
// Layer Text Edit
//==================================================================================
class LayerTextEditCommand : public Command {
public:
  LayerTextEditCommand(SdfLayerRefPtr layer, const std::string& newText);
  ~LayerTextEditCommand() {};
  void Do() override;
private:
  SdfLayerRefPtr _layer;
  std::string         _oldText;
  std::string         _newText;
};

//==================================================================================
// Create new prim
//==================================================================================
class Geometry;
class CreatePrimCommand : public Command {
public:
  CreatePrimCommand(SdfLayerRefPtr layer, const SdfPath& path, const TfToken& type, 
    bool asDefault=false, Geometry* geometry=NULL);
  ~CreatePrimCommand() {};
  void Do() override;

};

//==================================================================================
// Rename prim
//==================================================================================
class RenamePrimCommand : public Command {
public:
  RenamePrimCommand(SdfLayerRefPtr layer, const SdfPath& path, const TfToken& name);
  ~RenamePrimCommand() {};
  void Do() override;

};

//==================================================================================
// Duplicate prim
//==================================================================================
class DuplicatePrimCommand : public Command {
public:
  DuplicatePrimCommand(SdfLayerRefPtr layer, const SdfPath& path);
  ~DuplicatePrimCommand() {};
  void Do() override;
  
private:
  std::vector<Selection::Item> _selection;
};

//==================================================================================
// Delete prim
//==================================================================================
class DeletePrimCommand : public Command {
public:
  DeletePrimCommand(){};
  DeletePrimCommand(UsdStageRefPtr stage, const SdfPathVector& path);
  ~DeletePrimCommand() {};
  void Do() override;

private:
  std::vector<Selection::Item> _selection;
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
  SelectCommand(short type, const SdfPathVector& paths, int mode);
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
  TranslateCommand(UsdStageRefPtr stage, const ManipTargetDescList& targets,
    const UsdTimeCode& timeCode=UsdTimeCode::Default());
  ~TranslateCommand() {};
  void Do() override;
};

//==================================================================================
// Rotate 
//==================================================================================
class RotateCommand : public Command {
public:
  RotateCommand(UsdStageRefPtr stage, const ManipTargetDescList& targets,
    const UsdTimeCode& timeCode = UsdTimeCode::Default());
  ~RotateCommand() {};
  void Do() override;
};

//==================================================================================
// Scale 
//==================================================================================
class ScaleCommand : public Command {
public:
  ScaleCommand(UsdStageRefPtr stage, const ManipTargetDescList& targets, 
    const UsdTimeCode& timeCode = UsdTimeCode::Default());
  ~ScaleCommand() {};
  void Do() override;
};

//==================================================================================
// Pivot 
//==================================================================================
class PivotCommand : public Command {
public:
  PivotCommand(UsdStageRefPtr stage, const ManipTargetDescList& targets,
    const UsdTimeCode& timeCode = UsdTimeCode::Default());
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

  ShowHideCommand(SdfPathVector& paths, Mode mode);
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

  ActivateCommand(SdfPathVector& paths, Mode mode);
  ~ActivateCommand() {};
  void Do() override;
};

//==================================================================================
// Set Attribute
//==================================================================================
class SetAttributeCommand : public Command {
public:
  SetAttributeCommand(UsdAttributeVector& paths, 
    const VtValue& value, const VtValue& previous, 
    const UsdTimeCode& timeCode);
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
  CreateNodeCommand(const std::string& name, const SdfPath& path);
  ~CreateNodeCommand() {};
  void Do() override;
private:
  std::string   _name;
  SdfPath  _path;
};

//==================================================================================
// Move Node Command
//==================================================================================
class MoveNodeCommand : public Command {
public:
  MoveNodeCommand(const SdfPathVector& paths, const GfVec2f& offset);
  ~MoveNodeCommand() {};
  void Do() override;
private:
  SdfPathVector                _nodes;
  GfVec2f                      _offset;
};

//==================================================================================
// Size Node Command
//==================================================================================
class SizeNodeCommand : public Command {
public:
  SizeNodeCommand(const SdfPathVector& paths, const GfVec2f& offset);
  ~SizeNodeCommand() {};
  void Do() override;
private:
  SdfPathVector                _nodes;
  GfVec2f                      _offset;
};

//==================================================================================
// Expend Node Command
//==================================================================================
class ExpendNodeCommand : public Command {
public:
  ExpendNodeCommand(const SdfPathVector& nodes, const TfToken& state);
  ~ExpendNodeCommand() {};
  void Do() override;
private:
  SdfPathVector                _nodes;
};

//==================================================================================
// Connect Node Command
//==================================================================================
class ConnectNodeCommand : public Command {
public:
  ConnectNodeCommand(const SdfPath& source, const SdfPath& destination);
  ~ConnectNodeCommand() {};
  void Do() override;
private:
  SdfPath   _source;
  SdfPath   _destination;
};

//==================================================================================
// UI Generic Command
//==================================================================================
class UIGenericCommand : public Command {
public:
  UIGenericCommand(CALLBACK_FN fn);
  ~UIGenericCommand() {};
  void Do() override;

private:
  CALLBACK_FN _fn;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_COMMANDS_H