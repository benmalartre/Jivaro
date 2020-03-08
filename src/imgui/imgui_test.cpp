#include "imgui_test.h"

namespace ImGui {
IMGUI_API  bool 
TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, 
  const char** tabLabelTooltips, bool autoLayout, int *pOptionalHoveredIndex) 
{
  ImGuiStyle& style = ImGui::GetStyle();

  const ImVec2 itemSpacing =  style.ItemSpacing;
  const ImVec4 color =        style.Colors[ImGuiCol_Button];
  const ImVec4 colorActive =  style.Colors[ImGuiCol_ButtonActive];
  const ImVec4 colorHover =   style.Colors[ImGuiCol_ButtonHovered];
  style.ItemSpacing.x =       1;
  style.ItemSpacing.y =       1;


  if (numTabs>0 && (selectedIndex<0 || selectedIndex>=numTabs)) 
    selectedIndex = 0;
  if (pOptionalHoveredIndex) *pOptionalHoveredIndex = -1;

  // Parameters to adjust to make autolayout work as expected:----------
  // The correct values are probably the ones in the comments, but I took some
  // margin so that they work well
  // with a (medium size) vertical scrollbar too [Ok I should detect its 
  // presence and use the appropriate values...].
  const float btnOffset =         2.f*style.FramePadding.x;   
  const float sameLineOffset =    2.f*style.ItemSpacing.x;   
  const float uniqueLineOffset =  2.f*style.WindowPadding.x; 
  //--------------------------------------------------------------------

  float windowWidth = 0.f,sumX=0.f;
  if (autoLayout) windowWidth = ImGui::GetWindowWidth() - uniqueLineOffset;

  bool selection_changed = false;
  for (int i = 0; i < numTabs; i++)
  {
    // push the style
    if (i == selectedIndex)
    {
      style.Colors[ImGuiCol_Button] =         colorActive;
      style.Colors[ImGuiCol_ButtonActive] =   colorActive;
      style.Colors[ImGuiCol_ButtonHovered] =  colorActive;
    }
    else
    {
      style.Colors[ImGuiCol_Button] =         color;
      style.Colors[ImGuiCol_ButtonActive] =   colorActive;
      style.Colors[ImGuiCol_ButtonHovered] =  colorHover;
    }

    ImGui::PushID(i);   // otherwise two tabs with the same name would clash.

    if (!autoLayout) {if (i>0) ImGui::SameLine();}
    else if (sumX > 0.f) 
    {
      sumX+=sameLineOffset;  
      sumX+=ImGui::CalcTextSize(tabLabels[i]).x+btnOffset;
      if (sumX>windowWidth) sumX = 0.f;
      else ImGui::SameLine();
    }

    // Draw the button
    if (ImGui::Button(tabLabels[i]))   
    {
      selection_changed = (selectedIndex!=i);selectedIndex = i;
    }
    if (autoLayout && sumX==0.f) {
      // First element of a line
      sumX = ImGui::GetItemRectSize().x;
    }
    if (pOptionalHoveredIndex) {
      if (ImGui::IsItemHovered()) {
        *pOptionalHoveredIndex = i;
        if (tabLabelTooltips && tabLabelTooltips[i] &&
          strlen(tabLabelTooltips[i])>0)  
            ImGui::SetTooltip("%s",tabLabelTooltips[i]);
      }
    }
    else if (tabLabelTooltips && tabLabelTooltips[i] && 
              ImGui::IsItemHovered() && strlen(tabLabelTooltips[i])>0) 
                ImGui::SetTooltip("%s",tabLabelTooltips[i]);
    ImGui::PopID();
  }

  // Restore the style
  style.Colors[ImGuiCol_Button] =         color;
  style.Colors[ImGuiCol_ButtonActive] =   colorActive;
  style.Colors[ImGuiCol_ButtonHovered] =  colorHover;
  style.ItemSpacing =                     itemSpacing;

  return selection_changed;
}

IMGUI_API void
FillBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
}

IMGUI_API  void 
TestDummyView(bool* p_open, const pxr::GfVec2f& vmin, const pxr::GfVec2f& vmax,
  const pxr::GfVec4f& color)
{
  ImGui::SetCursorScreenPos(vmin);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImU32 col = ImColor(color);
  draw_list->AddRectFilled(vmin, vmax, col, 0,  0); 
}

