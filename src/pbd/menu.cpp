#include <pxr/usd/usd/stage.h>

#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>

#include "../ui/menu.h"
#include "../geometry/geometry.h"
#include "../pbd/menu.h"
#include "../app/application.h"
#include "../app/commands.h"

JVR_NAMESPACE_OPEN_SCOPE

void AddPbdMenu(MenuUI* menu)
{
  std::cout << "ADD PBD MENU CALLED..." << std::endl;

  MenuUI::Item* testItem = menu->Add("Pbd", false, true, NULL);
  /*
  testItem->Add("Create Cube", false, true, std::bind(CreatePrimCallback, Geometry::CUBE));
  testItem->Add("Create Sphere", false, true, std::bind(CreatePrimCallback, Geometry::SPHERE));
  testItem->Add("Create Capsule", false, true, std::bind(CreatePrimCallback, Geometry::CAPSULE));
  testItem->Add("Create Cone", false, true, std::bind(CreatePrimCallback, Geometry::CONE));
  */

  MenuUI::Item* subItem = testItem->Add("Create Solver", false, true, std::bind(CreateSolverCallback));
  subItem = testItem->Add("Add Body API", false, true, std::bind(AddBodyAPICallback));
  subItem = testItem->Add("Add Collision API", false, true, std::bind(AddCollisionAPICallback));
  subItem = testItem->Add("Add Constraint API", false, true, std::bind(AddConstraintAPICallback));

  /*
  subItem->Add("Sub0", false, true, NULL);
  subItem->Add("Sub1", false, true, NULL);
  MenuUI::Item* subSubItem = subItem->Add("Sub2", false, true, NULL);

  subSubItem->Add("SubSub0", false, true, NULL);
  subSubItem->Add("SubSub1", true, true, NULL);
  subSubItem->Add("SubSub2", false, false, NULL);
  subSubItem->Add("SubSub3", true, false, NULL);
  subSubItem->Add("SubSub4", false, true, NULL);
  subSubItem->Add("SubSub5", false, true, NULL);
  */
}

void CreateSolverCallback()
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetStage();
  pxr::SdfLayerHandle layer = stage->GetSessionLayer();

  pxr::SdfPath name(RandomString(32));
  ADD_COMMAND(CreatePrimCommand, layer, name, Geometry::SOLVER);
}

void CreateClothCallback()
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetStage();
  pxr::SdfLayerHandle layer = stage->GetSessionLayer();

  pxr::SdfPath name(RandomString(32));
  ADD_COMMAND(CreatePrimCommand, layer, name, Geometry::SOLVER);
}

void AddBodyAPICallback()
{
  std::cout << "ADD BODY API CALLED..." << std::endl;
}

void AddCollisionAPICallback()
{
  std::cout << "ADD COLIISION API CALLED..." << std::endl;
}

void AddConstraintAPICallback()
{
  std::cout << "ADD CONSTRAINT API CALLED..." << std::endl;
}

JVR_NAMESPACE_CLOSE_SCOPE