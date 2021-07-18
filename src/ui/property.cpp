#include "property.h"
#include "../app/view.h"
#include "../app/window.h"

#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/curves.h>

AMN_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PropertyUI::_flags = 
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;

PropertyUI::PropertyUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
{
}

PropertyUI::~PropertyUI()
{
}

void PropertyUI::SetPrim(const pxr::UsdPrim& prim)
{
  if(prim.IsValid())_prim = prim;
  _initialized = false;
}

bool PropertyUI::Draw()
{
  if (!_prim)return false;
  if (!_initialized)_initialized = true;

  if (_prim.IsA<pxr::UsdGeomXform>()) {
    pxr::UsdGeomXform xfo(_prim);
    pxr::TfTokenVector attrNames = xfo.GetSchemaAttributeNames();
  }
  /*
  std::cout << "PRIM : " << _prim.GetName() << std::endl;
  pxr::UsdSchemaBase base(_prim);
  std::cout << "BASE : " << base.GetPath() << std::endl;
  pxr::TfTokenVector attrNames = base.GetSchemaAttributeNames();
  */

  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
    ImGuiCol_WindowBg
  );

  static pxr::GfVec3f position;
  static pxr::GfVec3f rotation;
  static pxr::GfVec3f scale(1.f, 1.f, 1.f);
  static bool visible;
  static pxr::GfVec3f color;
  static float opacity;

  Window* window = _parent->GetWindow();

  const float valueWidth = GetWidth() - 100;
  ImGui::PushFont(window->GetBoldFont(0));
  if (ImGui::BeginTabBar("Tab")) {

    if (ImGui::BeginTabItem("Xform")) {
      // setup columns
      ImGui::Columns(2);
      ImGui::SetColumnWidth(0, 100);
      ImGui::SetColumnWidth(1, GetWidth() - 100);

      // draw title
      ImGui::PushFont(GetWindow()->GetMediumFont(0));
      ImGui::Text("Attribute");
      ImGui::NextColumn();
      ImGui::Text("Value");
      ImGui::NextColumn();
      ImGui::PopFont();

      ImGui::PushFont(window->GetRegularFont(0));
      ImGui::Text("Position");                            ImGui::NextColumn();
      ImGui::SetNextItemWidth(valueWidth);
      ImGui::InputFloat3("##Position", &position[0], 3);  ImGui::NextColumn();
      ImGui::Text("Rotation");                            ImGui::NextColumn();
      ImGui::SetNextItemWidth(valueWidth);
      ImGui::InputFloat3("##Rotation", &rotation[0], 3);  ImGui::NextColumn();
      ImGui::Text("Scale");                               ImGui::NextColumn();
      ImGui::SetNextItemWidth(valueWidth);
      ImGui::InputFloat3("##Scale", &scale[0],3);         ImGui::NextColumn();
      ImGui::EndTabItem();
      ImGui::PopFont();
    }
    if (ImGui::BeginTabItem("Display")) {
      ImGui::PushFont(window->GetRegularFont(0));
      ImGui::Checkbox("##Visibility", &visible);
      ImGui::ColorEdit3("##Color", &color[0]);
      ImGui::InputFloat("##Opacity", &opacity);
      ImGui::EndTabItem();
      ImGui::PopFont();
    }
    ImGui::EndTabBar();
  }
  ImGui::PopFont();
  ImGui::End();
  return true;
};
  

AMN_NAMESPACE_CLOSE_SCOPE