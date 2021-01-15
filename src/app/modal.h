#ifndef AMN_APPLICATION_MODAL_H
#define AMN_APPLICATION_MODAL_H
#pragma once

#include "../common.h"
#include "window.h"
#include "../ui/ui.h"


AMN_NAMESPACE_OPEN_SCOPE

// base class for modal window
class BaseModal
{
public:
  enum Status {
    ACTIVE,
    OK,
    CANCEL
  };


  // constructor
  BaseModal(int width, int height, const std::string& name);
  ~BaseModal();

  // get data
  Status GetStatus(){return _status;};

  // event loop
  virtual void Init();
  virtual void Term();
  virtual bool Loop();

private:
  size_t                    _width;
  size_t                    _height;
  Window*                   _window;
  BaseUI*                   _ui;
  std::string               _title;
  Status                    _status;
};

// file browser
class ModalFileBrowser : public BaseModal
{
public:
  std::string& GetResult(){return _result;};

private:
  std::string               _result;
  std::string               _folder;
};

// folder browser
class ModalFolderBrowser : public BaseModal
{
public:
  std::string& GetResult(){return _result;};

private:
  std::string               _result;
  std::string               _folder;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif