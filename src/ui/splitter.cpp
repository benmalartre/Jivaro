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

SplitterUI::SplitterUI(View* parent)
  : BaseUI(parent, UIType::SPLITTER)
  , _hovered(NULL)
  , _pixels(NULL)
  , _valid(false)
{
};

SplitterUI::~SplitterUI()
{
  if (_pixels)delete[] _pixels;
};

void SplitterUI::RecurseBuildMap(View* view)
{
  if(!view) return;
  _views.push_back(view);
  int viewID = _views.size();
  if(!view->GetFlag(View::LEAF))
  {
    View* parent = view->GetParent();
    if(!parent)parent = view;

    if (!view->GetFlag(View::LFIXED|View::RFIXED)) {
      pxr::GfVec2f sMin, sMax;
      view->GetSplitInfos(sMin, sMax, _width, _height);
      for (int y = sMin[1]; y < sMax[1]; ++y)
      {
        for (int x = sMin[0]; x < sMax[0]; ++x)
        {
          _pixels[y * _width + x] = viewID;
        }
      }
    }
    
    RecurseBuildMap(view->GetLeft());
    RecurseBuildMap(view->GetRight());
  }
  //else if(view->GetContent())view->GetContent()->SetActive(true);
}

void 
SplitterUI::BuildMap(int width, int height)
{
  size_t numPixels = width * height;
  if (_pixels)delete[]_pixels;
  _pixels = new int[numPixels];
  _valid = true;
  _width = width;
  _height = height;
  _views.clear();
  memset((void*)&_pixels[0], 0, numPixels * sizeof(int));
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
    _hovered = _views[_pixels[idx] - 1];
    return _pixels[idx] - 1;
  }
  return -1;
}

// draw the splitters
bool 
SplitterUI::Draw()
{
  static bool open;

  ImGui::Begin(_name.c_str(), &open, _flags);
  ImGui::SetWindowPos(ImVec2(0, 0));
  ImGui::SetWindowSize(ImVec2(_width, _height));
  
  for(auto view : _views)
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
  return _views[index];
}

void 
SplitterUI::Resize(int width, int height)
{
  std::cout << "splitter resize .." << std::endl;
  BuildMap(width, height);
  std::cout << "splitter map build" << std::endl;
  std::cout << "window : " << GetWindow() << std::endl;
  std::cout << "view : " << GetWindow()->GetMainView() << std::endl;
  RecurseBuildMap(GetWindow()->GetMainView());
  std::cout << "splitter resized" << std::endl;
}

JVR_NAMESPACE_CLOSE_SCOPE
