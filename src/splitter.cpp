#include "splitter.h"
#include "view.h"

namespace AMN {
  void 
  Splitter::RecurseBuildMap(View* view)
  {
    if(view->IsLeaf())
    {
      int x1, x2, y1, y2;
      int w = view->GetMax()[0] - view->GetMin()[0];
      if(view->IsHorizontal())
      { 
        x1 = view->GetMin()[0];
        x2 = view->GetMax()[0];
        int h = view->GetMin()[1] + (view->GetMax()[1] - view->GetMin()[1]) * 
          view->GetPerc() * 0.01f;
        y1 = h - SPLITTER_THICKNESS;
        y2 = h + SPLITTER_THICKNESS;
      }
      else
      {
        int w = view->GetMin()[0] + (view->GetMax()[0] - view->GetMin()[0]) * 
          view->GetPerc() * 0.01f;
        x1 = w - SPLITTER_THICKNESS;
        x2 = w + SPLITTER_THICKNESS;
        y1 = view->GetMin()[0];
        y2 = view->GetMax()[0];
      }
      for(int y = y1; y < y2; ++y)
      {
        for(int x = x1; x < x2; ++x)
        {
          _pixels[y * w + x] = _viewID;
        }
      }
      _viewID++;;
    }
    else
    {
      RecurseBuildMap(view->GetLeft());
      RecurseBuildMap(view->GetRight());
    }
  }

  void 
  Splitter::BuildMap(View* view)
  {
    if(_pixels)delete [] _pixels;
    if(_window)
    {
      const pxr::GfVec2i size(view->GetMax() - view->GetMin());
      _pixels = new unsigned[size[0] * size[1]];
      memset((void*)&_pixels[0], 0, size[0] * size[1] * sizeof(int));
      _viewID = 1;
      RecurseBuildMap(view);
    }
  }

} // namespace AMN