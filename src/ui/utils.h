#ifndef JVR_UI_UTILS_H
#define JVR_UI_UTILS_H

#include <pxr/usd/usd/attribute.h>
#include "../common.h"
#include "../utils/icons.h"
#include "../ui/style.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"


PXR_NAMESPACE_OPEN_SCOPE

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using
// a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

// from https://github.com/aoterodelaroza/imgui-goodies/blob/master/imgui_widgets.cpp
static ImGuiID tooltip_shownid = 0;
static ImGuiID tooltip_hoveredid = 0;
static int tooltip_thisframe = -1;
static bool tooltip_washovered = false;
static float tooltip_time = 0.f;
static float tooltip_lastactive = 0.f;

static void AttachTooltip(const char* desc, float delay, float maxwidth, ImFont* font)
{
	ImGuiContext *g = ImGui::GetCurrentContext();
  ImGuiID id = ImGui::GetActiveID();
  float time = ImGui::GetTime();
  int thisframe = ImGui::GetFrameCount();

  if (thisframe != tooltip_thisframe){
    // run once every frame, in the first call
    if (!tooltip_washovered){
      tooltip_time = time;
      tooltip_shownid = 0;
      tooltip_hoveredid = 0;
    }
    tooltip_thisframe = thisframe;
    tooltip_washovered = false;
  }

  if (g->HoveredId == id){
    // If no tooltip is being shown and the mouse moves from one tooltip element
    // to another, this is the same as if it moved from a zone without any
    // tooltip elements.
    if (id != tooltip_hoveredid && tooltip_shownid == 0)
      tooltip_time = time;

    if (tooltip_lastactive != 0.f)
      delay = fmin(delay,fmax(tooltip_time - tooltip_lastactive,0.f));

    tooltip_washovered = true;
    tooltip_hoveredid = id;

    if (time - tooltip_time > delay){
      tooltip_shownid = id;
      tooltip_lastactive = time;
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(maxwidth);
      ImGui::PushFont(font);
      ImGui::TextUnformatted(desc);
      ImGui::PopFont();
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  }
}

// callback prototype
typedef void(*IconPressedFunc)(...);

template<typename FuncT, typename ...ArgsT>
static void IconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::BeginGroup();
  ImGui::Image(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size)
  );
  ImGui::EndGroup();
}

template<typename FuncT, typename ...ArgsT>
bool AddIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    return true;
  }
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool AddTransparentIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGui::PushStyleColor(ImGuiCol_Button, JVR_TRANSPARENT_COLOR);
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    return true;
  }
  ImGui::PopStyleColor();
  return false;
}

template<typename FuncT, typename ...ArgsT>
bool AddCheckableIconButton(Icon* icon, short state, FuncT func, ArgsT... args)
{
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;
  if(state == ICON_SELECTED) {
    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_ACTIVE_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BUTTON_ACTIVE_COLOR);
  }
  
  if (ImGui::ImageButton( 
    (ImTextureID)(intptr_t)icon->tex[state],
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    if(state == ICON_SELECTED) ImGui::PopStyleColor(2);
    return true;
  }
  if(state == ICON_SELECTED) ImGui::PopStyleColor(2);
  
  return false;
}

