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
  GRAPH_COLOR_PRIM              = 0xFF6622FF
};

struct GraphPortMapData {
  const NodeUI*                        _node;
  std::string                          _port;
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

class GraphUI : public BaseUI
{
public:
  GraphUI(View* parent, const std::string& filename, bool docked);
  ~GraphUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void MouseWheel(int x, int y) override;
  void Draw() override;
  void DrawGrid();
  void DrawTxt();

  inline float GetZoom() { return _zoom; };
  inline const pxr::GfVec2f& GetOffset() { return _offset; };

  void Init(const std::string& filename);
  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  void Term();

  void SetCurrentColor(int color){_color = color;};
  int GetCurrentColor(){return _color;};
  
  void BuildGraph(int index);

  void UpdateFont();
  inline size_t GetFontIndex() { return _fontIndex; };
  inline float GetFontScale() { return _fontScale; };

  pxr::GfVec2f ViewPositionToGridPosition(const pxr::GfVec2f& mousePos);
  NodeUI* NodeUnderMouse(const pxr::GfVec2f& mousePos);

  void AddNode(NodeUI* node) { _nodes.push_back(node); };
  NodeUI* GetLastNode() { return _nodes.back(); };
  NodeUI* GetDrawingNode() { return _drawingNode; };
  const pxr::GfVec2f GetDrawingPos() { 
    return GetPosition() + (_drawingPos + _offset) * _zoom; 
  };
  const pxr::GfVec2f GetCursorPos() {
    return (_drawingPos + _offset) * _zoom;
  };
  void SetDrawingPos(const pxr::GfVec2f& pos) { _drawingPos = pos; };
  void IncrementDrawingPos(const pxr::GfVec2f& offset) { _drawingPos += offset; };

  void AddToSelection(NodeUI* node, bool bringToFront);
  void RemoveFromSelection(NodeUI* node);
  void ClearSelection();
  void GrabSelect(int mod);

  void BuildGrid();
  
  void BuildGraph();
  
private:
  void _RecurseStagePrim(const pxr::UsdPrim& prim);
  
  int                                   _color;
  int                                   _id;
  int                                   _depth;
  pxr::GfVec2f                          _offset;  
  float                                 _zoom;
  int                                   _lastX;
  int                                   _lastY;
  bool                                  _drag;
  bool                                  _grab;
  bool                                  _navigate;
  float                                 _fontScale;
  size_t                                _fontIndex;

  int                                   _nodeId;
  pxr::UsdStageRefPtr                   _stage;
  std::vector<NodeUI*>                  _nodes;
  std::vector<ConnexionUI>              _connexions;
  std::set<NodeUI*>                     _selected;
  NodeUI*                               _hoveredNode;
  NodeUI*                               _currentNode;
  NodeUI*                               _drawingNode;
  
  pxr::GfVec2f                          _drawingPos;
  GraphGrabUI                           _grabData;
  GraphCellUI*                          _grid;
};

AMN_NAMESPACE_CLOSE_SCOPE