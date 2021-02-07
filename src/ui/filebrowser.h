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
  enum Mode {
    OPEN,
    SAVE,
    SELECT,
    MULTI
  };

  FileBrowserUI(View* parent, const std::string& name, Mode mode);
  ~FileBrowserUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw() override;

  void SetPath(const std::string& path);
  void AppendPath(const std::string& name);
  void PopPath();
  void SetPathFromTokenIndex(size_t index);
  void SetFilters(const std::vector<std::string>& filters);
  void _GetPathEntries();
  void _GetRootEntries();

  // setters
  void SetResult(const std::string& name);

  // drawing methods
  void _DrawPath();
  bool _DrawEntries();
  void _DrawButtons();

  // state
  bool IsBrowsing(){return _browsing;};
  bool IsCanceled(){return _canceled;};

  // result
  bool GetResult(std::string&);
  size_t GetNumResults();
  bool GetResult(size_t index, std::string&);

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
  std::vector<std::string> _result;
  int                      _selected;
  Mode                     _mode;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_FILEBROWSER_H