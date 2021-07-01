#include "modal.h"
#include "../app/view.h"
#include "../utils/strings.h"

AMN_NAMESPACE_OPEN_SCOPE

ModalUI::ModalUI(View* parent, const std::string& name, Mode mode,
  Severity severity)
  : BaseUI(parent, name)
  , _mode(mode)
  , _severity(severity)
  , _status(OK)
{
}

ModalUI::~ModalUI()
{
}

bool ModalUI::Draw()
{
  bool opened;
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::Begin(_name.c_str(), &opened, flags);

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());

  ImGui::End();
  return true;
};

AMN_NAMESPACE_CLOSE_SCOPE