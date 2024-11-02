
#include <iostream>
#include <array>
#include <utility>
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_internal.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>

#include "../ui/attributeEditor.h"
#include "../app/view.h"
#include "../app/model.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags AttributesUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;


AttributesUI::AttributesUI(View* parent)
  : BaseUI(parent, UIType::DEMO)
{
}

AttributesUI::~AttributesUI()
{
}

bool 
AttributesUI::Draw()
{
  static bool opened = false;
  const GfVec2f pos(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);

  SdfPath primPath = _GetPrimToDisplay();

  if (primPath.IsEmpty()) return false;

  _AppendDisplayColorAttr(primPath);
  _AppendAllPrimAttrs(primPath);

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};

SdfPath 
AttributesUI::_GetPrimToDisplay()
{
  SdfPathVector primPaths = _model->GetSelection()->GetSelectedPaths();

  if (primPaths.size() > 0 && !primPaths[0].IsEmpty())
    _path = primPaths[0];

  return _path;
}

void 
AttributesUI::_AppendDisplayColorAttr(SdfPath primPath)
{

  // save the values before change
  GfVec3f color(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);

  float* data = color.data();
  if (ImGui::CollapsingHeader("Display Color"))
    ImGui::SliderFloat3("", data, 0, 1);

}

void 
AttributesUI::_AppendDataSourceAttrs(
  HdContainerDataSourceHandle containerDataSource)
{
  for (auto&& token : containerDataSource->GetNames()) {
    auto dataSource = containerDataSource->Get(token);
    const char* tokenText = token.GetText();

    auto containerDataSource =
        HdContainerDataSource::Cast(dataSource);
    if (containerDataSource) {
      bool clicked =
        ImGui::TreeNodeEx(tokenText, ImGuiTreeNodeFlags_OpenOnArrow);

      if (clicked) {
        _AppendDataSourceAttrs(containerDataSource);
        ImGui::TreePop();
      }
    }

    auto sampledDataSource = HdSampledDataSource::Cast(dataSource);
    if (sampledDataSource) {
      ImGui::Columns(2);
      VtValue value = sampledDataSource->GetValue(0);
      ImGui::Text("%s", tokenText);
      ImGui::NextColumn();
      ImGui::BeginChild(tokenText, ImVec2(0, 14), false);
      std::stringstream ss;
      ss << value;
      ImGui::Text("%s", ss.str().c_str());
      ImGui::EndChild();
      ImGui::Columns();
    }
  }
}

void AttributesUI::_AppendAllPrimAttrs(SdfPath primPath)
{
  
  HdSceneIndexPrim prim = _model->GetEditableSceneIndex()->GetPrim(primPath);
  TfTokenVector tokens = prim.dataSource->GetNames();

  if (tokens.size() < 1) return;

  if (ImGui::CollapsingHeader("Prim attributes")) {
      _AppendDataSourceAttrs(prim.dataSource);
  }
}

  

JVR_NAMESPACE_CLOSE_SCOPE