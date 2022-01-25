#ifndef JVR_UI_HEAD_H
#define JVR_UI_HEAD_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/utils.h"
#include "../ui/ui.h"


PXR_NAMESPACE_OPEN_SCOPE

#define JVR_HEAD_HEIGHT 20

class View;
class ViewHead;

class HeadedUI : public BaseUI
{
public:
  HeadedUI(View* parent, const std::string& name);
  ~HeadedUI();

  int GetX() override;
  int GetY() override;
  int GetWidth() override;
  int GetHeight() override;

private:
  ViewHead*                 _head;
  static ImGuiWindowFlags   _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_HEAD_H