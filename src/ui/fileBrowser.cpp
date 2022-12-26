#include "../ui/fileBrowser.h"
#include "../utils/strings.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

FileBrowserUI::FileBrowserUI(View* parent, Mode mode)
  : BaseUI(parent, UIType::FILEBROWSER)
  , _mode(mode)
  , _browsing(true)
  , _canceled(false)
  , _changed(true)
  , _showHiddenFiles(false)
{
}

FileBrowserUI::~FileBrowserUI()
{
}

void FileBrowserUI::SetPath(const std::string& path)
{
  _path = path;
  if(strlen(_path.c_str())) {
    _pathTokens = SplitString(_path, SEPARATOR);
    _GetPathEntries();
  } else {
    _GetRootEntries();
  }
  _ResetSelected();
}

void FileBrowserUI::AppendPath(const std::string& name)
{
  if(name == "..") {
    PopPath();
  } else {
    std::string newPath = _path + SEPARATOR + name;
    SetPath(newPath);
  }
}

void FileBrowserUI::PopPath()
{
  if(_pathTokens.size() > 2)
    SetPathFromTokenIndex(_pathTokens.size() - 2);
  else 
    SetPath("");
}

void FileBrowserUI::SetPathFromTokenIndex(size_t idx)
{
#ifdef _WIN32
  _path = "";
#else
  _path = SEPARATOR;
#endif
  for(size_t i=0; i<= idx; ++i) {
    if(!strlen(_pathTokens[i].c_str())) continue;
    _path += _pathTokens[i];
    if(i < idx) _path += SEPARATOR;
  }
  _pathTokens = SplitString(_path, SEPARATOR);
  _GetPathEntries();
  _ResetSelected();
}

void FileBrowserUI::SetFilters(const std::vector<std::string>& filters)
{
  _filters = filters;
}

void FileBrowserUI::_GetPathEntries()
{
  size_t numEntries = GetEntriesInDirectory(_path.c_str(), _entries);
}

void FileBrowserUI::_GetRootEntries()
{
  _pathTokens.clear();
#ifdef _WIN32
  _path = "";
#else
  _path = SEPARATOR;
#endif
  GetVolumes(_entries);
}

void FileBrowserUI::SetResult(const std::string& name)
{
  _result.resize(1);
  _result[0] = _path+ SEPARATOR + name;
}

bool FileBrowserUI::GetResult(std::string& result)
{
  if(_result.size()) {
    result = _result[0];
    return true;
  } else {
    return false;
  }
}

size_t FileBrowserUI::GetNumResults()
{
  return _result.size();
}

bool FileBrowserUI::GetResult(size_t index, std::string& result)
{
  if(index < _result.size()) {
    result = _result[index];
    return true;
  } else {
    return false;
  }
}

static void OnHomeCallback(FileBrowserUI* ui)
{
  ui->SetPath("");
}

static void OnParentCallback(FileBrowserUI* ui)
{
  ui->PopPath();
}

void FileBrowserUI::_ResetSelected()
{
  size_t numEntries = _entries.size();
  _selected.resize(numEntries);
  for(size_t i=0; i < _selected.size(); ++i) _selected[i] = false;
  _changed = true;
}

void FileBrowserUI::_DrawPath()
{
  UIUtils::AddIconButton<UIUtils::CALLBACK_FN, FileBrowserUI*>(
    0,
    ICON_FA_TRASH,
    ICON_DEFAULT,
    (UIUtils::CALLBACK_FN)&OnHomeCallback, this);
  ImGui::SameLine();
  size_t numTokens = _pathTokens.size();
  if(numTokens) {
    size_t lastTokenIndex = numTokens - 1;
    for(size_t i=0; i < numTokens; ++i) {
      if(_pathTokens[i] != "") {
        if(ImGui::Button(_pathTokens[i].c_str())) {
          lastTokenIndex = i;
        }
        ImGui::SameLine();
      }
      ImGui::Text("/");
      if(i < (numTokens - 1))ImGui::SameLine();
    }
    if(lastTokenIndex != numTokens - 1)
      SetPathFromTokenIndex(lastTokenIndex);
  } else {
    ImGui::Text("/");
  }
}

