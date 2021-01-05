#include "filebrowser.h"
#include "../app/view.h"
#include "../utils/strings.h"

AMN_NAMESPACE_OPEN_SCOPE

FileBrowserUI::FileBrowserUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
  , _browsing(true)
  , _canceled(false)
  , _selected(0){}

FileBrowserUI::~FileBrowserUI()
{
}

void FileBrowserUI::SetPath(const std::string& path)
{
  _path = path;
  _pathTokens = SplitString(_path, SEPARATOR);
  _GetPathEntries();
}

void FileBrowserUI::AppendPath(const std::string& name)
{
  std::string newPath = _path + SEPARATOR + name;
  SetPath(newPath);
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

bool FileBrowserUI::_DrawEntries()
{

  /*
  ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, window_flags);
            for (int i = 0; i < 100; i++)
            {
                ImGui::Text("%04d: scrollable region", i);
                if (goto_line && line == i)
                    ImGui::SetScrollHereY();
            }
            if (goto_line && line >= 100)
                ImGui::SetScrollHereY();
            ImGui::EndChild();
  */
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
  ImGuiSelectableFlags entryFlags = ImGuiSelectableFlags_AllowDoubleClick;
  ImGui::BeginChild("##Entries", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, windowFlags);
  for (size_t i=0; i < _entries.size(); ++i) {
    const EntryInfo& info = _entries[i];
    
    std::string itemid = "##" + std::to_string(i);
    if (ImGui::Selectable(itemid.c_str(), i == _selected, entryFlags)) {
      _selected = i;
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
      {      
        AppendPath(info.path);
      }
    }
    ImGui::SameLine();
    
    if(info.type == EntryInfo::Type::FOLDER) {
      const static Icon* folderIcon = &AMN_ICONS[AMN_ICON_SMALL]["folder.png"];
      ImGui::Image(
        (ImTextureID)(intptr_t)folderIcon->tex,
        ImVec2(folderIcon->size, folderIcon->size));
    } else if(info.type == EntryInfo::Type::FILE) {
      const static Icon* fileIcon = &AMN_ICONS[AMN_ICON_SMALL]["file.png"];
      ImGui::Image(
        (ImTextureID)(intptr_t)fileIcon->tex,
        ImVec2(fileIcon->size, fileIcon->size));
    }
    ImGui::SameLine();
    ImGui::Text("%s", _entries[i].path.c_str());

    
  }
  ImGui::EndChild();
}

bool FileBrowserUI::Draw()
{
  bool opened;
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;

  const auto& style = ImGui::GetStyle();

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());

  size_t numTokens = _pathTokens.size();
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

  // directory entries
  std::vector<const char*> entries;
  for (int i = 0; i < _entries.size(); ++i)
      entries.push_back(_entries[i].path.c_str());

  // List box
  /*
  const char** items = entries.data();
  static int item_current = 0;
  //ImGui::Scrollbar(ImGuiAxis::ImGuiAxis_Y);
  ImGui::SetNextItemWidth(GetWidth() - (2 * style.WindowPadding.x));
  size_t heightInItems = (GetHeight() - 120) / ImGui::GetTextLineHeightWithSpacing();
  
  ImGui::ListBox("##Entries", &item_current, items, entries.size(), heightInItems);
  */
 _DrawEntries();

  // directory entries
  //ImGui::Selectable
  if(ImGui::Button("OK", ImVec2(128, 32)))_browsing=false;
  ImGui::SameLine();
  if(ImGui::Button("Cancel", ImVec2(128, 32))){
    _browsing = false;
    _canceled = true;
  }
  /*
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(0, 0),
    ImVec2(_parent->GetSize()),
    ImColor(color[0], color[1], color[2], color[3])
  );
  */
  Demo();
  
  ImGui::End();
  return true;
};

void FileBrowserUI::Demo()
{
  ImGui::ShowDemoWindow();
}

AMN_NAMESPACE_CLOSE_SCOPE