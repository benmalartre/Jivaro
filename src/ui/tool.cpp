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

pxr::UsdGeomMesh _GetSelectedMesh()
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  Selection* selection = GetApplication()->GetSelection();
  if (selection->GetNumSelectedItems() > 0) {
    Selection::Item& item = selection->GetItem(0);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
      std::cout << "valid mesh selected : " << prim.GetPath() << std::endl;
      return pxr::UsdGeomMesh(prim);
    }
  }
  return pxr::UsdGeomMesh();
}

static void _SetMesh(pxr::UsdGeomMesh& mesh, const pxr::VtVec3fArray& points,
  const pxr::VtIntArray& faceCounts, const pxr::VtIntArray& faceConnects)
{
  mesh.CreatePointsAttr().Set(pxr::VtValue(points));
  mesh.CreateFaceVertexCountsAttr().Set(pxr::VtValue(faceCounts));
  mesh.CreateFaceVertexIndicesAttr().Set(pxr::VtValue(faceConnects));
  SceneChangedNotice().Send();
}

struct Dummy_t {
  Dummy_t* previous;
  Dummy_t* next;
};

static void _TestVtArray()
{
  size_t n = 8;
  pxr::VtArray<Dummy_t> dummys(n);

  for (size_t i = 0; i < n; ++i) {
    if (i == 0) {
      dummys[i].previous = NULL;
      dummys[i].next = &dummys[i + 1];
    }
    else if (i == n - 1) {
      dummys[i].previous = &dummys[i - 1];
      dummys[i].next = NULL;
    }
    else {
      dummys[i].previous = &dummys[i - 1];
      dummys[i].next = &dummys[i + 1];
    }
  }
  for (auto& dummy : dummys) {
    std::cout << &dummy << " : previous = " << dummy.previous << ", next = " << dummy.next << std::endl;
  }

  dummys.erase(dummys.begin() + n /2);
  std::cout << "--------------------------------------------------------------------------------------" << std::endl;
  for (auto& dummy : dummys) {
    std::cout << &dummy << " : previous = " << dummy.previous << ", next = " << dummy.next << std::endl;
  }
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
    UndoBlock block;
    //mesh.PolygonSoup(666, pxr::GfVec3f(-10.f), pxr::GfVec3f(10.f));
    pxr::UsdPrim defaultPrim = stage->GetDefaultPrim();
    if (defaultPrim.IsValid()) {
      pxr::UsdGeomMesh usdMesh = 
        pxr::UsdGeomMesh::Define(stage, 
          defaultPrim.GetPath().AppendChild(pxr::TfToken("RandomMesh")));
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
    else {
      pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, pxr::SdfPath("/RandomMesh"));
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      stage->SetDefaultPrim(usdMesh.GetPrim());
    }
  }

  if (ImGui::Button("Smooth Mesh")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if(usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      _SubdivideMesh(&mesh, 1);
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }

  if (ImGui::Button("Triangulate Mesh")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      mesh.Triangulate();
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }

  if (ImGui::Button("Flip Edge")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      size_t edgeIdx = RANDOM_0_X(mesh.GetNumEdges()-1);
      if (mesh.FlipEdge(edgeIdx)) {
        mesh.UpdateTopologyFromEdges();
        _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      }
    }
  }

  if (ImGui::Button("Collapse Edge")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      size_t edgeIdx = RANDOM_0_1 * (mesh.GetNumEdges() - 1);
      if (mesh.CollapseEdge(edgeIdx)) {
        mesh.UpdateTopologyFromEdges();
        _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      }
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("Collapse Edge X10")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      bool updated = false;
      for (size_t i = 0; i < 10; ++i) {
        size_t edgeIdx = RANDOM_0_1 * (mesh.GetNumEdges() - 1);
        if (mesh.CollapseEdge(edgeIdx)) {
          updated = true;
        }
      }
      if (updated) {
        mesh.UpdateTopologyFromEdges();
        _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      }
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("Collapse Edge X100")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      bool updated = false;
      for (size_t i = 0; i < 100; ++i) {
        size_t edgeIdx = RANDOM_0_X(mesh.GetNumEdges() - 1);
        if (mesh.CollapseEdge(edgeIdx)) {
          updated = true;
        }
      }
      if (updated) {
        mesh.UpdateTopologyFromEdges();
        _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      }
    }
  }
  
  if (ImGui::Button("Test VtArray")) {
    _TestVtArray();
  }

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};
  

JVR_NAMESPACE_CLOSE_SCOPE