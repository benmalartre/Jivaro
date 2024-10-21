#ifndef JVR_PBD_MENU_H
#define JVR_PBD_MENU_H

JVR_NAMESPACE_OPEN_SCOPE

class MenuUI;

void AddPbdMenu(MenuUI* menu);

static void CreateSolverCallback();

static void AddClothCallback();
static void AddBodyAPICallback();
static void AddCollisionAPICallback();

static void RemoveClothCallback();
static void RemoveBodyAPICallback();
static void RemoveCollisionAPICallback();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MENU_H