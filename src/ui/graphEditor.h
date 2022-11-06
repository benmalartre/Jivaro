#ifndef JVR_UI_GRAPH_H
#define JVR_UI_GRAPH_H

#include <vector>
#include <set>

#include "../common.h"
#include "../utils/icons.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../app/selection.h"

#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primFlags.h>

JVR_NAMESPACE_OPEN_SCOPE
#define GRAPH_CELL_MAX_NODES 6

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

#define NODE_CORNER_ROUNDING          2.f
#define NODE_PORT_RADIUS              4.f
#define NODE_PORT_PADDING             6.f
#define NODE_PORT_VERTICAL_SPACING    16.f
#define NODE_PORT_HORIZONTAL_SPACING  12.f
#define NODE_HEADER_HEIGHT            24.f
#define NODE_HEADER_PADDING           4
#define NODE_EXPENDED_SIZE            12
#define NODE_CONNEXION_THICKNESS      2.f
#define NODE_CONNEXION_RESOLUTION     0.1f

static ImColor NODE_CONTOUR_DEFAULT(0, 0, 0, 100);
static ImColor NODE_CONTOUR_SELECTED(255, 255, 255, 255);
static ImColor NODE_CONTOUR_HOVERED(60, 60, 60, 100);

static int _GetColorFromAttribute(const pxr::UsdAttribute& attr);

class Selection;

class GraphEditorUI : public BaseUI
{
public:
  static const pxr::TfToken NodeExpendState[3];

  enum GraphType {
    HIERARCHY,
    MATERIAL,
    EXECUTION
  };

  enum GraphDirection {
    VERTICAL,
    HORIZONTAL
  };

protected:
  enum ItemState {
    ITEM_STATE_NONE = 0,
    ITEM_STATE_HOVERED = 1,
    ITEM_STATE_SELECTED = 2,
    ITEM_STATE_ERROR = 4,
    ITEM_STATE_DISABLED = 8
  };

  enum DisplayState {
    COLLAPSED,
    CONNECTED,
    EXPENDED
  };

  class Node;
  class Port;
  class Connexion;

  // Graph item base class
  //-------------------------------------------------------------------
  class Item {
    public:
      Item();
      Item(const pxr::GfVec2f& pos, 
        const pxr::GfVec2f& size, int color);
      Item(int color);

      virtual void SetPosition(const pxr::GfVec2f& pos);
      virtual void SetSize(const pxr::GfVec2f& size);
      virtual void SetColor(const pxr::GfVec3f& color);
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
      virtual bool IsVisible(GraphEditorUI* editor) = 0;
      virtual void Draw(GraphEditorUI* editor) = 0;

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
      enum Flag {
        INPUT = 1,
        OUTPUT = 2
      };

      Port() {};
      Port(Node* node, Flag flag, const pxr::TfToken& label, 
        pxr::UsdAttribute& attr);

      bool Contains(const pxr::GfVec2f& position,
        const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;

      bool IsVisible(GraphEditorUI* editor) override { return true; };
      bool IsConnected(GraphEditorUI* editor, Connexion* connexion=NULL);
      void Draw(GraphEditorUI* editor) override;

      bool IsInput() { return _flags & INPUT; };
      bool IsOutput() { return _flags & OUTPUT; };
      bool IsBothInputOutput() { return _flags & (INPUT | OUTPUT); };

      const pxr::TfToken& GetName()const {return _label;};
      pxr::SdfPath GetPath();
      const Node* GetNode() const { return _node; };
      Node* GetNode() { return _node; };
      void SetNode(Node* node) { _node = node; };
      const pxr::UsdAttribute& GetAttr() const { return _attr;};
      pxr::UsdAttribute& GetAttr() { return _attr;};
      Flag GetFlags() { return _flags; };

    private:
      Node*                 _node;
      pxr::TfToken          _label;
      Flag                  _flags;
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

      bool IsVisible(GraphEditorUI* editor) override { return true; };
      void Draw(GraphEditorUI* editor) override;
      inline ConnexionData GetDescription();

      virtual bool Contains(const pxr::GfVec2f& position,
        const pxr::GfVec2f& extend = pxr::GfVec2f(0, 0)) override;
      virtual bool Intersect(const pxr::GfVec2f& start,
        const pxr::GfVec2f& end) override;

      pxr::GfRange2f GetBoundingBox();
      Port* GetStart() { return _start; };
      Port* GetEnd() { return _end; };

    private:
      Port*               _start;
      Port*               _end;
  };


