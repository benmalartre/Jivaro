#include "../ui/tool.h"
#include "../geometry/mesh.h"
#include "../geometry/subdiv.h"
#include "../app/selection.h"
#include "../app/notice.h"
#include "../app/application.h"
#include "../command/block.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ToolUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;


ToolUI::ToolUI(View* parent)
  : BaseUI(parent, UIType::TOOL)
{
}

ToolUI::~ToolUI()
{
}

bool ToolUI::Draw()
{
  static bool opened = false;
  const pxr::GfVec2f pos(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);

  
  if (ImGui::Button("Create Random Mesh")) {
    
    Mesh mesh;
    mesh.Random2DPattern();
    //mesh.PolygonSoup(666, pxr::GfVec3f(-10.f), pxr::GfVec3f(10.f));
    pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, pxr::SdfPath("/RandomMesh"));
    usdMesh.CreatePointsAttr().Set(pxr::VtValue(mesh.GetPositions()));
    usdMesh.CreateFaceVertexCountsAttr().Set(pxr::VtValue(mesh.GetFaceCounts()));
    usdMesh.CreateFaceVertexIndicesAttr().Set(pxr::VtValue(mesh.GetFaceConnects()));
    stage->SetDefaultPrim(usdMesh.GetPrim());
    SceneChangedNotice().Send();
  }

  if (ImGui::Button("Smooth Mesh")) {
    Selection* selection = GetApplication()->GetSelection();
    if (selection->GetNumSelectedItems() > 0) {
      Selection::Item& item = selection->GetItem(0);
      pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
        UndoBlock block;
        pxr::UsdGeomMesh usdMesh(prim);
        Mesh mesh(usdMesh);
        _SubdivideMesh(&mesh, 1);

        usdMesh.GetPointsAttr().Set(pxr::VtValue(mesh.GetPositions()));
        usdMesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(mesh.GetFaceCounts()));
        usdMesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(mesh.GetFaceConnects()));
        SceneChangedNotice().Send();
      }
    }
  }

  if (ImGui::Button("Triangulate Mesh")) {
    Selection* selection = GetApplication()->GetSelection();
    if (selection->GetNumSelectedItems() > 0) {
      Selection::Item& item = selection->GetItem(0);
      pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
        UndoBlock block;
        pxr::UsdGeomMesh usdMesh(prim);
        Mesh mesh(usdMesh);
        mesh.Triangulate();

        usdMesh.GetPointsAttr().Set(pxr::VtValue(mesh.GetPositions()));
        usdMesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(mesh.GetFaceCounts()));
        usdMesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(mesh.GetFaceConnects()));
        SceneChangedNotice().Send();
      }
    }
  }

  if (ImGui::Button("Flip Edge")) {
    Selection* selection = GetApplication()->GetSelection();
    if (selection->GetNumSelectedItems() > 0) {
      Selection::Item& item = selection->GetItem(0);
      pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
      if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
        
        UndoBlock block;
        pxr::UsdGeomMesh usdMesh(prim);
        Mesh mesh(usdMesh);
        size_t edgeIdx = RANDOM_0_X(mesh.GetNumEdges()-1);
        if(mesh.FlipEdge(edgeIdx))mesh.UpdateTopologyFromEdges();

        usdMesh.GetPointsAttr().Set(pxr::VtValue(mesh.GetPositions()));
        usdMesh.GetFaceVertexCountsAttr().Set(pxr::VtValue(mesh.GetFaceCounts()));
        usdMesh.GetFaceVertexIndicesAttr().Set(pxr::VtValue(mesh.GetFaceConnects()));
        SceneChangedNotice().Send();
      }
    }
  }

  if (ImGui::Button("Collapse Edge")) {

  }

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};
  

JVR_NAMESPACE_CLOSE_SCOPE