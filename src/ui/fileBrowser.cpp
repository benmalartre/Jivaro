#include "../ui/fileBrowser.h"
#include "../utils/strings.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

FileBrowserUI::FileBrowserUI(View* parent, Mode mode, size_t numFilters, const char* filters[])
  : BaseUI(parent, UIType::FILEBROWSER)
  , _mode(mode)
  , _current(0)
  , _browsing(true)
  , _canceled(false)
  , _changed(true)
  , _showHiddenFiles(false)
{
  memset(&_filename[0], (char)0, 256 * sizeof(char));
  _filters.resize(numFilters);
  for(size_t n = 0; n < numFilters; ++n)
    _filters[n] = filters[n];
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
  if (_ResetSelected()) {
    _current = 0;
    _selected[_current] = true;
  }
}

void FileBrowserUI::UndoPath()
{
  if(_undoPaths.size()) {
    _redoPaths.push(_path);
    SetPath(_undoPaths.top());
    _undoPaths.pop();
  }
}

void FileBrowserUI::RedoPath()
{
  if(_redoPaths.size()) {
    _undoPaths.push(_path);
    SetPath(_redoPaths.top());
    _redoPaths.pop();
  }
}

void FileBrowserUI::AppendPath(const std::string& name)
{
  if(name == "..") {
    PopPath();
  } else {
    std::string newPath = _path + SEPARATOR + name;
    _undoPaths.push(_path);
    SetPath(newPath);
  }
  _current = 0;
}

void FileBrowserUI::PopPath()
{
  _undoPaths.push(_path);
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
  if (_ResetSelected()) {
    _current = 0;
    _selected[_current] = true;
  }
}

void FileBrowserUI::SetFilters(const std::vector<std::string>& filters)
{
  _filters = filters;
}

// filename
bool 
FileBrowserUI::IsExtensionValid(const std::string& name)
{
  if(!_filters.size())
    return true;
    
  const std::string extension = "." + name.substr(name.find_last_of(".") + 1);
  for(const auto& filter: _filters)
    if(extension == filter)
      return true;
  
  return false;
}

bool 
FileBrowserUI::IsFilenameValid(const std::string& path)
{
  return true;
}

void FileBrowserUI::_GetPathEntries()
{
  size_t numEntries = GetEntriesInDirectory(_path.c_str(), _nextEntries);
}

void FileBrowserUI::_GetRootEntries()
{
  _pathTokens.clear();
#ifdef _WIN32
  _path = "";
#else
  _path = SEPARATOR;
#endif
  GetVolumes(_nextEntries);
}

void FileBrowserUI::SetResult(const std::string& name)
{
  if(_filters.size() && !IsExtensionValid(name)) {
    std::cout << name << " is not a valid file name, accepts only:\n" << std::endl;
    for(auto& filter: _filters)
      std::cout << "\t - " << filter.c_str() << "\n";
    std::cout << std::endl;
  } else {
    _result.resize(1);
    _result[0] = _path+ SEPARATOR + name;
    strcpy(_filename, name.c_str());
  }
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

size_t FileBrowserUI::_ResetSelected()
{
  size_t numEntries = _nextEntries.size();
  _selected.resize(numEntries);
  for(size_t i=0; i < numEntries; ++i) _selected[i] = false;
  _current = FILEBROWSER_INVALID_INDEX;
  _changed = true;
  return numEntries;
}

void FileBrowserUI::_SelectPrevious(int mods) 
{
  size_t previous = _current > 0 ? _current - 1 : _selected.size() - 1;
  if (!mods) _ResetSelected();
  _selected[previous] = true;
  _current = previous;
}

void FileBrowserUI::_SelectNext(int mods)
{
  size_t next = (_current + 1) % _selected.size();
  if (!mods) _ResetSelected();
  _selected[next] = true;
  _current = next;
}

void FileBrowserUI::_DrawPath()
{
  UI::AddIconButton(0, ICON_FA_HARD_DRIVE, UI::STATE_DEFAULT,
    std::bind(&FileBrowserUI::SetPath, this, ""));
  ImGui::SameLine();

  UI::AddIconButton(0, ICON_FA_ARROW_LEFT, 
    _undoPaths.size() ? UI::STATE_DEFAULT : UI::STATE_DISABLED,
    std::bind(&FileBrowserUI::UndoPath, this));
  ImGui::SameLine();

  UI::AddIconButton(0, ICON_FA_ARROW_RIGHT, 
    _redoPaths.size() ? UI::STATE_DEFAULT : UI::STATE_DISABLED,
    std::bind(&FileBrowserUI::RedoPath, this));
  ImGui::SameLine();
  
  ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);

  size_t numTokens = _pathTokens.size();
  if(numTokens) {
    size_t lastTokenIndex = numTokens - 1;
    for(size_t i=0; i < numTokens; ++i) {
      if(_pathTokens[i] != "") {
        ImGui::PushID(i);
        if(ImGui::Button(_pathTokens[i].c_str()))
          lastTokenIndex = i;
        ImGui::PopID();
        ImGui::SameLine();
      }
      ImGui::Text("/");
      if(i < (numTokens - 1))ImGui::SameLine();
    }
    if(lastTokenIndex != numTokens - 1) {
      _undoPaths.push(_path);
      SetPathFromTokenIndex(lastTokenIndex);
    }
  } else {
    ImGui::Text("/");
  }

  ImGui::PopStyleColor();

}

