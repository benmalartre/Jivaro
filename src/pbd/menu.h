#ifndef JVR_PBD_MENU_H
#define JVR_PBD_MENU_H

JVR_NAMESPACE_OPEN_SCOPE

class MenuUI;

void AddPbdMenu(MenuUI* menu);

static void CreateSolverCallback();
static void AddBodyAPICallback();
static void AddCollisionAPICallback();
static void AddConstraintAPICallback();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MENU_H