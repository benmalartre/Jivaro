#include <pxr/usd/usd/stage.h>


#include "../utils/strings.h"
#include "../ui/menu.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../pbd/menu.h"
#include "../app/application.h"
#include "../app/commands.h"
#include "../app/callbacks.h"

#include <usdPbd/solver.h>
#include <usdPbd/bodyAPI.h>
#include <usdPbd/collisionAPI.h>
#include <usdPbd/constraintAPI.h>

JVR_NAMESPACE_OPEN_SCOPE

void AddPbdMenu(MenuUI* menu)
{

  MenuUI::Item* testItem = menu->Add("Pbd", false, true, NULL);

  MenuUI::Item* subItem = testItem->Add("Create Solver", false, true, std::bind(CreateSolverCallback));
  subItem = testItem->Add("Add Cloth", false, true, std::bind(AddClothCallback));
  subItem = testItem->Add("Add Body API", false, true, std::bind(AddBodyAPICallback));
  subItem = testItem->Add("Add Collision API", false, true, std::bind(AddCollisionAPICallback));

  subItem = testItem->Add("Remove Cloth", false, true, std::bind(RemoveClothCallback));
  subItem = testItem->Add("Remove Body API", false, true, std::bind(RemoveBodyAPICallback));
  subItem = testItem->Add("Remove Collision API", false, true, std::bind(RemoveCollisionAPICallback));

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
  Model* model = app->GetModel();
  Selection* selection = model->GetSelection();

  UndoBlock block;
  if(selection->GetNumSelectedItems())
    UsdPbdSolver::Define(model->GetStage(), (*selection)[0].path.AppendChild(TfToken("Solver")));
  else
    UsdPbdSolver::Define(model->GetStage(), SdfPath(TfToken("/Solver")));
  
}

void AddClothCallback()
{
  static const float spacing = 0.01f;
  Application* app = Application::Get();
  Model* model = app->GetModel();
  UsdStageRefPtr stage = model->GetStage();

  UndoBlock editBlock;
  UsdPrim prim;

  Selection* selection = model->GetSelection();
  if(!selection->GetNumSelectedItems()) {
    Mesh mesh;
    mesh.TriangularGrid2D(spacing);
    //mesh->RegularGrid2D(spacing);
    //mesh.Randomize(0.1f);

    // get root prim
    UsdPrim rootPrim = stage->GetDefaultPrim();
    if(!rootPrim.IsValid()) {
      UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
      rootPrim = root.GetPrim();
      stage->SetDefaultPrim(rootPrim);
    }
    
    SdfPath path = rootPrim.GetPath().AppendChild(TfToken("Cloth"+RandomString(6)));
    UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, path);

    usdMesh.CreatePointsAttr().Set(mesh.GetPositions(), UsdTimeCode::Default());
    usdMesh.CreateFaceVertexCountsAttr().Set(mesh.GetFaceCounts(), UsdTimeCode::Default());
    usdMesh.CreateFaceVertexIndicesAttr().Set(mesh.GetFaceConnects(), UsdTimeCode::Default());

    UsdPrim prim = usdMesh.GetPrim();
    mesh.SetPrim(prim);
    UsdPbdBodyAPI::Apply(prim);
    UsdPbdConstraintAPI::Apply(prim, TfToken("attach"));
    UsdPbdConstraintAPI::Apply(prim, TfToken("stretch"));
    UsdPbdConstraintAPI::Apply(prim, TfToken("shear"));
    UsdPbdConstraintAPI::Apply(prim, TfToken("dihedral"));

  }
  else {
    for(size_t s = 0; s < selection->GetNumSelectedItems(); ++s) {
      prim = stage->GetPrimAtPath((*selection)[0].path);
      UsdPbdBodyAPI::Apply(prim);
      UsdPbdConstraintAPI::Apply(prim, TfToken("attach"));
      UsdPbdConstraintAPI::Apply(prim, TfToken("stretch"));
      UsdPbdConstraintAPI::Apply(prim, TfToken("shear"));
      UsdPbdConstraintAPI::Apply(prim, TfToken("dihedral"));
    }
    
  }
}

void AddBodyAPICallback()
{
  Application* app = Application::Get();
  Model* model = app->GetModel();
  UsdStageRefPtr stage = model->GetStage();

  UsdPrim prim;

  Selection* selection = model->GetSelection();
  size_t numSelected = selection->GetNumSelectedItems();
  if(!numSelected)return;


  UndoBlock editBlock;

  for(size_t n = 0; n < numSelected; ++n) {
    prim = stage->GetPrimAtPath(selection->GetItem(n).path);
    if(!prim.IsValid())continue;
    UsdPbdBodyAPI::Apply(prim);
  }

}

void AddCollisionAPICallback()
{
  Application* app = Application::Get();
  Model* model = app->GetModel();
  UsdStageRefPtr stage = model->GetStage();

  UsdPrim prim;

  Selection* selection = model->GetSelection();
  size_t numSelected = selection->GetNumSelectedItems();
  if(!numSelected)return;


  UndoBlock editBlock;

  for(size_t n = 0; n < numSelected; ++n) {
    prim = stage->GetPrimAtPath(selection->GetItem(n).path);
    if(!prim.IsValid())continue;
    UsdPbdCollisionAPI::Apply(prim);
  }
}

void RemoveClothCallback()
{
  Application* app = Application::Get();
  Model* model = app->GetModel();
  UsdStageRefPtr stage = model->GetStage();

  UndoBlock editBlock;
  UsdPrim prim;

  Selection* selection = model->GetSelection();

  for(size_t s = 0; s < selection->GetNumSelectedItems(); ++s) {
    prim = stage->GetPrimAtPath((*selection)[0].path);
    if(prim.HasAPI<UsdPbdBodyAPI>())
      prim.RemoveAPI(UsdPbdTokens->PbdBodyAPI);
    
    if(prim.HasAPI<UsdPbdConstraintAPI>(TfToken("attach")))
      prim.RemoveAPI(UsdPbdTokens->PbdConstraintAPI, TfToken("attach"));
    
    if(prim.HasAPI<UsdPbdConstraintAPI>(TfToken("stretch")))
      prim.RemoveAPI(UsdPbdTokens->PbdConstraintAPI, TfToken("stretch"));
    
    if(prim.HasAPI<UsdPbdConstraintAPI>(TfToken("shear")))
      prim.RemoveAPI(UsdPbdTokens->PbdConstraintAPI, TfToken("shear"));
    
    if(prim.HasAPI<UsdPbdConstraintAPI>(TfToken("bend")))
      prim.RemoveAPI(UsdPbdTokens->PbdConstraintAPI, TfToken("bend"));

    if(prim.HasAPI<UsdPbdConstraintAPI>(TfToken("dihedral")))
      prim.RemoveAPI(UsdPbdTokens->PbdConstraintAPI, TfToken("dihedral"));
  }
}

void RemoveBodyAPICallback()
{

}

void RemoveCollisionAPICallback()
{

}


JVR_NAMESPACE_CLOSE_SCOPE