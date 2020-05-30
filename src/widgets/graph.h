#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/ui.h"
#include "../utils/utils.h"
#include "node.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primFlags.h>
#include <vector>
#include <set>

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

struct GraphCellUI {
  bool                                 isLeaf;
  std::vector<NodeUI*>                 nodes;
  GraphCellUI*                         cells[4];

  GraphCellUI():isLeaf(true) {
    for (int i = 0; i < 4; ++i) cells[i] = NULL;
  };
};

struct GraphGrabUI {
  pxr::GfVec2f start;
  pxr::GfVec2f end;
};

struct GraphConnectUI {
  PortUI* startPort;
  PortUI* endPort;
  int     color;
};

class GraphUI : public BaseUI
{
public:
  GraphUI(View* parent, const std::string& filename, bool docked);
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
  void AddNode(NodeUI* node) { _nodes.push_back(node); };
  NodeUI* GetLastNode() { return _nodes.back(); };

  // connection
  void StartConnexion();
  void UpdateConnexion();
  void EndConnexion();

  // selection
  void AddToSelection(NodeUI* node, bool bringToFront);
  void RemoveFromSelection(NodeUI* node);
  void ClearSelection();
  void GrabSelect(int mod);

  void BuildGrid();
  void BuildGraph();
  
private:
  void _GetPortUnderMouse(const pxr::GfVec2f& mousePos, NodeUI* node);
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
  bool                                  _grab;
  bool                                  _connect;
  bool                                  _navigate;
  float                                 _fontScale;
  size_t                                _fontIndex;

  int                                   _nodeId;
  pxr::UsdStageRefPtr                   _stage;
  std::vector<NodeUI*>                  _nodes;
  std::vector<ConnexionUI*>             _connexions;
  std::set<NodeUI*>                     _selected;
  NodeUI*                               _hoveredNode;
  NodeUI*                               _currentNode;
  PortUI*                               _hoveredPort;
  PortUI*                               _currentPort;
  ConnexionUI*                          _hoveredConnexion;

  GraphGrabUI                           _grabData;
  GraphCellUI*                          _grid;
  GraphConnectUI                        _connector;
};

AMN_NAMESPACE_CLOSE_SCOPE