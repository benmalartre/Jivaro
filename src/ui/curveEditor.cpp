#include "../ui/curveEditor.h"
#include "../app/application.h"
#include "../app/notice.h"
#include "../app/window.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags CurveEditorUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove;

// constructor
CurveEditorUI::CurveEditorUI(View* parent)
  : BaseUI(parent, UIType::CURVEEDITOR)
  , _offset(0,0)
  , _scale(1,1)
  , _invScale(1,1)
  , _navigate(false)
  , _grab(false)
{
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
  if (Application::Get()->GetWorkStage()) {
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
  const pxr::GfVec2f min(GetX(), GetY());
  pxr::GfVec2f mousePos = 
    (ImGui::GetMousePos() - min);
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
}

void CurveEditorUI::DrawCurve(pxr::UsdAnimXCurve* crv)
{

}

void CurveEditorUI::DrawTime()
{
  Time& time = *Time::Get();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  float ox = _parent->GetX() + _offset[0] * _scale[0];
  float oy = _parent->GetY() + _offset[1] * _scale[1];
  static ImU32 orange = ImColor(255, 180, 120);
  
  drawList->AddRectFilled(
    ImVec2(ox + time.GetActiveTime() * _scale[0], _parent->GetY()),
    ImVec2(ox + time.GetActiveTime() * _scale[0] + 4, _parent->GetY() + _parent->GetHeight()),
    orange);

}

float CurveEditorUI::_GetTimeMinimum()
{
  return -_offset[0] * _scale[0];
}
float CurveEditorUI::_GetTimeMaximum()
{
  return (-_offset[0] + _parent->GetWidth()) * _scale[0];
}

float CurveEditorUI::_GetTimeStep(float width, float scale)
{
  return (float)KEYFRAME_SAMPLE_SIZE / width;
}

size_t CurveEditorUI::_GetNumSamples(float width)
{
  return (width / KEYFRAME_SAMPLE_SIZE);
}


void CurveEditorUI::DrawCurves()
{
  pxr::UsdAnimXKeyframe keyframe;
  pxr::UsdAnimXKeyframe lastKeyframe;
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  float ox = _parent->GetX() + _offset[0] * _scale[0];
  float oy = _parent->GetY() + _offset[1] * _scale[1];
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
      const float minimum = _GetTimeMinimum();
      const float maximum = _GetTimeMaximum();
      drawList->AddRectFilled(
        ImVec2((minimum + _offset[0]) * _scale[0] - 2, GetY()),
        ImVec2((minimum + _offset[0]) * _scale[0] + 2, GetY() + GetHeight()),
        ImColor(0, 255, 0)
      );
      drawList->AddRectFilled(
        ImVec2((maximum + _offset[0]) * _scale[0] - 2, GetY()),
        ImVec2((maximum + _offset[0]) * _scale[0] + 2, GetY() + GetHeight()),
        ImColor(0, 255, 0)
      );
      size_t numSamples = _GetNumSamples(GetWidth());
      float sampleStep = (maximum - minimum) / numSamples;
      for (size_t k = 0; k < numSamples - 1; ++k) {
        ImU32 col = red;
        if (cid % 3 == 1)col = green;
        else if (cid % 3 == 2)col = blue;
        float t0 = minimum + (k + 0.000) * sampleStep;
        float t1 = minimum + (k + 0.333) * sampleStep;
        float t2 = minimum + (k + 0.666) * sampleStep;
        float t3 = minimum + (k + 1.000) * sampleStep;

        const ImVec2 A(t0 * _scale[0],
          oy - curve->evaluate(t0) * _scale[1]);
        const ImVec2 B(t1 * _scale[0],
          oy - curve->evaluate(t1) * _scale[1]);
        const ImVec2 C(t2 * _scale[0],
          oy - curve->evaluate(t2) * _scale[1]);
        const ImVec2 D(t3 * _scale[0],
          oy - curve->evaluate(t3) * _scale[1]);
        drawList->AddBezierCurve(A, B, C, D, col, 1);
      }
      // draw keyframes
      for (size_t k = 0; k < curve->keyframeCount(); ++k) {
        curve->keyframeAtIndex(k, keyframe);

        drawList->AddCircleFilled(
          ImVec2(
            ox + keyframe.time * _scale[0],
            oy + keyframe.value * _scale[1]),
          2,
          ImColor(255, 255, 255));
      }
    }
    cid++;
  }
}

bool CurveEditorUI::Draw()
{
  if (!_initialized)Init();

  const pxr::GfVec2f min(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  const pxr::GfVec2f max(min + size);
  
  //if (!_active)return false;
  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_name.c_str(), NULL, _flags);
  
  ImGui::PushClipRect(min, max, false);
  Application* app = Application::Get();
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
  DrawTime();
  ImGui::PopClipRect();
  ImGui::End();
  return false;
}

JVR_NAMESPACE_CLOSE_SCOPE