bool FileBrowserUI::_DrawEntry(ImDrawList* drawList, size_t idx, bool flip)
{
  const ImGuiStyle& style = ImGui::GetStyle();
  bool selected = false;
  const EntryInfo& info = _entries[idx];
  if(info.path == "" || info.path == "." || (!_showHiddenFiles && info.isHidden)) return false;

  std::string itemid = "##" + info.path;
  ImGui::Selectable(itemid.c_str(), &selected, 
    ImGuiSelectableFlags_AllowDoubleClick);
  if(selected) {
      _selected[idx] = 1 - _selected[idx];
  }
  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {    
    if(info.type == EntryInfo::Type::FOLDER) {
      AppendPath(info.path);
    } else if(info.type == EntryInfo::Type::FILE) {
      SetResult(info.path);
      _browsing = false;
    }
  }

  ImVec2 pos = ImVec2(
    ImGui::GetCursorPosX(), 
    ImGui::GetCursorPosY() - ImGui::GetScrollY());
  const float width = (float)GetWidth();
  if (_selected[idx]) {
    drawList->AddRectFilled(
      { 0, pos.y },
      { width, pos.y + FILEBROWSER_LINE_HEIGHT },
      ImColor(style.Colors[ImGuiCol_ButtonActive]));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + FILEBROWSER_LINE_HEIGHT },
        ImColor(style.Colors[ImGuiCol_WindowBg]));
    else
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + FILEBROWSER_LINE_HEIGHT },
        ImColor(style.Colors[ImGuiCol_ChildBg]));
  }
  
  ImGui::SameLine();

  if(info.type == EntryInfo::Type::FOLDER) {
    const static Icon* folderIcon = &ICONS[ICON_SIZE_SMALL][ICON_FOLDER];
    ImGui::Image(
      (ImTextureID)(intptr_t)folderIcon->tex[0],
      ImVec2(folderIcon->size, folderIcon->size));
  } else if(info.type == EntryInfo::Type::FILE) {
    const static Icon* fileIcon = &ICONS[ICON_SIZE_SMALL][ICON_FILE];
    ImGui::Image(
      (ImTextureID)(intptr_t)fileIcon->tex[0],
      ImVec2(fileIcon->size, fileIcon->size));
  }
  ImGui::SameLine();
  ImGui::Text("%s", info.path.c_str());

  return true;
}

bool FileBrowserUI::_DrawEntries()
{
  static bool selected = false;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
  float height = GetHeight() - 64;

  ImGui::SetCursorPosX(8);
  ImGui::BeginChild(
    "##Entries", 
    ImVec2(ImGui::GetWindowContentRegionWidth() - 8, height), 
    false, 
    windowFlags
  );

  if(_changed) {
    _changed = false;
    ImGui::SetScrollY(0.f);
  }

  ImDrawList* drawList = ImGui::GetBackgroundDrawList();
  static bool flip = false;
  for (size_t i=0; i < _entries.size(); ++i) {
    if(_DrawEntry(drawList, i, flip)) {
      flip = 1 - flip;
    }
  }
  
  ImGui::EndChild();
  return false;
}

void FileBrowserUI::_DrawButtons()
{
  ImGui::SetCursorPos(ImVec2(GetWidth()-300, GetHeight()-40));
  if(ImGui::Button("OK", ImVec2(128, 32)))_browsing=false;
  ImGui::SameLine();
  if(ImGui::Button("Cancel", ImVec2(128, 32))){
    _browsing = false;
    _canceled = true;
  }
}

bool FileBrowserUI::Draw()
{
  bool opened;
  int flags = 
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar | 
    ImGuiWindowFlags_NoMove;

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());

  _DrawPath();
  _DrawEntries();
  _DrawButtons();
  
  ImGui::End();
  return true;
};

JVR_NAMESPACE_CLOSE_SCOPE