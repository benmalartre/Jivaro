#pragma once

#include "../common.h"
#include "../ui/ui.h"
#include "../utils/utils.h"
//#include "../graph/node.h"
//#include "../graph/graph.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdShade/input.h>
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>

AMN_NAMESPACE_OPEN_SCOPE

class NodeUI;
class GraphUI;
class Grid2DUI;

#define NODE_CORNER_ROUNDING      4.f
#define NODE_PORT_RADIUS          4.f
#define NODE_PORT_PADDING         6.f
#define NODE_PORT_SPACING         12.f
#define NODE_HEADER_HEIGHT        24.f
#define NODE_CONNEXION_THICKNESS  2.f
#define NODE_CONNEXION_RESOLUTION 0.1f

enum ItemState {
  ITEM_STATE_NONE = 0,
  ITEM_STATE_HOVERED = 1,
  ITEM_STATE_SELECTED = 2,
  ITEM_STATE_ERROR = 4,
  ITEM_STATE_DISABLED = 8
};

static int GetColorFromAttribute(const pxr::UsdAttribute& attr);

class ItemUI {
public:
  ItemUI();
  ItemUI(const pxr::GfVec2f& pos, 
    const pxr::GfVec2f& size, int color);
  ItemUI(int color);

  void SetPosition(const pxr::GfVec2f& pos);
  void SetSize(const pxr::GfVec2f& size);
  void SetColor(const pxr::GfVec3f& color);
  void SetColor(int color);
  const pxr::GfVec2f& GetPosition() const { return _pos; };
  const pxr::GfVec2f& GetSize() const { return _size; };
  float GetWidth() const { return _size[0]; };
  float GetHeight() const { return _size[1]; };
  float GetX() const { return _pos[0]; };
  float GetY() const { return _pos[1]; };
  const int GetColor() const { return _color; };
  
  void SetState(size_t flag, bool value);
  bool GetState(size_t flag);
  virtual bool Contains(const pxr::GfVec2f& position, 
    const pxr::GfVec2f& extend = pxr::GfVec2f(0,0));
  virtual bool Intersect(const pxr::GfVec2f& start, 
    const pxr::GfVec2f& end);
  virtual bool IsVisible(GraphUI* editor) = 0;
  virtual void Draw(GraphUI* editor) = 0;

protected:
  pxr::GfVec2f _pos;
  pxr::GfVec2f _size;
  int          _color;
  short        _state;
};

class PortUI : public ItemUI {
public:
  PortUI(){};
  PortUI(NodeUI* node, bool io, const std::string& label, pxr::UsdAttribute& attr);
  PortUI(NodeUI* node, const pxr::UsdShadeInput& port);
  PortUI(NodeUI* node, const pxr::UsdShadeOutput& port);

  bool Contains(const pxr::GfVec2f& position,
    const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;

  bool IsVisible(GraphUI* editor) override { return true; };
  void Draw(GraphUI* editor) override;

  const std::string& GetName()const {return _label;};
  NodeUI* GetNode() { return _node; };
  void SetNode(NodeUI* node) { _node = node; };

private:
  NodeUI*               _node;
  std::string           _label;
  bool                  _io;
};

struct ConnexionUIData
{
  pxr::GfVec2f  p0, p1, p2, p3;
  int           numSegments;
};

class ConnexionUI : public ItemUI {
public:
  ConnexionUI(PortUI* start, PortUI* end, int color)
    : ItemUI(color)
    , _start(start)
    , _end(end){};

  bool IsVisible(GraphUI* editor) override { return true; };
  void Draw(GraphUI* editor) override;
  inline ConnexionUIData GetDescription();

  virtual bool Contains(const pxr::GfVec2f& position,
    const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;
  virtual bool Intersect(const pxr::GfVec2f& start,
    const pxr::GfVec2f& end) override;

  pxr::GfRange2f GetBoundingBox();

private:
  PortUI*               _start;
  PortUI*               _end;
};

class NodeUI : public ItemUI {
public: 
  NodeUI(const pxr::UsdPrim& prim);
  ~NodeUI();

  void AddInput(const std::string& name, pxr::SdfValueTypeName type);
  void AddOutput(const std::string& name, pxr::SdfValueTypeName type);
  size_t GetNumInputs() { return _inputs.size(); };
  size_t GetNumOutputs() { return _outputs.size(); };
  std::vector<PortUI>& GetInputs() {return _inputs;};
  std::vector<PortUI>& GetOutputs() {return _outputs;};
  void Init();
  void Update();
  bool IsVisible(GraphUI* editor) override;
  void Draw(GraphUI* graph) override;

  void ComputeSize();
  void Move(const pxr::GfVec2f& offset) { _pos += offset; };

private:
  std::string                 _name;
  pxr::UsdPrim                _prim;
  std::vector<PortUI>         _inputs;
  std::vector<PortUI>         _outputs;
};

AMN_NAMESPACE_CLOSE_SCOPE