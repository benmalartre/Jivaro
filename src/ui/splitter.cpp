#include "../ui/style.h"
#include "../ui/splitter.h"
#include "../app/view.h"
#include "../app/window.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags SplitterUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoBackground |
  ImGuiWindowFlags_NoDecoration |
  ImGuiWindowFlags_NoInputs |
  ImGuiWindowFlags_NoMouseInputs |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoNavInputs |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar;

SplitterUI::SplitterUI(Window* window)
  : BaseUI(NULL, UIType::SPLITTER)
  , _hovered(NULL)
  , _pixels(NULL)
  , _valid(false)
{
};

SplitterUI::~SplitterUI()
{
  if (_pixels)delete[] _pixels;
};

void 
SplitterUI::BuildMap(int width, int height)
{
  std::cout << "build map start : " << width << "," << height << std::endl;
  // delete previous allocated memory
  if (_pixels)delete[]_pixels;
  std::cout << "delete ol pixel" << std::endl;
  // reallocate memory according to new resolution
  size_t numPixels = width * height;
  _pixels = new int[numPixels];
  _valid = true;
  _width = width;
  _height = height;
  std::cout << "allocate nw pixel" << std::endl;
  // fill with black
  memset((void*)&_pixels[0], 0, numPixels * sizeof(int));
  std::cout << "memset pixel" << std::endl;

  // then for each leaf view assign an indexed color
  const std::vector<View*>& views = GetWindow()->GetLeaves();
  std::cout << "num leaves : " << views.size() << std::endl;
  for (size_t viewIdx = 0; viewIdx < views.size(); ++viewIdx) {
    if (views[viewIdx]->GetFlag(View::LFIXED) || views[viewIdx]->GetFlag(View::RFIXED))
      continue;
    
    pxr::GfVec2f sMin, sMax;
    views[viewIdx]->GetSplitInfos(sMin, sMax, _width, _height);
    for (int y = sMin[1]; y < sMax[1]; ++y)
      for (int x = sMin[0]; x < sMax[0]; ++x)
        _pixels[y * _width + x] = viewIdx;
  }
  std::cout << "repaint done " << views.size() << std::endl;
}

// pick splitter
int 
SplitterUI::Pick(int x, int y)
{
  if(!_valid || x<0 || y <0 || x >= _width || y >= _height) 
    return -1;

  int idx = y * _width + x;
  if(idx >= 0 && idx < (_width * _height))
  {
    const std::vector<View*>& views = GetWindow()->GetLeaves();
    _hovered = views[_pixels[idx] - 1];
    return _pixels[idx] - 1;
  }
  return -1;
}

// draw the splitters
bool 
SplitterUI::Draw()
{
  static bool open;
  const std::vector<View*>& views = GetWindow()->GetLeaves();
  ImGui::Begin(_name.c_str(), &open, _flags);
  ImGui::SetWindowPos(ImVec2(0, 0));
  ImGui::SetWindowSize(ImVec2(_width, _height));
  
  for(auto view : views)
  {
    if(view->GetFlag(View::LEAF)) continue;

    if(_cursor == ImGuiMouseCursor_ResizeEW) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    } else if(_cursor == ImGuiMouseCursor_ResizeNS) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    } else {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }
  }

  ImGui::End();
  return true;
}

View* 
SplitterUI::GetViewByIndex(int index)
{
  return GetWindow()->GetLeaves()[index];
}

void 
SplitterUI::Resize(int width, int height)
{
  std::cout << "splitter resize start " << std::endl;
  std::cout << "size : " << width << "," << height << std::endl;
  BuildMap(width, height);
  std::cout << "splitter build map done" << std::endl;
}

Window*
SplitterUI::GetWindow()
{
  return _window;
}

JVR_NAMESPACE_CLOSE_SCOPE