bool FileBrowserUI::_DrawEntries()
{
  ImVec2 size(0, GetSize()[1] - 64);
  ImGui::PushItemWidth(-1); // List takes the full size
  if (ImGui::BeginListBox("##FileList", size)) {
    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("Files", 4, tableFlags)) {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("Filename", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      int i = 0;
      ImGui::PushID("direntries");
      for (auto entry : _entries) {
        const bool isDirectory = entry.type == EntryInfo::Type::FOLDER;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(i);

        int flags = 
          ImGuiSelectableFlags_SpanAllColumns | 
          ImGuiSelectableFlags_AllowItemOverlap | 
          ImGuiSelectableFlags_AllowDoubleClick;

        if (ImGui::Selectable("", false, flags)) {
          _selected[i] = 1 - _selected[i];
          
          if(entry.type == EntryInfo::Type::FILE)
            SetResult(entry.path);

          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (entry.type == EntryInfo::Type::FOLDER) 
              AppendPath(entry.path);
            
            else if (entry.type == EntryInfo::Type::FILE)
              if(IsExtensionValid(entry.path))
                _browsing = false;
          }
        }
        ImGui::PopID();
        ImGui::SameLine();
        if (entry.type == EntryInfo::Type::FOLDER) {
          ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "%s ", ICON_FA_FOLDER);
          ImGui::TableSetColumnIndex(1); // the following line is allocating string for each directory/files, this is BAD
          ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "%s", entry.path.c_str());
        }
        else {
          ImGui::TextColored(ImVec4(0.9, 0.9, 0.9, 1.0), "%s ", ICON_FA_FILE);
          ImGui::TableSetColumnIndex(1);
          ImGui::TextColored(ImVec4(0.5, 1.0, 0.5, 1.0), "%s", entry.path.c_str());
        }
        ImGui::TableSetColumnIndex(2);

        ImGui::TableSetColumnIndex(3);
        if (!isDirectory) {
          //DrawFileSize(entry.size);
        }

        i++;
      }
      ImGui::PopID(); // direntries
      ImGui::EndTable();
    }
    ImGui::EndListBox();
    
  }

  ImGui::PopItemWidth();
  return false;
}

void FileBrowserUI::_DrawButtons()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  ImVec2 buttonSize(100.f, 0.f);
  float widthNeeded = buttonSize.x + 2 * style.ItemSpacing.x + buttonSize.x;
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);

  if(ImGui::Button("OK", buttonSize))
    _browsing=false;
  ImGui::SameLine();

  if(ImGui::Button("Cancel", buttonSize)){
    _browsing = false;
    _canceled = true;
  }
}

void FileBrowserUI::_DrawFilename()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  ImGui::BeginGroup();
  ImGui::TextColored(style.Colors[ImGuiCol_TextDisabled], "name");
  ImGui::SameLine();
    
  if(_mode == Mode::SAVE) {
    if(ImGui::InputText("##filename", &_filename[0], 256))
      SetResult(_filename);
  } else {
    ImGui::TextColored(ImVec4(0.5,0.7,0.9,1.0), _filename);
  }

  ImGui::TextColored(style.Colors[ImGuiCol_TextDisabled], "result");
  ImGui::SameLine();
  if (_result.size())
    ImGui::TextColored(style.Colors[ImGuiCol_PlotLines], _result[0].c_str());

  else
    ImGui::TextColored(style.Colors[ImGuiCol_TextDisabled], "not set");

    
  ImGui::EndGroup();
  ImGui::SameLine();
  
}

bool FileBrowserUI::Draw()
{
  bool opened;
  int flags = 
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar | 
    ImGuiWindowFlags_NoMove;

  ImGui::SetNextWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetNextWindowPos(_parent->GetMin());

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0,0,0,0 });
  ImGui::PushFont(DEFAULT_FONT);
  
  _entries = _nextEntries;
  _DrawPath();
  _DrawEntries();
  _DrawFilename();
  _DrawButtons();

  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::End();
  return true;
};

static size_t _GetFistSelectedIndex(std::vector<bool>& selected)
{
  for (size_t i = 0; i < selected.size(); ++i) {
    if (selected[i]) return i;
  }
  return FILEBROWSER_INVALID_INDEX;
}

void FileBrowserUI::Keyboard(int key, int scancode, int action, int mods)
{

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (key == GLFW_KEY_DOWN) {
      _SelectNext(mods);
    }
    else if (key == GLFW_KEY_UP) {
      _SelectPrevious(mods);
    }
    else if (key == GLFW_KEY_ENTER) {
      if (_current != FILEBROWSER_INVALID_INDEX) {
        const EntryInfo& info = _entries[_current];
        if (info.type == EntryInfo::Type::FOLDER) {
          if(_mode == Mode::SAVE) {
            SetResult(_filename);
             _browsing = false;
          } else AppendPath(info.path);
        }
        else if (info.type == EntryInfo::Type::FILE) {
          SetResult(info.path);
          _browsing = false;
        }
      } else {
        _canceled = true;
      }
    }
   }
}

JVR_NAMESPACE_CLOSE_SCOPE