  // Graph node class
  //-------------------------------------------------------------------
  class Node : public Item {
    public: 
      enum {
        DIRTY_CLEAN = 0,
        DIRTY_SIZE = 1,
        DIRTY_POSITION = 2,
      };
      Node(pxr::UsdPrim prim, bool write=false);
      ~Node();

      void AddInput(pxr::UsdAttribute& attribute, const pxr::TfToken& name);
      void AddOutput(pxr::UsdAttribute& attribute, const pxr::TfToken& name);
      void AddPort(pxr::UsdAttribute& attribute, const pxr::TfToken& name);

      size_t GetNumPorts() { return _ports.size(); };
      std::vector<Port>& GetPorts() { return _ports; };
      pxr::UsdPrim& GetPrim() { return _prim; };
      const pxr::UsdPrim& GetPrim() const { return _prim; };
      void Init();
      void Update();
      void SetPosition(const pxr::GfVec2f& pos) override;
      void SetSize(const pxr::GfVec2f& size) override;
      void SetColor(const pxr::GfVec3f& color) override;
      bool IsVisible(GraphEditorUI* editor) override;
      void Draw(GraphEditorUI* graph) override;
      void SetBackgroundColor(const pxr::GfVec3f& color) { _backgroundColor = color; };
      void SetExpansionState(short state) { _state = state; };
      void UpdateExpansionState();

      void ComputeSize(GraphEditorUI* editor);

      Port* GetPort(const pxr::TfToken& name);

    private:
      Node*                       _parent;
      pxr::GfVec3f                _backgroundColor;
      short                       _expended;
      pxr::TfToken                _name;
      pxr::UsdPrim                _prim;
      std::vector<Port>           _ports;
      short                       _dirty;
  };

  // Graph graph class
  //-------------------------------------------------------------------
  class Graph : public Node {
    public: 
      Graph(pxr::UsdPrim& prim);
      ~Graph();

      void Populate(pxr::UsdPrim& prim);
      void Clear();

      void AddNode(Node* node);
      void RemoveNode(Node* node);

      void AddConnexion(Connexion* connexion);
      void RemoveConnexion(Connexion* connexion);

      const std::vector<Node*>& GetNodes() const { return _nodes; };
      std::vector<Node*>& GetNodes() { return _nodes; };

      const Node* GetNode(const pxr::UsdPrim& prim) const;
      Node* GetNode(const pxr::UsdPrim& prim);

      const std::vector<Connexion*>& GetConnexions() const { return _connexions; };
      std::vector<Connexion*>& GetConnexions() { return _connexions; };

      /*
      void AddInput(const std::string& name, pxr::SdfValueTypeName type);
      void AddOutput(const std::string& name, pxr::SdfValueTypeName type);
      size_t GetNumInputs() { return _inputs.size(); };
      size_t GetNumOutputs() { return _outputs.size(); };
      std::vector<Port>& GetInputs() {return _inputs;};
      std::vector<Port>& GetOutputs() {return _outputs;};
      void Init();
      void Update();
      bool IsVisible(GraphEditorUI* editor) override;
      void Draw(GraphEditorUI* graph) override;

      void ComputeSize();
      void Move(const pxr::GfVec2f& offset) { _pos += offset; };
      */

    private:
      void _DiscoverNodes(pxr::UsdPrim& prim);
      void _RecurseNodes(pxr::UsdPrim& prim);
      void _DiscoverConnexions(pxr::UsdPrim& prim);
      void _RecurseConnexions(pxr::UsdPrim& prim);

