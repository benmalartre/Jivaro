#ifndef JVR_UI_FILEBROWSER_H
#define JVR_UI_FILEBROWSER_H

#include <bitset>
#include <limits>
#include <stack>
#include "../common.h"
#include "../utils/icons.h"
#include "../ui/ui.h"
#include "../ui/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

static size_t FILEBROWSER_LINE_HEIGHT = 20;
static size_t FILEBROWSER_INVALID_INDEX = std::numeric_limits<size_t>::max();

class FileBrowserUI : public BaseUI
{
public:
  using PathStack = std::stack<std::string>;
  enum Mode {
    OPEN,
    SAVE,
    SELECT,
    MULTI
  };

  FileBrowserUI(View* parent, Mode mode, size_t numFilter=0, const char* filters[]={});
  ~FileBrowserUI()         override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  void Keyboard(int key, int scancode, int action, int mods) override;
  bool Draw() override;

  void SetPath(const std::string& path);
  void UndoPath();
  void RedoPath();
  void AppendPath(const std::string& name);
  void PopPath();
  void SetPathFromTokenIndex(size_t index);
  void SetFilters(const std::vector<std::string>& filters);

  // filename
  bool IsExtensionValid(const std::string& name);
  bool IsFilenameValid(const std::string& path);

  // setters
  void SetResult(const std::string& name);

  // state
  bool IsBrowsing(){return _browsing;};
  bool IsCanceled(){return _canceled;};

  // result
  bool GetResult(std::string&);
  size_t GetNumResults();
  bool GetResult(size_t index, std::string&);

protected:
  // selection
  void _SelectNext(int mods);
  void _SelectPrevious(int mods);
  size_t _ResetSelected();
  inline bool _IsSelected(int idx) {
    return _selected[idx];
  }

  // drawing methods
  void _DrawPath();
  bool _DrawEntries();
  void _DrawButtons();
  void _DrawFilename();

  // filesystem
  void _GetPathEntries();
  void _GetRootEntries();

private:
  std::string              _path;
  char                     _filename[256];
  std::vector<std::string> _pathTokens;
  std::vector<EntryInfo>   _nextEntries;
  std::vector<EntryInfo>   _entries;
  std::vector<EntryInfo>   _volumes;
  std::vector<std::string> _directories;
  std::vector<std::string> _files;
  std::vector<std::string> _filters;

  PathStack                _undoPaths;
  PathStack                _redoPaths;

  size_t                   _current;
  bool                     _canceled;
  bool                     _browsing;
  bool                     _changed;
  bool                     _showHiddenFiles;
  bool                     _valid;
  std::vector<std::string> _result;
  std::vector<bool>        _selected;
  Mode                     _mode;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_FILEBROWSER_H