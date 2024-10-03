#ifndef JVR_UI_CURVEEDITOR_H
#define JVR_UI_CURVEEDITOR_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <vector>

JVR_NAMESPACE_OPEN_SCOPE

#define KEYFRAME_SAMPLE_SIZE 32

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
  ~CurveEditorUI() override;

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
  void DrawTime();

  void SetLayer(pxr::SdfLayerHandle layer);
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
  float _GetTimeMinimum();
  float _GetTimeMaximum();
  float _GetTimeStep(float width, float scale);
  size_t _GetNumSamples(float width);

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
  static ImGuiWindowFlags       _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_CURVEEDITOR_H