IMGUI_API  void 
TestGraphNodes(bool* p_open, const pxr::GfVec2f& vmin, const pxr::GfVec2f& vmax) 
{

  //FillBackground();

  ImGui::SetCursorScreenPos(vmin);

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  const pxr::GfVec2f p = ImGui::GetCursorScreenPos();
  pxr::GfVec2f _min1 = pxr::GfVec2f(25,25) + p;
  pxr::GfVec2f _max1 = pxr::GfVec2f(75,50) + p;

  pxr::GfVec2f _min2 = pxr::GfVec2f(100,25) + p;
  pxr::GfVec2f _max2 = pxr::GfVec2f(150,50) + p;
  static ImVec4 colf = ImVec4(
    (float)rand()/(float)RAND_MAX, 
    (float)rand()/(float)RAND_MAX, 
    (float)rand()/(float)RAND_MAX,
    1.0f
  );

  //std::cout << colf.x << "," << colf.y << "," << colf.z << std::endl;
  const ImU32 col = rand();//ImColor(colf.x, colf.y, colf.z, 1.f);
  //std::cout << col << std::endl;
  float rounding = 8.f;
  float th = 1.f;
  const ImDrawCornerFlags corners_none = 0;
  const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;

  draw_list->AddRect(_min1, _max1, col, rounding,  corners_none, th); 
  draw_list->AddRect(_min2, _max2, col, rounding, corners_all, th);

  pxr::GfVec2f offset(32, 64);
  draw_list->AddRectFilled(_min1+offset, _max1+offset, col, rounding,  corners_none); 
  draw_list->AddRectFilled(_min2+offset, _max2+offset, col, rounding, corners_all);

}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------
/*
// Demonstrate using the low-level ImDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
    if (!ImGui::Begin("Example: Custom rendering", p_open))
    {
        ImGui::End();
        return;
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your 
    // own geometrical types and benefit of overloaded operators, etc.
    // Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions 
    // between your types and ImVec2/ImVec4.
    // ImGui defines overloaded operators but they are internal to imgui.cpp 
    // and not exposed outside (to avoid messing with your types)
    // In this example we are not using the maths operators!
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (ImGui::BeginTabBar("##TabBar"))
    {
        // Primitives
        if (ImGui::BeginTabItem("Primitives"))
        {
            static float sz = 36.0f;
            static float thickness = 3.0f;
            static int ngon_sides = 6;
            static bool circle_segments_override = false;
            static int circle_segments_override_v = 12;
            static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            ImGui::PushItemWidth(-ImGui::GetFontSize() * 10);
            ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
            ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
            ImGui::SliderInt("N-gon sides", &ngon_sides, 3, 12);
            ImGui::Checkbox("##circlesegmentoverride", &circle_segments_override);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::SliderInt("Circle segments", &circle_segments_override_v, 3, 40))
                circle_segments_override = true;
            ImGui::ColorEdit4("Color", &colf.x);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImU32 col = ImColor(colf);
            const float spacing = 10.0f;
            const ImDrawCornerFlags corners_none = 0;
            const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;
            const ImDrawCornerFlags corners_tl_br = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight;
            const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
            float x = p.x + 4.0f, y = p.y + 4.0f;
            for (int n = 0; n < 2; n++)
            {
                // First line uses a thickness of 1.0f, second line uses the configurable thickness
                float th = (n == 0) ? 1.0f : thickness;
                draw_list->AddNgon(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, ngon_sides, th);         x += sz + spacing;  // N-gon
                draw_list->AddCircle(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments, th);  x += sz + spacing;  // Circle
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 0.0f,  corners_none, th);     x += sz + spacing;  // Square
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_all, th);      x += sz + spacing;  // Square with all rounded corners
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br, th);    x += sz + spacing;  // Square with two rounded corners
                draw_list->AddTriangle(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col, th);      x += sz + spacing;      // Triangle
                draw_list->AddTriangle(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col, th); x += sz*0.4f + spacing; // Thin triangle
                draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y), col, th);                               x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col, th);                               x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col, th);                          x += sz + spacing;  // Diagonal line
                draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x + sz*1.3f, y + sz*0.3f), ImVec2(x + sz - sz*1.3f, y + sz - sz*0.3f), ImVec2(x + sz, y + sz), col, th);
                x = p.x + 4;
                y += sz + spacing;
            }
            draw_list->AddNgonFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz*0.5f, col, ngon_sides);   x += sz + spacing;  // N-gon
            draw_list->AddCircleFilled(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments);x += sz + spacing;  // Circle
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col);                        x += sz + spacing;  // Square
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f);                 x += sz + spacing;  // Square with all rounded corners
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);  x += sz + spacing;  // Square with two rounded corners
            draw_list->AddTriangleFilled(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col);      x += sz + spacing;      // Triangle
            draw_list->AddTriangleFilled(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + thickness), col);                 x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + thickness, y + sz), col);                 x += spacing*2.0f;  // Vertical line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), col);                          x += sz;            // Pixel (faster than AddLine)
            draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
            ImGui::Dummy(ImVec2((sz + spacing) * 9.8f, (sz + spacing) * 3));

            // Draw black and white gradients
            static int gradient_steps = 16;
            ImGui::Separator();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Gradient steps");
            ImGui::SameLine(); if (ImGui::RadioButton("16", gradient_steps == 16)) { gradient_steps = 16; }
            ImGui::SameLine(); if (ImGui::RadioButton("32", gradient_steps == 32)) { gradient_steps = 32; }
            ImGui::SameLine(); if (ImGui::RadioButton("256", gradient_steps == 256)) { gradient_steps = 256; }
            ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), 64.0f);
            x = ImGui::GetCursorScreenPos().x;
            y = ImGui::GetCursorScreenPos().y;
            for (int n = 0; n < gradient_steps; n++)
            {
                float f0 = n / (float)gradient_steps;
                float f1 = (n + 1) / (float)gradient_steps;
                ImU32 col32 = ImGui::GetColorU32(ImVec4(f0, f0, f0, 1.0f));
                draw_list->AddRectFilled(ImVec2(x + gradient_size.x * f0, y), ImVec2(x + gradient_size.x * f1, y + gradient_size.y), col32);
            }
            ImGui::InvisibleButton("##gradient", gradient_size);

            ImGui::PopItemWidth();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Canvas"))
        {
            static ImVector<ImVec2> points;
            static bool adding_line = false;
            if (ImGui::Button("Clear")) points.clear();
            if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
            ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

            // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
            // But you can also draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
            // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
            if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
            if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
            draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
            draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

            bool adding_preview = false;
            ImGui::InvisibleButton("canvas", canvas_size);
            ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
            if (adding_line)
            {
                adding_preview = true;
                points.push_back(mouse_pos_in_canvas);
                if (!ImGui::IsMouseDown(0))
                    adding_line = adding_preview = false;
            }
            if (ImGui::IsItemHovered())
            {
                if (!adding_line && ImGui::IsMouseClicked(0))
                {
                    points.push_back(mouse_pos_in_canvas);
                    adding_line = true;
                }
                if (ImGui::IsMouseClicked(1) && !points.empty())
                {
                    adding_line = adding_preview = false;
                    points.pop_back();
                    points.pop_back();
                }
            }
            draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
            for (int i = 0; i < points.Size - 1; i += 2)
                draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i + 1].x, canvas_pos.y + points[i + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
            draw_list->PopClipRect();
            if (adding_preview)
                points.pop_back();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("BG/FG draw lists"))
        {
            static bool draw_bg = true;
            static bool draw_fg = true;
            ImGui::Checkbox("Draw in Background draw list", &draw_bg);
            ImGui::SameLine(); HelpMarker("The Background draw list will be rendered below every Dear ImGui windows.");
            ImGui::Checkbox("Draw in Foreground draw list", &draw_fg);
            ImGui::SameLine(); HelpMarker("The Foreground draw list will be rendered over every Dear ImGui windows.");
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            if (draw_bg)
                ImGui::GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, IM_COL32(255, 0, 0, 200), 48, 10+4);
            if (draw_fg)
                ImGui::GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, IM_COL32(0, 255, 0, 200), 48, 10);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
*/
} // namespace ImGui