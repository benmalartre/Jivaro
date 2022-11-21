#ifndef JVR_APPLICATION_MODAL_H
#define JVR_APPLICATION_MODAL_H
#pragma once

#include "../common.h"
#include "window.h"
#include "../ui/ui.h"


JVR_NAMESPACE_OPEN_SCOPE

// base class for modal window
class BaseModal
{
public:
  enum Status {
    ACTIVE,
    OK,
    CANCEL,
    FAIL
  };


  // constructor
  BaseModal(int x, int y, int width, int height, const std::string& title);
  virtual ~BaseModal();

  // get data
  Status GetStatus(){return _status;};

  // event loop
  void Init();
  void Term();
  void Loop();

  // callbacks
  virtual void _LoopImpl()=0;

protected:
  size_t        _x;
  size_t        _y;
  size_t        _width;
  size_t        _height;
  Window*       _window;
  BaseUI*       _ui;
  std::string   _title;
  Status        _status;
};

// file browser
class ModalFileBrowser : public BaseModal
{
public:
  enum Mode {
    OPEN,
    SAVE,
    SELECT,
    MULTI
  };

  ModalFileBrowser(int x, int y, const std::string& title, Mode mode);
  std::string& GetResult(){return _result;};

  void _LoopImpl() override;

private:
  std::string               _result;
  std::string               _folder;
  Mode                      _mode;
};

// folder browser
class ModalFolderBrowser : public BaseModal
{
public:
  ModalFolderBrowser(int x, int y, const std::string& title);
  std::string& GetResult(){return _result;};

  void _LoopImpl() override;

private:
  std::string               _result;
  std::string               _folder;
};

class ModalDemo : public BaseModal
{
public:
  ModalDemo(int x, int y, const std::string& title);
  void _LoopImpl() override;
};

class ModalMenu : public BaseModal
{
public:
  ModalMenu(int x, int y, const std::string& title);
  void _LoopImpl() override;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif