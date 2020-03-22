#include "splitter.h"
#include "view.h"

AMN_NAMESPACE_OPEN_SCOPE

void Splitter::RecurseBuildMap(View* view)
{
  if(!view) return;
  _views.push_back(view);
  _viewID++;
  if(!view->IsLeaf())
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
}

void 
Splitter::BuildMap(View* view)
{
  
  if(_pixels){ delete [] _pixels; _pixels = NULL; };

  _views.clear();
  const pxr::GfVec2f size(view->GetMax() - view->GetMin());
  
  _width = size[0];
  _height = size[1];

  _pixels = new int[_width * _height];
  memset((void*)&_pixels[0], 0, _width * _height * sizeof(unsigned));
  _viewID = 0;
  RecurseBuildMap(view); 
}

// pick splitter
int 
Splitter::Pick(int x, int y)
{
  if(x<0 || y <0 || x >= _width || y >= _height) 
    return -1;

  unsigned idx = (unsigned) y * _width + (unsigned)x;
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
    if(view->IsLeaf()) continue;
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
Splitter::Resize(int width, int height, View* view, bool isWindowResize)
{
  _width = width;
  _height = height;
  view->Resize(0, 0, width, height, isWindowResize);
  BuildMap(view);
}

AMN_NAMESPACE_CLOSE_SCOPE
