#ifndef AMN_APPLICATION_MODAL_H
#define AMN_APPLICATION_MODAL_H
#pragma once

#include "../common.h"
#include "window.h"
AMN_NAMESPACE_OPEN_SCOPE

class Modal
{
public:
  enum Mode {
    FILE,
    FOLDER,
    WARNING,
    ERROR,
    OK_CANCEL
  };

  enum Status {
    UNKNOWN,
    OK,
    CANCEL,
    PATH
  };


  // constructor
  Modal(int width, int height, const std::string& name, Mode mode);
  ~Modal();

  // set data
  void SetMessage(const std::string& message) {_message=message;};
  void SetTitle(const std::string& title) {_title=title;};
  void SetFolder(const std::string& folder) {_folder=folder;};

  // event loop
  void Loop();

private:
  Mode                      _mode;
  size_t                    _width;
  size_t                    _height;
  Window*                   _window;
  std::string               _message;
  std::string               _title;
  std::string               _folder;
  Status                    _status;
  std::vector<std::string>  _result;

};
AMN_NAMESPACE_CLOSE_SCOPE

#endif