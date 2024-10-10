#include <pxr/usd/usd/stage.h>

#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

#include "../utils/strings.h"
#include "../ui/menu.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
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
  subItem = testItem->Add("Create Cloth", false, true, std::bind(CreateClothCallback));
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
  Selection* selection = app->GetSelection();

  UndoBlock block;
  if(selection->GetNumSelectedItems())
    pxr::UsdPbdSolver::Define(app->GetStage(), (*selection)[0].path.AppendChild(pxr::TfToken("Solver")));
  else
    pxr::UsdPbdSolver::Define(app->GetStage(), pxr::SdfPath(pxr::TfToken("/Solver")));
  
}

void CreateClothCallback()
{
  static const float spacing = 0.1f;
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetStage();


  UndoBlock editBlock;
  pxr::UsdPrim prim;

  Selection* selection = app->GetSelection();
  if(!selection->GetNumSelectedItems()) {
    Mesh mesh;
    mesh.TriangularGrid2D(spacing);
    //mesh->RegularGrid2D(spacing);
    //mesh.Randomize(0.1f);
    
    pxr::SdfPath path(pxr::TfToken("/Cloth"+RandomString(6)));
    pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

    usdMesh.CreatePointsAttr().Set(mesh.GetPositions(), pxr::UsdTimeCode::Default());
    usdMesh.CreateFaceVertexCountsAttr().Set(mesh.GetFaceCounts(), pxr::UsdTimeCode::Default());
    usdMesh.CreateFaceVertexIndicesAttr().Set(mesh.GetFaceConnects(), pxr::UsdTimeCode::Default());

    pxr::UsdPrim prim = usdMesh.GetPrim();
    mesh.SetPrim(prim);
    pxr::UsdPbdBodyAPI::Apply(prim);
    pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("Stretch"));
  }
  else {
    for(size_t s = 0; s < selection->GetNumSelectedItems(); ++s) {
      prim = stage->GetPrimAtPath((*selection)[0].path);
      pxr::UsdPbdBodyAPI::Apply(prim);
      pxr::UsdPbdConstraintAPI::Apply(prim, pxr::TfToken("Stretch"));
    }
    
  }
}

void AddBodyAPICallback()
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetStage();

  pxr::UsdPrim prim;

  Selection* selection = app->GetSelection();
  size_t numSelected = selection->GetNumSelectedItems();
  if(!numSelected)return;


  UndoBlock editBlock;

  for(size_t n = 0; n < numSelected; ++n) {
    prim = stage->GetPrimAtPath(selection->GetItem(n).path);
    if(!prim.IsValid())continue;
    pxr::UsdPbdBodyAPI::Apply(prim);
  }

}

void AddCollisionAPICallback()
{
  Application* app = Application::Get();
  pxr::UsdStageRefPtr stage = app->GetStage();

  pxr::UsdPrim prim;

  Selection* selection = app->GetSelection();
  size_t numSelected = selection->GetNumSelectedItems();
  if(!numSelected)return;


  UndoBlock editBlock;

  for(size_t n = 0; n < numSelected; ++n) {
    prim = stage->GetPrimAtPath(selection->GetItem(n).path);
    if(!prim.IsValid())continue;
    pxr::UsdPbdCollisionAPI::Apply(prim);
  }
}

void AddConstraintAPICallback()
{
  std::cout << "ADD CONSTRAINT API CALLED..." << std::endl;
}

JVR_NAMESPACE_CLOSE_SCOPE