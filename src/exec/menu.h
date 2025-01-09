#ifndef JVR_EXEC_MENU_H
#define JVR_EXEC_MENU_H

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class MenuUI;

void AddExecMenu(MenuUI* menu);

static void CreateSolverCallback();

static void SetExecCallback(const std::string &name);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_EXEC_MENU_H