/*
template <typename PropertyT> 
static std::string 
GetDisplayName(const PropertyT &property) {
  return property.GetNamespace().GetString() + (property.GetNamespace() == TfToken() 
    ? std::string() 
    : std::string(":")) + property.GetBaseName().GetString();
}

inline void 
RightAlignNextItem(const char *str) {
  ImGui::SetCursorPosX(
    ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 
    ImGui::CalcTextSize(str).x - ImGui::GetScrollX() -
    2 * ImGui::GetStyle().ItemSpacing.x);
}

/// Specialization for DrawPropertyMiniButton, between UsdAttribute and UsdRelashionship
template <typename UsdPropertyT> 
const char*
SmallButtonLabel();
template <> const char *SmallButtonLabel<pxr::UsdAttribute>() { return "(a)"; };
template <> const char *SmallButtonLabel<pxr::UsdRelationship>() { return "(r)"; };

template <typename UsdPropertyT> 
void 
DrawMenuClearAuthoredValues(UsdPropertyT &property){};

template <> 
void 
DrawMenuClearAuthoredValues(pxr::UsdAttribute &attribute) {
    if (attribute.IsAuthored()) {
        if (ImGui::MenuItem(ICON_FA_EJECT " Clear")) {
          //ExecuteAfterDraw(&pxr::UsdAttribute::Clear, attribute);
        }
    }
}

template <typename UsdPropertyT> 
void 
DrawMenuRemoveProperty(UsdPropertyT &property){};

template <> 
void 
DrawMenuRemoveProperty(pxr::UsdAttribute &attribute) {
    if (ImGui::MenuItem(ICON_FA_TRASH " Remove property")) {
      //ExecuteAfterDraw(&pxr::UsdPrim::RemoveProperty, attribute.GetPrim(), attribute.GetName());
    }
}

template <typename UsdPropertyT> 
void 
DrawMenuSetKey(UsdPropertyT &property, pxr::UsdTimeCode currentTime){};

template <> 
void 
DrawMenuSetKey(pxr::UsdAttribute &attribute, pxr::UsdTimeCode currentTime) {
  if (attribute.GetVariability() == pxr::SdfVariabilityVarying && 
    attribute.HasValue() && 
    ImGui::MenuItem(ICON_FA_KEY " Set key")) {
      pxr::VtValue value;
      attribute.Get(&value, currentTime);
      //ExecuteAfterDraw<AttributeSet>(attribute, value, currentTime);
  }
}


static void 
DrawPropertyMiniButton(const char *btnStr, const ImVec4 &btnColor = ImVec4(MiniButtonUnauthoredColor)) {
  ImGui::PushStyleColor(ImGuiCol_Text, btnColor);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(TransparentColor));
  ImGui::SmallButton(btnStr);
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

template <typename UsdPropertyT> 
void 
DrawMenuEditConnection(pxr::UsdPropertyT &property) {}

template <> 
void 
DrawMenuEditConnection(UsdAttribute &attribute) {
  if (ImGui::MenuItem(ICON_FA_LINK " Create connection")) {
    //DrawModalDialog<CreateConnectionDialog>(attribute);
  }
}

// TODO: relationship
template <typename UsdPropertyT> 
void 
DrawMenuCreateValue(UsdPropertyT &property){};

template <> 
void 
DrawMenuCreateValue(pxr::UsdAttribute &attribute) {
  if (!attribute.HasValue()) {
    if (ImGui::MenuItem(ICON_FA_DONATE " Create value")) {
      //ExecuteAfterDraw<AttributeCreateDefaultValue>(attribute);
    }
  }
}

// Property mini button, should work with UsdProperty, UsdAttribute and UsdRelationShip
template <typename UsdPropertyT>
void 
DrawPropertyMiniButton(pxr::UsdPropertyT &property, const pxr::UsdEditTarget &editTarget, 
  pxr::UsdTimeCode currentTime) {
  ImVec4 propertyColor = property.IsAuthoredAt(editTarget) 
    ? ImVec4(MiniButtonAuthoredColor) 
    : ImVec4(MiniButtonUnauthoredColor);
  DrawPropertyMiniButton(SmallButtonLabel<UsdPropertyT>(), propertyColor);
  if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
    DrawMenuSetKey(property, currentTime);
    DrawMenuCreateValue(property);
    DrawMenuClearAuthoredValues(property);
    DrawMenuRemoveProperty(property);
    DrawMenuEditConnection(property);
    if (ImGui::MenuItem(ICON_FA_COPY " Copy attribute path")) {
      ImGui::SetClipboardText(property.GetPath().GetString().c_str());
    }
    ImGui::EndPopup();
  }
}
*/

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H