#pragma once

#include <pxr/imaging/glf/glew.h>
#include "../common.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "icons.h"
#include <bitset>

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

static void ImDrawList_TransformChannel_Inner(ImVector<ImDrawVert>& vtxBuffer,
  const ImVector<ImDrawIdx>& idxBuffer, const ImVector<ImDrawCmd>& cmdBuffer,
  const ImVec2& preOffset, const ImVec2& scale, const ImVec2& postOffset)
{
  auto idxRead = idxBuffer.Data;

  std::bitset<65536> indexMap;

  int minIndex = 65536;
  int maxIndex = 0;
  int indexOffset = 0;
  for (auto& cmd : cmdBuffer)
  {
    int idxCount = cmd.ElemCount;

    if (idxCount == 0) continue;

    for (int i = 0; i < idxCount; ++i)
    {
      int idx = idxRead[indexOffset + i];
      indexMap.set(idx);
      if (minIndex > idx) minIndex = idx;
      if (maxIndex < idx) maxIndex = idx;
    }

    indexOffset += idxCount;
  }

  ++maxIndex;
  for (int idx = minIndex; idx < maxIndex; ++idx)
  {
    if (!indexMap.test(idx))
      continue;

    auto& vtx = vtxBuffer.Data[idx];

    vtx.pos.x *= (vtx.pos.x + preOffset.x) * scale.x + postOffset.x;
    vtx.pos.y *= (vtx.pos.y + preOffset.y) * scale.y + postOffset.y;
  }
}

static void ImDrawList_TransformChannels(ImDrawList* drawList, int begin, int end,
  const ImVec2& preOffset, const ImVec2& scale, const ImVec2& postOffset)
{
  int lastCurrentChannel = drawList->_Splitter._Current;
  if (lastCurrentChannel != 0)
    drawList->ChannelsSetCurrent(0);

  auto& vtxBuffer = drawList->VtxBuffer;

  if (begin == 0 && begin != end)
  {
    ImDrawList_TransformChannel_Inner(vtxBuffer, drawList->IdxBuffer, drawList->CmdBuffer, preOffset, scale, postOffset);
    ++begin;
  }

  for (int channelIndex = begin; channelIndex < end; ++channelIndex)
  {
    auto& channel = drawList->_Splitter._Channels[channelIndex];
    ImDrawList_TransformChannel_Inner(vtxBuffer, channel._IdxBuffer, channel._CmdBuffer, preOffset, scale, postOffset);
  }

  if (lastCurrentChannel != 0)
    drawList->ChannelsSetCurrent(lastCurrentChannel);
}


AMN_NAMESPACE_CLOSE_SCOPE