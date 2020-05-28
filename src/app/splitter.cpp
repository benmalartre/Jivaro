#include "splitter.h"
#include "view.h"
#include "window.h"

AMN_NAMESPACE_OPEN_SCOPE

void Splitter::RecurseBuildMap(View* view)
{
  if(!view) return;
  _views.push_back(view);
  _viewID++;
  if(!view->GetFlag(View::LEAF))
  {
    View* parent = view->GetParent();
    if(!parent)parent = view;

    pxr::GfVec2f sMin, sMax;
    view->GetSplitInfos(sMin, sMax, _width, _height);
    for(int y = sMin[1]; y < sMax[1]; ++y)
    {
      for(int x = sMin[0]; x < sMax[0]; ++x)
      {
        _pixels[y * _width + x] = _viewID;
      }
    }
    RecurseBuildMap(view->GetLeft());
    RecurseBuildMap(view->GetRight());
  }
  //else if(view->GetContent())view->GetContent()->SetActive(true);
}

void 
Splitter::BuildMap(int width, int height)
{
  _viewID = 0;
  if (_pixels)delete[]_pixels;
  _pixels = new int[width * height];
  _valid = true;
  _width = width;
  _height = height;
  _views.clear();
  memset((void*)&_pixels[0], 0, _width * _height * sizeof(int));
}

// pick splitter
int 
Splitter::Pick(int x, int y)
{
  if(!_valid || x<0 || y <0 || x >= _width || y >= _height) 
    return -1;

  int idx = y * _width + x;
  if(idx >= 0 && idx < (_width * _height))
  {
    return _pixels[idx] - 1;
  }
  return -1;
}

// draw the splitters
void 
Splitter::Draw()
{
  ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
  pxr::GfVec2f sMin, sMax;
  static ImVec4 colf = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
  const ImU32 col = ImColor(colf);
  for(auto view : _views)
  {
    if(view->GetFlag(View::LEAF)) continue;
    view->GetSplitInfos(sMin, sMax, _width, _height);
    draw_list->AddRectFilled(sMin, sMax, col, 0.0f, 0);
    if(_cursor == ImGuiMouseCursor_ResizeEW)
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    else if(_cursor == ImGuiMouseCursor_ResizeNS)
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
  }
}

View* 
Splitter::GetViewByIndex(int index)
{
  return _views[index];
}

void 
Splitter::Resize(int width, int height)
{
  BuildMap(width, height);
}

AMN_NAMESPACE_CLOSE_SCOPE
