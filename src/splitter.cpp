#include "splitter.h"
#include "view.h"

namespace AMN {
  void Splitter::RecurseBuildMap(View* view)
  {
    if(!view) return;
    _views.push_back(view);
    _viewID++;
    if(!view->IsLeaf())
    {
      View* parent = view->GetParent();
      if(!parent)parent = view;

      pxr::GfVec2i sMin, sMax;
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
    const pxr::GfVec2i size(view->GetMax() - view->GetMin());
    
    _width = size[0];
    _height = size[1];

    _pixels = new unsigned[_width * _height];
    memset((void*)&_pixels[0], 0, _width * _height * sizeof(unsigned));
    _viewID = 0;
    RecurseBuildMap(view); 
  }

  // get pixel value under mouse
  unsigned 
  Splitter::GetPixelValue(double xPos, double yPos)
  {
    unsigned idx = (unsigned) yPos * _width + (unsigned)xPos;
    if(idx >= 0 && idx < (_width * _height))
    {
      return _pixels[idx];
    }
    return 0;
  }

  // debug draw the splitters
  void 
  Splitter::Draw()
  {
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.f, 1.f, 0.f, 1.f);
    pxr::GfVec2i sMin, sMax;
    for(auto view : _views)
    {
      if(view->IsLeaf()) continue;
      view->GetSplitInfos(sMin, sMax, _width, _height);
      glScissor(sMin[0], _height -sMax[1], sMax[0]-sMin[0], sMax[1]-sMin[1]);
      glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);
  }

  View* 
  Splitter::GetViewByIndex(int index)
  {
    return _views[index];
  }

  void 
  Splitter::Resize(View* view)
  {
    view->Resize(view->GetMin()[0], view->GetMin()[1], 
      view->GetWidth(), view->GetHeight());
  }
} // namespace AMN