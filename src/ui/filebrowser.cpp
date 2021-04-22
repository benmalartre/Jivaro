#include "filebrowser.h"
#include "../app/view.h"
#include "../utils/strings.h"

AMN_NAMESPACE_OPEN_SCOPE

FileBrowserUI::FileBrowserUI(View* parent, const std::string& name, Mode mode)
  : BaseUI(parent, name)
  , _mode(mode)
  , _browsing(true)
  , _canceled(false)
  , _selected(0){}

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
  ResetSelected();
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

void FileBrowserUI::SetPathFromTokenIndex(size_t index)
{
  _path = "";
  for(size_t i=0; i<=index; ++i) {
    if(!strlen(_pathTokens[i].c_str())) continue;
    _path += SEPARATOR;
    _path += _pathTokens[i];
  }
  _pathTokens = SplitString(_path, SEPARATOR);
  _GetPathEntries();
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
#ifdef _WIN32

#else
  _path = SEPARATOR;
  _pathTokens.clear();
  _GetPathEntries();
#endif
}

void FileBrowserUI::SetResult(const std::string& name)
{
  _result.resize(1);
  _result[0] = _path+ SEPARATOR + name;

  std::cout << "RESULT : " << _result[0] << std::endl;
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
  if(index>=0 && index < _result.size()) {
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

void FileBrowserUI::ResetSelected()
{
  size_t numEntries = _entries.size();
  _selected.resize(numEntries);
  for(size_t i=0; i < _selected.size(); ++i) _selected[i] = false;
}

void FileBrowserUI::_DrawPath()
{
  AddIconButton<IconPressedFunc, FileBrowserUI*>(
    &AMN_ICONS[AMN_ICON_SMALL][ICON_HOME],
    (IconPressedFunc)&OnHomeCallback, this);

  size_t numTokens = _pathTokens.size();
  if(numTokens) {
    size_t lastTokenIndex = numTokens - 1;
    for(size_t i=0; i < numTokens; ++i) {
      if(ImGui::Button(_pathTokens[i].c_str())) {
        lastTokenIndex = i;
      }
      ImGui::SameLine();
      ImGui::Text("/");
      if(i < (numTokens - 1))ImGui::SameLine();
    }
    SetPathFromTokenIndex(lastTokenIndex);
  } else {
    ImGui::Text("/");
  }
}

bool FileBrowserUI::_DrawEntry(ImDrawList* drawList, size_t idx, bool flip)
{
  const EntryInfo& info = _entries[idx];
  if(info.path == ".") return false;

  std::string itemid = "##" + info.path;
  if(ImGui::Selectable(itemid.c_str(), _selected[idx], 
    ImGuiSelectableFlags_AllowDoubleClick)) {
      _selected[idx] = 1 - _selected[idx];
  }
  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {    
    if(info.type == EntryInfo::Type::FOLDER) 
      AppendPath(info.path);
  }

  ImVec2 pos = ImVec2(
    ImGui::GetCursorPosX(), 
    ImGui::GetCursorPosY() - ImGui::GetScrollY());
  const float width = (float)GetWidth();
  if (_selected[idx]) {
    drawList->AddRectFilled(
      { 0, pos.y },
      { width, pos.y + AMN_FILEBROWSER_LINE_HEIGHT },
      ImColor(AMN_SELECTED_COLOR));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + AMN_FILEBROWSER_LINE_HEIGHT },
        ImColor(AMN_BACKGROUND_COLOR));
    else
      drawList->AddRectFilled(
        { 0, pos.y },
        { width, pos.y + AMN_FILEBROWSER_LINE_HEIGHT },
        ImColor(AMN_ALTERNATE_COLOR));
  }
  
  ImGui::SameLine();

  if(info.type == EntryInfo::Type::FOLDER) {
    const static Icon* folderIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_FOLDER];
    ImGui::Image(
      (ImTextureID)(intptr_t)folderIcon->tex,
      ImVec2(folderIcon->size, folderIcon->size));
  } else if(info.type == EntryInfo::Type::FILE) {
    const static Icon* fileIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_FILE];
    ImGui::Image(
      (ImTextureID)(intptr_t)fileIcon->tex,
      ImVec2(fileIcon->size, fileIcon->size));
  }
  ImGui::SameLine();
  ImGui::Text("%s", info.path.c_str());

  return true;

  
  /*
  ImGui::SameLine();

  if(info.type == EntryInfo::Type::FOLDER) {
    const static Icon* folderIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_FOLDER];
    ImGui::Image(
      (ImTextureID)(intptr_t)folderIcon->tex,
      ImVec2(folderIcon->size, folderIcon->size));
  } else if(info.type == EntryInfo::Type::FILE) {
    const static Icon* fileIcon = &AMN_ICONS[AMN_ICON_SMALL][ICON_FILE];
    ImGui::Image(
      (ImTextureID)(intptr_t)fileIcon->tex,
      ImVec2(fileIcon->size, fileIcon->size));
  }
  ImGui::SameLine();
  ImGui::Text("%s", info.path.c_str());
  */
}

bool FileBrowserUI::_DrawEntries()
{
  static bool selected = false;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
  float height = GetHeight() - 64;

  ImGui::PushStyleColor(
    ImGuiCol_ChildBg, 
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
  
  ImGui::SetCursorPosX(8);
  ImGui::BeginChild(
    "##Entries", 
    ImVec2(ImGui::GetWindowContentRegionWidth() - 8, height), 
    false, 
    windowFlags);

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  bool flip = false;
  for (size_t i=0; i < _entries.size(); ++i) {
    if(_DrawEntry(drawList, i, flip)) {
      flip = 1 - flip;
    }
  }
  
  ImGui::PopStyleColor();
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
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());

  // draw
  _DrawPath();
  _DrawEntries();
  _DrawButtons();
  
  ImGui::End();
  return true;
};

AMN_NAMESPACE_CLOSE_SCOPE