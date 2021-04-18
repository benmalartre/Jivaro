#include "curveEditor.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/window.h"
#include "../app/view.h"

#include <pxr/usd/usdAnimX/fileFormat.h>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
CurveEditorUI::CurveEditorUI(View* parent)
  : BaseUI(parent, "CurveEditor")
  , _offset(0,0)
  , _scale(1,1)
  , _invScale(1,1)
  , _navigate(false)
  , _grab(false)
{
  _flags = ImGuiWindowFlags_None
    | ImGuiWindowFlags_NoResize
    | ImGuiWindowFlags_NoTitleBar
    | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoMove;

  _parent->SetDirty();
}

// destructor
CurveEditorUI::~CurveEditorUI()
{
}

void CurveEditorUI::Init()
{
  Update();
  _parent->SetDirty();
  _initialized = true;
}

void CurveEditorUI::Update()
{
  if (GetApplication()->GetStage()) {
    //RecurseStage();
  }
}

void CurveEditorUI::MouseButton(int button, int action, int mods)
{
  const pxr::GfVec2f& mousePos = ImGui::GetMousePos();
  _lastX = mousePos[0];
  _lastY = mousePos[1];

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if (mods & GLFW_MOD_ALT)_navigate = true;
      /*
      else if (_hoveredPort) {
        _connect = true;
        StartConnexion();
      }
      else if (_hoveredNode) {
        if (_hoveredNode->GetState(ITEM_STATE_SELECTED))
          _drag = true;
        else {
          bool state = _hoveredNode->GetState(ITEM_STATE_SELECTED);
          _hoveredNode->SetState(ITEM_STATE_SELECTED, 1 - state);
          if (mods & GLFW_MOD_CONTROL) {
            if (1 - state) {
              AddToSelection(_hoveredNode, false);
              _drag = true;
            }
            else RemoveFromSelection(_hoveredNode);
          }
          else if (mods & GLFW_MOD_SHIFT) {
            AddToSelection(_hoveredNode, false);
            _drag = true;
          }
          else {
            ClearSelection();
            if (1 - state) {
              AddToSelection(_hoveredNode, true);
              _drag = true;
            }
          }
        }
      }*/
      else {
        if (_parent->GetFlag(View::ACTIVE)) {
          pxr::GfVec2f viewPos;
          GetRelativeMousePosition(mousePos[0], mousePos[1], _grabData.start[0], _grabData.start[1]);
          _grabData.end = _grabData.start;
          _grab = true;
        }
      }
    }
    else if (action == GLFW_RELEASE) {
      _navigate = false;
      _drag = false;
      /*
      if (_connect) EndConnexion();
      else if (_grab)GrabSelect(mods);
      */
      _grab = false;
    }
  }
  _parent->SetDirty();
}

void CurveEditorUI::MouseMove(int x, int y)
{
  if (_navigate) {
    _offset += pxr::GfVec2f(
      (x - _lastX) * _invScale[0], 
      (y - _lastY) * _invScale[1]);
    _parent->SetDirty();
  }
  /*
  else if (_drag) {
    for (auto& node : _selected) {
      node->Move(pxr::GfVec2f(x - _lastX, y - _lastY) / _scale);
    }
    _parent->SetDirty();
  }*/
  else if (_grab) {
    GetRelativeMousePosition(x, y, _grabData.end[0], _grabData.end[1]);
    _parent->SetDirty();
  }
  /*
  else {
    _GetNodeUnderMouse(pxr::GfVec2f(x, y), true);
    if (_connect && _hoveredPort)UpdateConnexion();
    if (_hoveredPort)_parent->SetDirty();
  }*/
  _lastX = x;
  _lastY = y;
}

void CurveEditorUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    pxr::SdfPath path = pxr::SdfPath("/Cube").AppendProperty(pxr::TfToken("size"));
    AnimXCurves *curves = pxr::TfMapLookupPtr(_curves, path);
    pxr::UsdAnimXCurve *curve = (*(curves))[0];
    pxr::UsdAnimXKeyframe keyframe;
    curve->keyframeAtIndex(1, keyframe);
    if(key == GLFW_KEY_K)keyframe.value += 2;
    else if (key == GLFW_KEY_L)keyframe.value -= 2;
    curve->setKeyframeAtIndex(1, *(adsk::Keyframe*)&keyframe);
   
    if (_layer) {
      _layer->SetField(
        path,
        pxr::SdfFieldKeys->Default,
        pxr::VtValue(keyframe.value));
    }
   
    GetWindow()->ForceRedraw();
  }
}

void CurveEditorUI::MouseWheel(int x, int y)
{
  pxr::GfVec2f mousePos = 
    (ImGui::GetMousePos() - _parent->GetMin());
  pxr::GfVec2f originalPos(
    mousePos[0] / _scale[0], 
    mousePos[1] / _scale[1]);

  _scale += pxr::GfVec2f(y * 1.f, y * 1.f);
  _scale[0] = CLAMP(_scale[0], 0.01f, 100.f);
  _scale[1] = CLAMP(_scale[1], 0.01f, 100.f);
  _invScale[0] = 1.f / _scale[0];
  _invScale[1] = 1.f / _scale[1];
  pxr::GfVec2f scaledPos(
    mousePos[0] / _scale[0],
    mousePos[1] / _scale[1]);
  _offset -= (originalPos - scaledPos);
  //UpdateFont();
  _parent->SetDirty();

}

