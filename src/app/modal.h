#ifndef AMN_APPLICATION_MODAL_H
#define AMN_APPLICATION_MODAL_H
#pragma once

#include "../common.h"
#include "window.h"
AMN_NAMESPACE_OPEN_SCOPE

class Modal : public Window
{
  enum Mode {
    FILE,
    FOLDER,
    WARNING,
    OK_CANCEL
  };
public:
  // constructor
  Modal(int width, int height, const std::string& name);
  Modal(bool fullscreen, const std::string& name);
  Modal(int width, int height, GLFWwindow* parent, const std::string& name);

};
AMN_NAMESPACE_CLOSE_SCOPE

#endif