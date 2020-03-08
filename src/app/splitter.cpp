#include "splitter.h"
#include "view.h"

AMN_NAMESPACE_OPEN_SCOPE

void AmnSplitter::RecurseBuildMap(AmnView* view)
{
  if(!view) return;
  _views.push_back(view);
  _viewID++;
  if(!view->IsLeaf())
  {
    AmnView* parent = view->GetParent();
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
AmnSplitter::BuildMap(AmnView* view)
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

// get pixel value under mouse
int 
AmnSplitter::GetPixelValue(double xPos, double yPos)
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
AmnSplitter::Draw()
{
  glEnable(GL_SCISSOR_TEST);
  glClearColor(0.f, 1.f, 0.f, 1.f);
  pxr::GfVec2f sMin, sMax;
  for(auto view : _views)
  {
    if(view->IsLeaf()) continue;
    view->GetSplitInfos(sMin, sMax, _width, _height);
    glScissor(sMin[0], _height -sMax[1], sMax[0]-sMin[0], sMax[1]-sMin[1]);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  glDisable(GL_SCISSOR_TEST);
}

AmnView* 
AmnSplitter::GetViewByIndex(int index)
{
  return _views[index];
}

void 
AmnSplitter::Resize(AmnView* view)
{
  view->Resize(view->GetMin()[0], view->GetMin()[1], 
    view->GetWidth(), view->GetHeight());
}
AMN_NAMESPACE_CLOSE_SCOPE
