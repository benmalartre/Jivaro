#ifndef AMN_UI_MODAL_H
#define AMN_UI_MODAL_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "../utils/icons.h"
#include "ui.h"

AMN_NAMESPACE_OPEN_SCOPE

class ModalUI : public BaseUI
{
public:
  enum Mode {
    OKONLY,
    OKCANCEL
  };

  enum Severity {
    INFO,
    WARNING,
    ERROR
  };

  enum Status {
    OK,
    CANCEL
  };

  ModalUI(View* parent, const std::string& name, Mode mode, Severity severity);
  ~ModalUI() override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw() override;

  void SetMessage(const std::string& message){_message=message;};
  void SetSeverity(Severity severity){_severity=severity;};
  void SetStatus(Status status){_status=status;};

  Status GetStatus(){return _status;};

private:
  Severity                 _severity;
  Mode                     _mode;
  std::string              _message;
  Status                   _status;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_MODAL_H