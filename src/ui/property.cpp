#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/curves.h>

#include "../ui/property.h"
#include "../app/view.h"
#include "../app/window.h"

JVR_NAMESPACE_OPEN_SCOPE

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

// Draw a xform common api in a table
// I am not sure this is really useful
bool PropertyUI::_DrawXformsCommon(pxr::UsdTimeCode time)
{
  pxr::UsdGeomXformCommonAPI xformAPI(_prim);

  if (xformAPI) {
    pxr::GfVec3d translation;
    pxr::GfVec3f scale;
    pxr::GfVec3f pivot;
    pxr::GfVec3f rotation;
    pxr::UsdGeomXformCommonAPI::RotationOrder rotOrder;
    xformAPI.GetXformVectors(&translation, &rotation, &scale, &pivot, &rotOrder, time);
    pxr::GfVec3f translationf(translation[0], translation[1], translation[2]);
    /*
    if (ImGui::BeginTable("##DrawXformsCommon", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
      ImGui::TableSetupColumn("Transform");
      ImGui::TableSetupColumn("Value");

      ImGui::TableHeadersRow();
      // Translate
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("translation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Translation", translationf.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        translation[0] = translationf[0]; // TODO: InputDouble3 instead, we don't want to loose values
        translation[1] = translationf[1];
        translation[2] = translationf[2];
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetTranslate, xformAPI, translation, currentTime);
      }
      // Rotation
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("rotation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Rotation", rotation.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        std::cout << "SET XFORM API !" << std::endl;
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetRotate, xformAPI, rotation, rotOrder, currentTime);
      }
      // Scale
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("scale");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Scale", scale.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetScale, xformAPI, scale, currentTime);
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("pivot");

      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat3("Pivot", pivot.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetPivot, xformAPI, pivot, currentTime);
      }
      // TODO rotation order
      ImGui::EndTable();
    }
    */
    return true;
  }
  return false;
}

bool PropertyUI::Draw()
{
  if (!_prim)return false;
  if (!_initialized)_initialized = true;

  if (_prim.IsA<pxr::UsdGeomXform>()) {
    pxr::UsdGeomXform xfo(_prim);
    pxr::TfTokenVector attrNames = xfo.GetSchemaAttributeNames();
  }

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
  

JVR_NAMESPACE_CLOSE_SCOPE