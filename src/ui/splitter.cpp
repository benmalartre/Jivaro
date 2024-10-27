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
  , _window(window)
{
};

SplitterUI::~SplitterUI()
{
  if (_pixels)delete[] _pixels;
};

void 
SplitterUI::BuildMap(int width, int height)
{
  // delete previous allocated memory
  if (_pixels)delete[]_pixels;

  // reallocate memory according to new resolution
  size_t numPixels = width * height;
  _pixels = new int[numPixels];
  _valid = true;
  _width = width;
  _height = height;

  // fill with black
  memset((void*)&_pixels[0], 0, numPixels * sizeof(int));

  // then for each leaf view assign an indexed color
  const std::vector<View*>& views = GetWindow()->GetViews();

  for (size_t viewIdx = 0; viewIdx < views.size(); ++viewIdx) {
    View* current = views[viewIdx];
    if (current->GetFlag(View::LEAF))continue;
    if (current->GetFlag(View::LFIXED | View::RFIXED)) continue;
    GfVec2f sMin, sMax;
    current->GetSplitInfos(sMin, sMax, _width, _height);
    for (int y = sMin[1]; y < sMax[1]; ++y)
      for (int x = sMin[0]; x < sMax[0]; ++x)
        _pixels[y * _width + x] = viewIdx + 1;
  }
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
    const std::vector<View*>& views = GetWindow()->GetViews();
    _hovered = views[_pixels[idx] - 1];
    return _pixels[idx] - 1;
  }
  return -1;
}

// draw the splitters
bool 
SplitterUI::Draw()
{
  const std::vector<View*>& views = GetWindow()->GetViews();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(_width, _height));

  ImGui::Begin(_name.c_str(), NULL, _flags);

  if(_cursor == ImGuiMouseCursor_ResizeEW) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  } else if(_cursor == ImGuiMouseCursor_ResizeNS) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
  } else {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
  }

  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  ImU32 color = ImColor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
  for(auto view : views)
    if(view->GetFlag(View::LEAF) && view->GetFlag(View::DIRTY))
      drawList->AddRect(view->GetMin(), view->GetMax(), color, 0.f, 0, 1.f);

  ImGui::End();
  return true;
}

View* 
SplitterUI::GetViewByIndex(int index)
{
  return GetWindow()->GetViews()[index];
}

void 
SplitterUI::Resize(int width, int height)
{
  BuildMap(width, height);
}

Window*
SplitterUI::GetWindow()
{
  return _window;
}

JVR_NAMESPACE_CLOSE_SCOPE
