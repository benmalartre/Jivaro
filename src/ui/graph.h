#ifndef AMN_UI_GRAPH_H
#define AMN_UI_GRAPH_H

#include <vector>
#include <set>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"

#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primFlags.h>

AMN_NAMESPACE_OPEN_SCOPE

#define AMN_GRAPH_CELL_MAX_NODES 6

enum ColorGraph {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFF0066FF,
  GRAPH_COLOR_INTEGER           = 0xFF116633,
  GRAPH_COLOR_ENUM              = 0xFF119933,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_VECTOR2           = 0xFF00CCFF,
  GRAPH_COLOR_VECTOR3           = 0xFF00FFFF,
  GRAPH_COLOR_VECTOR4           = 0xFF66FFFF,
  GRAPH_COLOR_COLOR             = 0xFF0000FF, 
  GRAPH_COLOR_ROTATION          = 0xFFFFFFCC,
  GRAPH_COLOR_QUATERNION        = 0xFFFFFF66,
  GRAPH_COLOR_MATRIX3           = 0xFFFFFF00,
  GRAPH_COLOR_MATRIX4           = 0xFFFFCC33,
  GRAPH_COLOR_STRING            = 0xFFFF99CC,
  GRAPH_COLOR_SHAPE             = 0xFF9933FF,
  GRAPH_COLOR_TOPOLOGY          = 0xFFCCCCCC,
  GRAPH_COLOR_GEOMETRY          = 0xFF6633FF,
  GRAPH_COLOR_LOCATION          = 0xFF775555,
  GRAPH_COLOR_CONTOUR           = 0xFF000000,
  GRAPH_COLOR_PRIM              = 0xFF6622FF
};

#define NODE_CORNER_ROUNDING          4.f
#define NODE_PORT_RADIUS              4.f
#define NODE_PORT_PADDING             6.f
#define NODE_PORT_VERTICAL_SPACING    16.f
#define NODE_PORT_HORIZONTAL_SPACING  12.f
#define NODE_HEADER_HEIGHT            24.f
#define NODE_HEADER_PADDING           4
#define NODE_CONNEXION_THICKNESS      2.f
#define NODE_CONNEXION_RESOLUTION     0.1f

static ImColor NODE_CONTOUR_DEFAULT(0, 0, 0, 100);
static ImColor NODE_CONTOUR_SELECTED(255, 255, 255, 255);
static ImColor NODE_CONTOUR_HOVERED(60, 60, 60, 100);

static int GetColorFromAttribute(const pxr::UsdAttribute& attr);

class GraphUI : public BaseUI
{
protected:
  enum ItemState {
    ITEM_STATE_NONE = 0,
    ITEM_STATE_HOVERED = 1,
    ITEM_STATE_SELECTED = 2,
    ITEM_STATE_ERROR = 4,
    ITEM_STATE_DISABLED = 8
  };

  class Node;
  class Port;

  // Graph item base class
  //-------------------------------------------------------------------
  class Item {
    public:
      Item();
      Item(const pxr::GfVec2f& pos, 
        const pxr::GfVec2f& size, int color);
      Item(int color);

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

  // Graph port class
  //-------------------------------------------------------------------
  class Port : public Item {
    public:
      Port() {};
      Port(Node* node, bool io, const std::string& label, 
        pxr::UsdAttribute& attr);
      //Port(Node* node, const pxr::UsdShadeInput& port);
      //Port(Node* node, const pxr::UsdShadeOutput& port);

      bool Contains(const pxr::GfVec2f& position,
        const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;

      bool IsVisible(GraphUI* editor) override { return true; };
      void Draw(GraphUI* editor) override;

      bool IsInput() { return _io; };
      bool IsOutput() { return !_io; };

      const std::string& GetName()const {return _label;};
      Node* GetNode() { return _node; };
      void SetNode(Node* node) { _node = node; };
      const pxr::UsdAttribute& GetAttr() const { return _attr;};
      pxr::UsdAttribute& GetAttr() { return _attr;};

    private:
      Node*                 _node;
      std::string           _label;
      bool                  _io;
      pxr::UsdAttribute     _attr;
  };

  struct ConnexionData
  {
    pxr::GfVec2f  p0, p1, p2, p3;
    int           numSegments;
  };

  // Graph connexion class
  //-------------------------------------------------------------------
  class Connexion : public Item {
    public:
      Connexion(Port* start, Port* end, int color)
        : Item(color)
        , _start(start)
        , _end(end){};

