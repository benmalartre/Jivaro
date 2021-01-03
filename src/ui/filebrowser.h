#ifndef AMN_UI_FILEBROWSER_H
#define AMN_UI_FILEBROWSER_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "ui.h"

AMN_NAMESPACE_OPEN_SCOPE

class FileBrowserUI : public BaseUI
{
public:
  FileBrowserUI(View* parent, const std::string& name);
  ~FileBrowserUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw() override;

  void FillBackground();
  void Demo();
private:
  std::string _path;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_FILEBROWSER_H