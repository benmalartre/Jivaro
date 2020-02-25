#include "view.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
#include "imgui/imgui_test.h"

namespace AMN {

  // view constructor
  //----------------------------------------------------------------------------
  View::View(View* parent, const pxr::GfVec2i& min, const pxr::GfVec2i& max):
    _parent(parent), 
    _min(min), 
    _max(max), 
    _flags(HORIZONTAL|LEAF)
  {
    if(_parent==NULL)_name = "main";
    _color = pxr::GfVec3f(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
  }

  View::View(View* parent, int x, int y, int w, int h):
    _parent(parent), 
    _min(pxr::GfVec2i(x, y)), 
    _max(pxr::GfVec2i(x+w, y+h)), 
    _flags(HORIZONTAL|LEAF)
  {
    if(_parent==NULL)_name = "main";
    _color = pxr::GfVec3f(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
  }

  void 
  View::Draw()
  {
    if(!IsLeaf()){
      if(_left)_left->Draw();
      if(_right)_right->Draw();
    }
    else
    {
      bool opened;
      int flags = 0;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoMove;
      

      ImGui::Begin(GetText(), &opened, flags);
      pxr::GfVec4f color(
        RANDOM_0_1,
        RANDOM_0_1,
        RANDOM_0_1, 
        1.f
      );
      //ImGui::TestDummyView(&opened, GetMin(), GetMax(), color);
      ImGui::TestGrapNodes(&opened, GetMin(), GetMax());
      ImGui::SetWindowSize(GetMax() - GetMin());
      ImGui::SetWindowPos(GetMin());
      ImGui::End();
    }
    
   
    
    /*
    int x, y, w, h;
    if(!BITMASK_CHECK(_flags, HORIZONTAL))
    {
      x = (_min[0] + ((_max[0]-_min[0]) * _perc)/100) - 1;
      y = _min[1];
      w = 2;
      h = _max[1] - _min[1];
    }
    else
    {
      x = _min[0];
      y = (_min[1] + ((_max[1]-_min[1]) * _perc)/100 ) - 1;
      w = _max[0] - _min[0];
      h = 2;
    }
    
    
    glScissor(x,y,w,h);
    //glClearColor(_color[0], _color[1], _color[2], 1.f);
    glClearColor(
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      1.f
    );
    glClear(GL_COLOR_BUFFER_BIT);
    */
  }

  void 
  View::MouseEnter()
  {
    _color = pxr::GfVec3f(1.f, 0.f, 0.f);
  }

  void 
  View::MouseLeave()
  {
    _color = pxr::GfVec3f(0.f, 1.f, 0.f);
  }

  void
  View::GetChildMinMax(bool leftOrRight, pxr::GfVec2i& cMin, pxr::GfVec2i& cMax)
  {
    // horizontal splitter
    if(IsHorizontal())
    {
      if(leftOrRight)
      {
        cMin = GetMin();
        cMax[0] = GetMax()[0];
        cMax[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc * 0.01f;
      }
      else
      { 
        cMin[0] = GetMin()[0];
        cMin[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc * 0.01f;
        cMax = GetMax();
      }
    }
    // vertical splitter
    else
    {
      if(leftOrRight)
      {
        cMin = GetMin();
        cMax[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc * 0.01f;
        cMax[1] = GetMax()[1];
      }
      else
      {
        cMin[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc * 0.01f;
        cMin[1] = GetMin()[1];
        cMax = GetMax();
      }
    }
  }

  void
  View::Split()
  {
    ClearLeaf();
    pxr::GfVec2i cMin, cMax;    
    GetChildMinMax(true, cMin, cMax);
    _left = new View(this, cMin, cMax);
    _left->_name = _name + ":left";

    GetChildMinMax(false, cMin, cMax);
    _right = new View(this, cMin, cMax);
    _right->_name = _name + ":right";

    if(_content){
      std::cerr << 
        "WE GOT FUCKIN CONTENT : WE HAVE TO MOVE THIS SHIT FROM HERE !!!! " 
          << std::endl;
    }
  }
} // namespace AMN