      pxr::GfVec3f                _backgroundColor;
      float                       _currentX;
      float                       _currentY;
      pxr::TfToken                _name;
      pxr::UsdPrim                _prim;
      std::vector<Node*>          _nodes;
      std::vector<Connexion*>     _connexions;
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
    int   color;
    bool  inputOrOutput;
  };

private:
  enum NavigateMode {
    IDLE,
    PAN,
    ZOOM
  };

public:
  GraphEditorUI(View* parent);
  ~GraphEditorUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void MouseWheel(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void Input(int key) override;
  bool Draw() override;
  void DrawGrid();

  inline float GetScale() const { return _scale; };
  inline const pxr::GfVec2f& GetOffset() const { return _offset; };

  void Init();
  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  void Term();

  // font
  void UpdateFont();
  inline size_t GetFontIndex() { return _fontIndex; };
  inline float GetFontScale() { return _fontScale; };

  // conversion
  pxr::GfVec2f ViewPositionToGridPosition(const pxr::GfVec2f& mousePos);
  pxr::GfVec2f GridPositionToViewPosition(const pxr::GfVec2f& gridPos);

  // graph
  Graph* GetGraph() { return _graph; };
  
  // nodes
  void AddNode(Node* node) { _graph->AddNode(node); };
  Node* GetLastNode() { return _graph->GetNodes().back(); };

  // connexion
  void StartConnexion();
  void UpdateConnexion();
  void EndConnexion();

  // selection
  void AddToSelection(Node* node, bool bringToFront);
  void RemoveFromSelection(Node* node);
  void AddToSelection(Connexion* connexion);
  void RemoveFromSelection(Connexion* connexion);
  void ClearSelection();
  void MarqueeSelect(int mod);
  std::set<Node*>& GetSelectedNodes() { return _selectedNodes; };
  const std::set<Node*>& GetSelectedNodes() const { return _selectedNodes; };
  pxr::SdfPathVector GetSelectedNodesPath();

  // display
  void ResetScaleOffset();
  void FrameSelection();
  void FrameAll();

  // io
  bool Populate(pxr::UsdPrim& prim);
  void Update();
  void Clear();
  bool Read(const std::string& filename);
  bool Write(const std::string& filename);

  void BuildGrid();
  void BuildGraph();

  // notices
  void OnAttributeChangedNotice(const AttributeChangedNotice& n) override;
  void OnSceneChangedNotice(const SceneChangedNotice& n) override;
  void OnNewSceneNotice(const NewSceneNotice& n) override;
  
private:
  void _GetPortUnderMouse(const pxr::GfVec2f& mousePos, Node* node);
  void _GetNodeUnderMouse(const pxr::GfVec2f& mousePos, bool useExtend = false);
  void _GetConnexionUnderMouse(const pxr::GfVec2f& mousePos);
  void _RecurseStagePrim(const pxr::UsdPrim& prim, const pxr::SdfPath& skipPath);
  bool _ConnexionPossible(const Port* lhs, const Port* rhs);
  
  int                                   _id;
  int                                   _depth;
  pxr::GfVec2f                          _offset;  
  pxr::GfVec2f                          _dragOffset;
  float                                 _scale;
  float                                 _invScale;
  float                                 _currentX;
  float                                 _currentY;
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
  Graph*                                _graph;
  std::set<Node*>                       _selectedNodes;
  std::set<Connexion*>                  _selectedConnexions;
  Node*                                 _hoveredNode;
  Node*                                 _currentNode;
  Port*                                 _hoveredPort;
  Port*                                 _currentPort;
  Connexion*                            _hoveredConnexion;
  short                                 _inputOrOutput;

  Marquee                               _marquee;
  Cell*                                 _grid;
  Connect                               _connector;

  static ImGuiWindowFlags               _flags;
  Selection                             _selection;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_GRAPH_H