void CurveEditorUI::SetLayer(pxr::SdfLayerHandle layer)
{
  std::cout << "### CURVE EDITOR UI : SET LAYER!!!" << std::endl;
  _layer = layer;
  pxr::UsdAnimXFileFormatConstPtr fileFormat =
    pxr::TfStatic_cast<pxr::UsdAnimXFileFormatConstPtr>(layer->GetFileFormat());
  pxr::SdfAbstractDataConstPtr datas = fileFormat->GetData(&(*layer));
  pxr::UsdAnimXDataPtr animXDatas =
    pxr::TfDynamic_cast<pxr::UsdAnimXDataPtr>(
      pxr::TfConst_cast<pxr::SdfAbstractDataPtr>(datas));
  std::cout << "[ANIMX] DATAS : " << animXDatas->GetCurrentCount() << std::endl;
  SetDatas(animXDatas);
  PopulateCurves();
  std::cout << "##############################################" << std::endl;
  std::cout << "###   " << _layer.GetUniqueIdentifier() << std::endl;
  std::cout << "##############################################" << std::endl;
}

void CurveEditorUI::SetDatas(const pxr::UsdAnimXDataRefPtr& datas)
{
  _curves.clear();
  _datas = datas;
}

void CurveEditorUI::PopulateCurves()
{
  if (_datas) {
    std::vector<pxr::SdfPath> animatedPrims;
    _datas->GetAnimatedPrims(animatedPrims);
    for (auto& animatedPrim : animatedPrims) {
      _datas->GetCurves(animatedPrim, _curves);
    }
  }
}

void CurveEditorUI::DrawBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(0, 255, 255, 255));
}

void CurveEditorUI::DrawCurve(pxr::UsdAnimXCurve* crv)
{

}

void CurveEditorUI::DrawCurves()
{
  pxr::UsdAnimXKeyframe keyframe;
  pxr::UsdAnimXKeyframe lastKeyframe;
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  float ox = ImGui::GetWindowPos().x + _offset[0] * _scale[0];
  float oy = ImGui::GetWindowPos().y + _offset[1] * _scale[1];
  static ImU32 red = ImColor(255, 64, 32);
  static ImU32 green = ImColor(32, 255, 64);
  static ImU32 blue = ImColor(64, 32, 255);
  size_t cid = 0;

  /*
  adsk::evaluateCurveSegment(interpolationMethod,
    CurveInterpolatorMethod curveInterpolatorMethod,
    double time,
    double startX, double startY,
    double x1, double y1,
    double x2, double y2,
    double endX, double endY);*/

  
  for (auto& it : _curves) {
    const std::vector<pxr::UsdAnimXCurve*>& curves = it.second;
    cid = 0;
    for (const auto& curve: curves) {
      // draw curve
      for (size_t k = 0; k < curve->keyframeCount(); ++k) {
        ImU32 col = red;
        if (cid % 3 == 1)col = green;
        else if (cid % 3 == 2)col = blue;
        curve->keyframeAtIndex(k, keyframe);

        if (k > 0) {
          const ImVec2 A(ox + lastKeyframe.time * _scale[0],
            oy + lastKeyframe.value * _scale[1]);
          const ImVec2 B(A.x + lastKeyframe.tanOut.x * _scale[0],
            A.y + lastKeyframe.tanOut.y * _scale[1]);
          const ImVec2 D(ox + keyframe.time * _scale[0],
            oy + keyframe.value * _scale[1]);
          const ImVec2 C(D.x - keyframe.tanIn.x * _scale[0],
            D.y - keyframe.tanIn.y * _scale[1]);
          drawList->AddBezierCurve(A, B, C, D, col, 1);
        }
        lastKeyframe = keyframe;
      }
      // draw keyframes
      for (size_t k = 0; k < curve->keyframeCount(); ++k) {
        curve->keyframeAtIndex(k, keyframe);

        drawList->AddCircleFilled(
          ImVec2(
            ox + keyframe.time * _scale[0],
            oy + keyframe.value * _scale[1]),
          2,
          ImColor(60, 60, 60));
      }
    }
    cid++;
  }
}

bool CurveEditorUI::Draw()
{
  if (!_initialized)Init();
  
  //if (!_active)return false;
  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::PushClipRect(_parent->GetMin(), _parent->GetMax(), false);
  Application* app = GetApplication();
  /*
  if (app->GetStage())
  {
    // setup colors
    const size_t numColorIDs = 7;
    int colorIDs[numColorIDs] = {
      ImGuiCol_WindowBg,
      ImGuiCol_Header,
      ImGuiCol_HeaderHovered,
      ImGuiCol_HeaderActive,
      ImGuiCol_Button,
      ImGuiCol_ButtonActive,
      ImGuiCol_ButtonHovered
    };
    for (int i = 0; i<numColorIDs; ++i)
      ImGui::PushStyleColor(
        colorIDs[i],
        ImVec4(0, 0, 0, 0));
    
    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, GetWidth() - 100);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 40);
    
    // draw title
    ImGui::PushFont(GetWindow()->GetBoldFont(0));
    ImGui::Text("Prim");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopFont();
    
    DrawBackground();
    
    ImGui::PushFont(GetWindow()->GetMediumFont(0));
    //DrawItem(_root, true);
    ImGui::PopFont();
    
    ImGui::PopStyleColor(numColorIDs);
  }
  
  ImGui::End();

  return
    ImGui::IsAnyItemHovered() ||
    ImGui::IsAnyItemActive() ||
    ImGui::IsAnyItemFocused() ||
    ImGui::IsAnyMouseDown();
  */
  DrawBackground();
  DrawCurves();
  ImGui::PopClipRect();
  ImGui::End();
  return true;
}

AMN_NAMESPACE_CLOSE_SCOPE