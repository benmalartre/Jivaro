#ifndef AMN_UI_FILEBROWSER_H
#define AMN_UI_FILEBROWSER_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "../utils/icons.h"
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

  void SetPath(const std::string& path);
  void AppendPath(const std::string& name);
  void SetPathFromTokenIndex(size_t index);
  void SetFilters(const std::vector<std::string>& filters);
  void _GetPathEntries();
  bool _DrawEntries();

  const std::string& GetResult(){return _result;};
  bool IsBrowsing(){return _browsing;};
  bool IsCanceled(){return _canceled;};
  void Demo();
private:
  std::string              _path;
  std::vector<std::string> _pathTokens;
  std::vector<EntryInfo>   _entries;
  std::vector<std::string> _directories;
  std::vector<std::string> _files;
  std::vector<std::string> _filters;
  bool                     _canceled;
  bool                     _browsing;
  std::string              _result;
  int                      _selected;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_FILEBROWSER_H