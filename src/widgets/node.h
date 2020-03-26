#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/ui.h"
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

class NodeUI;

#define NODE_CORNER_ROUNDING    4.f
#define NODE_PORT_RADIUS        4.f
#define NODE_PORT_SPACING       12.f
#define NODE_HEADER_HEIGHT      32.f

static int GetColorFromAttribute(const pxr::UsdAttribute& attr);

class PortUI {
public:
  PortUI(){};
  PortUI(const pxr::GraphInput& port, int index);
  PortUI(const pxr::GraphOutput& port, int index);
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

class ConnexionUI {
public:
  ConnexionUI(int id, int start, int end, int color):
    _id(id), _start(start), _end(end), _color(color){};

  void Draw();
  const int GetId() const{return _id;};
private:
  int                   _id;
  int                   _start;
  int                   _end;
  unsigned              _color;
};

class NodeUI
{
public: 
  NodeUI(const pxr::UsdPrim& prim, int& id);
  ~NodeUI();

  const pxr::GfVec2f& GetPos() const {return _pos;};
  const pxr::GfVec2f& GetSize() const {return _size;};
  const std::vector<PortUI>& GetInputs() const {return _inputs;};
  const std::vector<PortUI>& GetOutputs() const{return _outputs;};
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
  std::vector<PortUI>      _inputs;
  std::vector<PortUI>      _outputs;
  std::vector<PortUI>      _overrides;

};

AMN_NAMESPACE_CLOSE_SCOPE