#pragma once

#include <pxr/imaging/glf/glew.h>
#include "../common.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "icons.h"

AMN_NAMESPACE_OPEN_SCOPE

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

/*
bool InputRect(const char* label, pxr::GfVec2f& pos, pxr::GfVec2f& size,
    int decimal_precision = -1, ImGuiInputTextFlags extra_flags = 0)
{
    ImGui::PushID(label);
    ImGui::BeginGroup();

    bool valueChanged = false;

    std::array<float*, 4> arr = { &pos[0], &pos[1],
                                  &size[0], &size[1] };

    for (auto& elem : arr) {
        ImGui::PushID(elem);
        ImGui::PushItemWidth(64.f);
        valueChanged |= ImGui::InputFloat("##arr", elem, 0, 0, decimal_precision, extra_flags);
        ImGui::PopID();
        ImGui::SameLine();
    }

    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    ImGui::EndGroup();

    ImGui::PopID(); // pop label id;

    return valueChanged;
}
*/

// callback prototype
typedef void (* IconPressedFunc)(...);

template<typename FuncT, typename ...ArgsT>
static void IconButton(Icon* icon, FuncT func, ArgsT... args)
{
  ImGui::BeginGroup();
  ImGui::Image(
    (ImTextureID)(intptr_t)icon->_tex,
    ImVec2(icon->_size, icon->_size)
  );
  ImGui::EndGroup();
}



template<typename FuncT, typename ...ArgsT>
void AddIconButton(Icon* icon, FuncT func, ArgsT... args)
{
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->_tex, 
    ImVec2(icon->_size,icon->_size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
  }
  ImGui::SameLine();
}

AMN_NAMESPACE_CLOSE_SCOPE