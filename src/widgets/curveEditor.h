#ifndef AMN_WIDGETS_CURVEEDITOR_H
#define AMN_WIDGETS_CURVEEDITOR_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usdAnimX/data.h>
#include <pxr/usd/usdAnimX/curve.h>
#include <pxr/usd/usdAnimX/keyframe.h>
#include <vector>

AMN_NAMESPACE_OPEN_SCOPE

struct CurveEditorGrabUI {
  pxr::GfVec2f start;
  pxr::GfVec2f end;
};

typedef std::vector<pxr::UsdAnimXCurve*> AnimXCurves;
typedef pxr::TfHashMap <pxr::SdfPath, AnimXCurves, pxr::SdfPath::Hash> AnimXCurvesMap;

class CurveEditorUI : public BaseUI
{
friend pxr::UsdAnimXData;
public:
  CurveEditorUI(View* parent);
  ~CurveEditorUI()         override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void MouseWheel(int x, int y) override;
  void Init();
  void Update();
  bool Draw() override;

  void DrawBackground();
  void DrawCurve(pxr::UsdAnimXCurve* crv);
  void DrawCurves();

  void SetLayer(pxr::SdfLayerHandle& layer);
  void SetDatas(const pxr::UsdAnimXDataRefPtr& datas);
  void PopulateCurves();
  /*
  void RecurseStage();
  void RecursePrim(ExplorerItem* current);
  void DrawItem(ExplorerItem* item, bool heritedVisibility);
  void DrawItemType(ExplorerItem* item);
  void DrawItemVisibility(ExplorerItem* item, bool heritedVisibility);
  void DrawBackground();
  void DrawItemBackground(ImDrawList* drawList, const ExplorerItem* item, bool& flip);
  */

private:
  AnimXCurvesMap                _curves;
  pxr::SdfLayerHandle           _layer;
  pxr::UsdAnimXDataRefPtr       _datas;

  pxr::GfVec2f                  _offset;
  pxr::GfVec2f                  _scale;
  pxr::GfVec2f                  _invScale;
  float                         _lastX;
  float                         _lastY;
  bool                          _drag;
  bool                          _grab;
  bool                          _navigate;
  CurveEditorGrabUI             _grabData;
  /*
  pxr::GfVec3f                  _color;
  bool                          _locked;
  ExplorerItem*                 _root;
  ImGuiTreeNodeFlags            _selectBaseFlags;
  Icon*                         _visibleIcon;
  Icon*                         _invisibleIcon;
  ImVec4                        _backgroundColor;
  ImVec4                        _alternateColor;
  ImVec4                        _selectedColor;
  ImVec4                        _hoveredColor;
  bool                          _needRefresh;
  */
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGETS_CURVEEDITOR_H