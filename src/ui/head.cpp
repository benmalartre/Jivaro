#include <memory>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/head.h"
#include "../app/view.h"


PXR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags HeadedUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoDecoration;


// constructor
HeadedUI::HeadedUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
{
  _head = _parent->GetHead();
  if (!_head) _head = _parent->CreateHead();
  _head->AddChild(this);
}

// destructor
HeadedUI::~HeadedUI()
{
}

int HeadedUI::GetX()
{
  return _parent->GetMin()[0] - 1;
}

int HeadedUI::GetY()
{
  return (_parent->GetMin()[1] + JVR_HEAD_HEIGHT) - 1;
}

int HeadedUI::GetWidth()
{
  return _parent->GetWidth() + 2;
}

int HeadedUI::GetHeight()
{
  return (_parent->GetHeight() - JVR_HEAD_HEIGHT) + 2;
}

// mouse positon relative to the view
void HeadedUI::GetRelativeMousePosition(const float inX, const float inY,
  float& outX, float& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1] + JVR_HEAD_HEIGHT;
  outX = inX - parentX;
  outY = inY - parentY;
}


PXR_NAMESPACE_CLOSE_SCOPE