      bool IsVisible(GraphUI* editor) override { return true; };
      void Draw(GraphUI* editor) override;
      inline ConnexionData GetDescription();

      virtual bool Contains(const pxr::GfVec2f& position,
        const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;
      virtual bool Intersect(const pxr::GfVec2f& start,
        const pxr::GfVec2f& end) override;

      pxr::GfRange2f GetBoundingBox();

    private:
      Port*               _start;
      Port*               _end;
  };

  // Graph node class
  //-------------------------------------------------------------------
  class Node : public Item {
    public: 
      Node(pxr::UsdPrim& prim);
      ~Node();

      void AddInput(const std::string& name, pxr::SdfValueTypeName type);
      void AddOutput(const std::string& name, pxr::SdfValueTypeName type);
      size_t GetNumInputs() { return _inputs.size(); };
      size_t GetNumOutputs() { return _outputs.size(); };
      std::vector<Port>& GetInputs() {return _inputs;};
      std::vector<Port>& GetOutputs() {return _outputs;};
      void Init();
      void Update();
      bool IsVisible(GraphUI* editor) override;
      void Draw(GraphUI* graph) override;

      void ComputeSize();
      void Move(const pxr::GfVec2f& offset) { _pos += offset; };

    private:
      pxr::TfToken                _name;
      pxr::UsdPrim                _prim;
      std::vector<Port>           _inputs;
      std::vector<Port>           _outputs;
  };

  // Graph cell class
  //-------------------------------------------------------------------
  struct Cell {
    bool                                 isLeaf;
    std::vector<Node*>                   nodes;
    Cell*                                cells[4];

    Cell():isLeaf(true) {
      for (int i = 0; i < 4; ++i) cells[i] = NULL;
    };
  };

  struct Marquee {
    pxr::GfVec2f start;
    pxr::GfVec2f end;
  };

  struct Connect {
    Port* startPort;
    Port* endPort;
    int     color;
  };

private:
  enum NavigateMode {
    IDLE,
    PAN,
    ZOOM
  };

public:
  GraphUI(View* parent, const std::string& filename);
  ~GraphUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void MouseWheel(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  bool Draw() override;
  void DrawGrid();

  inline float GetScale() const { return _scale; };
  inline const pxr::GfVec2f& GetOffset() const { return _offset; };

  void Init(const std::string& filename);
  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  void Term();

  // font
  void UpdateFont();
  inline size_t GetFontIndex() { return _fontIndex; };
  inline float GetFontScale() { return _fontScale; };

  // conversion
  pxr::GfVec2f ViewPositionToGridPosition(const pxr::GfVec2f& mousePos);
  pxr::GfVec2f GridPositionToViewPosition(const pxr::GfVec2f& gridPos);
  
  // nodes
  void AddNode(Node* node) { _nodes.push_back(node); };
  Node* GetLastNode() { return _nodes.back(); };

  // connection
  void StartConnexion();
  void UpdateConnexion();
  void EndConnexion();

  // selection
  void AddToSelection(Node* node, bool bringToFront);
  void RemoveFromSelection(Node* node);
  void ClearSelection();
  void MarqueeSelect(int mod);

  void BuildGrid();
  void BuildGraph();
  
private:
  void _GetPortUnderMouse(const pxr::GfVec2f& mousePos, Node* node);
  void _GetNodeUnderMouse(const pxr::GfVec2f& mousePos, bool useExtend = false);
  void _GetConnexionUnderMouse(const pxr::GfVec2f& mousePos);
  void _RecurseStagePrim(const pxr::UsdPrim& prim);
  
  int                                   _id;
  int                                   _depth;
  pxr::GfVec2f                          _offset;  
  float                                 _scale;
  float                                 _invScale;
  int                                   _lastX;
  int                                   _lastY;
  bool                                  _drag;
  bool                                  _marque;
  bool                                  _connect;
  short                                 _navigate;
  float                                 _fontScale;
  size_t                                _fontIndex;

  int                                   _nodeId;
  pxr::UsdStageRefPtr                   _stage;
  std::vector<Node*>                    _nodes;
  std::vector<Connexion*>               _connexions;
  std::set<Node*>                       _selected;
  Node*                                 _hoveredNode;
  Node*                                 _currentNode;
  Port*                                 _hoveredPort;
  Port*                                 _currentPort;
  Connexion*                            _hoveredConnexion;

  Marquee                               _marquee;
  Cell*                                 _grid;
  Connect                               _connector;

  static ImGuiWindowFlags               _flags;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_GRAPH_H