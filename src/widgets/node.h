#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../imgui/imgui_nodes.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>

AMN_NAMESPACE_OPEN_SCOPE
enum GRAPH_COLORS {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFFFF6600,
  GRAPH_COLOR_INTEGER           = 0xFF336611,
  GRAPH_COLOR_ENUM              = 0xFF339911,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_VECTOR2           = 0xFFFFCC00,
  GRAPH_COLOR_VECTOR3           = 0xFFFFFF00,
  GRAPH_COLOR_VECTOR4           = 0xFFFFFF66,
  GRAPH_COLOR_COLOR             = 0xFFFF0000, 
  GRAPH_COLOR_ROTATION          = 0xFFCCFFFF,
  GRAPH_COLOR_QUATERNION        = 0xFF66FFFF,
  GRAPH_COLOR_MATRIX3           = 0xFF00FFFF,
  GRAPH_COLOR_MATRIX4           = 0xFF33CCFF,
  GRAPH_COLOR_STRING            = 0xFFCC99FF,
  GRAPH_COLOR_SHAPE             = 0xFFFF3399,
  GRAPH_COLOR_TOPOLOGY          = 0xFFCCCCCC,
  GRAPH_COLOR_GEOMETRY          = 0xFFFF3366,
  GRAPH_COLOR_LOCATION          = 0xFF555577,
  GRAPH_COLOR_CONTOUR           = 0xFF000000,
};

class AmnNodeUI;

#define NODE_CORNER_ROUNDING    4.f
#define NODE_PORT_RADIUS        4
#define NODE_PORT_SPACING       12
#define NODE_HEADER_HEIGHT      32

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