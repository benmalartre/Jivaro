#include <pxr/usd/usd/schemaBase.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/curves.h>

#include "../ui/property.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../app/selection.h"
#include "../command/command.h"

PXR_NAMESPACE_OPEN_SCOPE

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

void 
PropertyUI::SetPrim(const pxr::UsdPrim& prim)
{
  if(prim.IsValid())_prim = prim;
  _initialized = false;
}

void 
PropertyUI::OnSelectionChangedNotice(const SelectionChangedNotice& n)
{
  Application* app = GetApplication();
  Selection* selection = app->GetSelection();
  if (selection->GetNumSelectedItems()) {
    pxr::UsdPrim prim = app->GetStage()->GetPrimAtPath(selection->GetSelectedPrims().back());
    SetPrim(prim);
  }
  else {
    SetPrim(pxr::UsdPrim());
  }
}

// Draw a xform common api in a table
// I am not sure this is really useful
bool 
PropertyUI::_DrawXformsCommon(pxr::UsdTimeCode time)
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
    std::vector<pxr::GfVec3d> translations;
    
    if (ImGui::BeginTable("##DrawXformsCommon", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
      ImGui::TableSetupColumn("Transform");
      ImGui::TableSetupColumn("Value");

      ImGui::TableHeadersRow();
      // Translate
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("translation");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      //ImGui::InputFloat3("Translation", translationf.data(), DecimalPrecision);
      ImGui::InputScalarN("Translation", ImGuiDataType_Double, &translation[0], 3, NULL, NULL, DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        /*
        translation[0] = translationf[0]; // TODO: InputDouble3 instead, we don't want to loose values
        translation[1] = translationf[1];
        translation[2] = translationf[2];
        */
       
        translations.push_back(translation);
        GetApplication()->AddCommand(std::shared_ptr<TranslateCommand>(
          new TranslateCommand(GetApplication()->GetStage(), { _prim.GetPath() }, translations, time, NULL))
        );

      }
      // Rotation
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

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
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("scale");

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      ImGui::InputFloat3("Scale", scale.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetScale, xformAPI, scale, currentTime);
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      //DrawPropertyMiniButton("(x)");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("pivot");

      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat3("Pivot", pivot.data(), DecimalPrecision);
      if (ImGui::IsItemDeactivatedAfterEdit()) {
        //ExecuteAfterDraw(&UsdGeomXformCommonAPI::SetPivot, xformAPI, pivot, currentTime);
      }
      // TODO rotation order
      ImGui::EndTable();
    }
    
    return true;
  }
  return false;
}

/*
bool 
PropertyUI::_DrawVariantSetsCombos(pxr::UsdPrim &prim) {
  int buttonID = 0;
  if (!prim.HasVariantSets())
    return false;
  auto variantSets = prim.GetVariantSets();

  if (ImGui::BeginTable("##DrawVariantSetsCombos", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24); // 24 => size of the mini button
    ImGui::TableSetupColumn("VariantSet");
    ImGui::TableSetupColumn("");

    ImGui::TableHeadersRow();

    for (auto variantSetName : variantSets.GetNames()) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);

      // Variant set mini button --- TODO move code from this function
      auto variantSet = variantSets.GetVariantSet(variantSetName);
      // TODO: how do we figure out if the variant set has been edited in this edit target ?
      // Otherwise after a "Clear variant selection" the button remains green and it visually looks like it did nothing
      ImVec4 variantColor =
        variantSet.HasAuthoredVariantSelection() 
        ? ImVec4(MiniButtonAuthoredColor) 
        : ImVec4(MiniButtonUnauthoredColor);
      ImGui::PushID(buttonID++);
      DrawPropertyMiniButton("(v)", variantColor);
      ImGui::PopID();
      if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::MenuItem("Clear variant selection")) {
          //ExecuteAfterDraw(&UsdVariantSet::ClearVariantSelection, variantSet);
        }
        ImGui::EndPopup();
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%s", variantSetName.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::PushItemWidth(-FLT_MIN);
      if (ImGui::BeginCombo(variantSetName.c_str(), variantSet.GetVariantSelection().c_str())) {
        for (auto variant : variantSet.GetVariantNames()) {
          if (ImGui::Selectable(variant.c_str(), false)) {
            //ExecuteAfterDraw(&UsdVariantSet::SetVariantSelection, variantSet, variant);
          }
        }
        ImGui::EndCombo();
      }
    }
    ImGui::EndTable();
  }
  return true;
}
*/

bool 
PropertyUI::Draw()
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

  Window* window = _parent->GetWindow();
  ImGui::PushFont(window->GetBoldFont(0));
  _DrawXformsCommon(pxr::UsdTimeCode::Default());

  ImGui::PopFont();
  ImGui::End();
  return true;
};
  

PXR_NAMESPACE_CLOSE_SCOPE