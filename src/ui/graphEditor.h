#ifndef JVR_UI_GRAPH_H
#define JVR_UI_GRAPH_H

#include <vector>
#include <set>

#include "../common.h"
#include "../utils/icons.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../graph/graph.h"
#include "../app/selection.h"

#include <pxr/base/tf/type.h>
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

static int _GetColorFromAttribute(const UsdAttribute& attr);

class Selection;

class GraphEditorUI : public BaseUI
{
public:
  static const TfToken NodeExpendState[3];

  enum GraphDirection {
    VERTICAL,
    HORIZONTAL,
  };

  enum ItemState {
    ITEM_STATE_NONE = 0,
    ITEM_STATE_HOVERED = 1,
    ITEM_STATE_SELECTED = 2,
    ITEM_STATE_ERROR = 4,
    ITEM_STATE_DISABLED = 8
  };

  enum ExpendedState {
    COLLAPSED,
    CONNECTED,
    EXPENDED
  };

protected:
  class Node;
  class Port;
  class Connexion;

  // Graph item base class
  //-------------------------------------------------------------------
  class Item {
    public:
      Item();
      Item(const GfVec2f& pos, 
        const GfVec2f& size, int color);
      Item(int color);

      virtual void SetPosition(const GfVec2f& pos);
      virtual void SetSize(const GfVec2f& size);
      virtual void SetColor(const GfVec3f& color);
      void SetColor(int color);
      virtual const GfVec2f& GetPosition() const { return _pos; };
      virtual const GfVec2f& GetSize() const { return _size; };
      virtual float GetWidth() const { return _size[0]; };
      virtual float GetHeight() const { return _size[1]; };
      virtual float GetX() const { return _pos[0]; };
      virtual float GetY() const { return _pos[1]; };
      virtual int GetColor() const { return _color; };

      void SetState(size_t flag, bool value);
      bool GetState(size_t flag);
      virtual bool Contains(const GfVec2f& position, 
        const GfVec2f& extend = GfVec2f(0,0));
      virtual bool Intersect(const GfVec2f& start, 
        const GfVec2f& end);
      virtual bool IsVisible(GraphEditorUI* editor) = 0;
      virtual void Draw(GraphEditorUI* editor) = 0;

    protected:
      GfVec2f _pos;
      GfVec2f _size;
      int          _color;
      short        _state;
    };

  // Graph port class
  //-------------------------------------------------------------------
  class Port : public Item {
    public:
      Port() {};
      Port(Node* node, Graph::Port* port);

      bool Contains(const GfVec2f& position,
        const GfVec2f& extend = GfVec2f(0, 0)) override;

      bool IsVisible(GraphEditorUI* editor) override { return true; };
      bool IsConnected(GraphEditorUI* editor, Graph::Connexion* connexion=NULL);
      void Draw(GraphEditorUI* editor) override;
      Graph::Port* Get() { return _port; };
      Node* GetNode() { return _node; };
    private:
      // ui
      Node*                 _node;

      // data
      Graph::Port*          _port;

  };

  struct ConnexionData
  {
    GfVec2f  p0, p1, p2, p3;
    int           numSegments;
  };

  // Graph connexion class
  //-------------------------------------------------------------------
  class Connexion : public Item {
    public:
      Connexion(Port* start, Port* end, Graph::Connexion* connexion, int color)
        : Item(color) 
        , _start(start)
        , _end(end)
        , _connexion(connexion) {};

      bool IsVisible(GraphEditorUI* editor) override { return true; };
      void Draw(GraphEditorUI* editor) override;
      inline ConnexionData GetDescription();

      virtual bool Contains(const GfVec2f& position,
        const GfVec2f& extend = GfVec2f(0, 0)) override;
      virtual bool Intersect(const GfVec2f& start,
        const GfVec2f& end) override;

      GfRange2f GetBoundingBox();
      Graph::Connexion* Get() { return _connexion; };
      Port* GetStart() { return _start; };
      Port* GetEnd() { return _end; };

  private:
    // ui
    Port*                           _start;
    Port*                           _end;

    // data
    Graph::Connexion*               _connexion;
  };

  // Graph node class
  //-------------------------------------------------------------------
  class Node : public Item {
    public: 
      enum {
        DIRTY_CLEAN = 0,
        DIRTY_SIZE = 1,
        DIRTY_POSITION = 2,
        DIRTY_COLOR = 4
      };
      Node(Graph::Node* node);
      ~Node();

      void SetColor(const GfVec3f& color) override;
      bool IsVisible(GraphEditorUI* editor) override;
      void Draw(GraphEditorUI* graph) override;
      void ComputeSize(GraphEditorUI* editor);

      std::vector<Port>& GetPorts() { return _ports; };
      Port* GetPort(const TfToken& name);
      Graph::Node* Get() { return _node; };
      TfToken& GetExpended() { return _expended; };
      short GetDirty() { return _dirty; };
      void SetDirty(short dirty) { _dirty = dirty; };

      int GetColor() const override;

      void Write();
      void Read();

    private:
      // ui
      std::vector<Port>           _ports;
      Node*                       _parent;
      TfToken                     _expended;
      short                       _dirty;

      // data
      Graph::Node*                _node;
  };

  struct Marquee {
    GfVec2f start;
    GfVec2f end;
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
  inline const GfVec2f& GetOffset() const { return _offset; };

  void Init();
  void Init(const std::vector<UsdStageRefPtr>& stages);
  void Term();

  // font
  void UpdateFont();
  inline size_t GetFontIndex() { return _fontIndex; };
  inline float GetFontScale() { return _fontScale; };

  // conversion
  GfVec2f ViewPositionToGridPosition(const GfVec2f& mousePos);
  GfVec2f GridPositionToViewPosition(const GfVec2f& gridPos);

  // graph
  Graph* GetGraph() { return _graph; };
  
  // nodes
  void AddNode(Graph::Node* node) { _graph->AddNode(node); };

  // port
  Port* GetPort(Graph::Port* port);

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
  SdfPathVector GetSelectedNodesPath();

  // display
  void ResetScaleOffset();
  void FrameSelection();
  void FrameAll();

  // io
  bool Populate(Graph* graph);
  void Write();
  void Read();
  void Clear();
  bool Read(const std::string& filename);
  bool Write(const std::string& filename);

  // notices
  void OnAttributeChangedNotice(const AttributeChangedNotice& n) override;
  void OnSceneChangedNotice(const SceneChangedNotice& n) override;
  void OnNewSceneNotice(const NewSceneNotice& n) override;
  
private:
  void _GetPortUnderMouse(const GfVec2f& mousePos, Node* node);
  void _GetNodeUnderMouse(const GfVec2f& mousePos, bool useExtend = false);
  void _GetConnexionUnderMouse(const GfVec2f& mousePos);
  
  int                                   _id;
  int                                   _depth;
  GfVec2f                               _offset;  
  GfVec2f                               _dragOffset;
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
  uint64_t                              _lastClick;

  int                                   _nodeId;
  UsdStageRefPtr                        _stage;
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
  Connect                               _connector;

  static ImGuiWindowFlags               _flags;
  Selection                             _selection;

  std::vector<Node*>                    _nodes;
  std::vector<Connexion*>               _connexions;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_GRAPH_H