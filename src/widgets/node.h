#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../imgui/imgui_nodes.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include "../imgui/imgui_nodes.h"

AMN_NAMESPACE_OPEN_SCOPE

class AmnNodeUI;

#define NODE_CORNER_ROUNDING    4.f
#define NODE_PORT_RADIUS        4.f
#define NODE_PORT_SPACING       12.f
#define NODE_HEADER_HEIGHT      32.f

static int GetColorFromAttribute(const pxr::UsdAttribute& attr);

class AmnPortUI {
public:
  AmnPortUI(){};
  AmnPortUI(const pxr::GraphInput& port, int index);
  AmnPortUI(const pxr::GraphOutput& port, int index);
  void Draw();

  const std::string& GetName()const {return _label;};
  const int GetId() const{return _id;};

private:
  int                   _id;
  std::string           _label;
  int                   _color;
  pxr::GfVec2f          _pos;
  bool                  _io;
};

class AmnConnexionUI {
public:
  AmnConnexionUI(int id, int start, int end, int color):
    _id(id), _start(start), _end(end), _color(color){};

  void Draw();
  const int GetId() const{return _id;};
private:
  int                   _id;
  int                   _start;
  int                   _end;
  unsigned              _color;
};

class AmnNodeUI
{
public: 
  AmnNodeUI(const pxr::UsdPrim& prim, int& id);
  ~AmnNodeUI();

  const pxr::GfVec2f& GetPos() const {return _pos;};
  const pxr::GfVec2f& GetSize() const {return _size;};
  const std::vector<AmnPortUI>& GetInputs() const {return _inputs;};
  const std::vector<AmnPortUI>& GetOutputs() const{return _outputs;};
  const int GetId() const{return _id;};
  void Update();
  void Draw();

private:
  int                         _id;
  pxr::GfVec2f                _pos;
  pxr::GfVec2f                _size;
  pxr::GfVec3f                _color;
  std::string                 _name;
  pxr::UsdPrim                _prim;
  std::vector<AmnPortUI>      _inputs;
  std::vector<AmnPortUI>      _outputs;
  std::vector<AmnPortUI>      _overrides;

};

AMN_NAMESPACE_CLOSE_SCOPE