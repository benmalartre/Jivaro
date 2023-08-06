#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usdLux/lightAPI.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/pointInstancer.h>

#include "../ui/tool.h"
#include "../utils/timer.h"
#include "../acceleration/hashGrid.h"
#include "../geometry/mesh.h"
#include "../geometry/subdiv.h"
#include "../geometry/tesselator.h"
#include "../geometry/voxels.h"
#include "../geometry/smooth.h"
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

static void
_SetupBVHInstancer(pxr::UsdStageRefPtr& stage, BVH* bvh)
{
  std::vector<BVH::Cell*> cells;
  bvh->GetRoot()->GetCells(cells);
  size_t numPoints = cells.size();
  pxr::VtArray<pxr::GfVec3f> points(numPoints);
  pxr::VtArray<pxr::GfVec3f> scales(numPoints);
  pxr::VtArray<int64_t> indices(numPoints);
  pxr::VtArray<int> protoIndices(numPoints);
  pxr::VtArray<pxr::GfQuath> rotations(numPoints);
  pxr::VtArray<pxr::GfVec3f> colors(numPoints);
  pxr::VtArray<int> curveVertexCount(1);
  curveVertexCount[0] = numPoints;
  pxr::VtArray<float> widths(numPoints);
  widths[0] = 1.f;

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = pxr::GfVec3f(cells[pointIdx]->GetSize());
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    colors[pointIdx] =
      pxr::GfVec3f(bvh->ComputeCodeAsColor(bvh->GetRoot(),
        pxr::GfVec3f(cells[pointIdx]->GetMidpoint())));
    rotations[pointIdx] = pxr::GfQuath::GetIdentity();
    widths[pointIdx] = RANDOM_0_1;
  }

  pxr::UsdGeomPointInstancer instancer =
    pxr::UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("bvh_instancer")));

  pxr::UsdGeomCube proto =
    pxr::UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(pxr::TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  pxr::UsdGeomPrimvarsAPI primvarsApi(instancer);
  pxr::UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(pxr::UsdGeomTokens->primvarsDisplayColor, pxr::SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(pxr::UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);



}

pxr::UsdGeomMesh _GetSelectedMesh()
{
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  Selection* selection = GetApplication()->GetSelection();
  if (selection->GetNumSelectedItems() > 0) {
    Selection::Item& item = selection->GetItem(0);
    pxr::UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<pxr::UsdGeomMesh>()) {
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

static void _CollapseEdges(float factor) 
{
  pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh);
    float l = mesh.GetAverageEdgeLength() * factor;
    std::cout << "average edge length = " << l << std::endl;
    Tesselator tesselator(&mesh);
    tesselator.Update(l);

    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::VtArray<int> faceCounts, faceConnects;
    tesselator.GetPositions(positions);
    tesselator.GetTopology(faceCounts, faceConnects);
    _SetMesh(usdMesh, positions, faceCounts, faceConnects);

  }
}

static void _Voxelize(float radius, short axis)
{
  pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh);
    Voxels voxels;
    voxels.Init(&mesh, radius);
    if(axis & 1) voxels.Trace(0);
    if(axis & 2) voxels.Trace(1);
    if(axis & 4) voxels.Trace(2);
    if(axis & 8) voxels.Proximity();
    voxels.Build();


    const pxr::VtArray<pxr::GfVec3f>& positions = voxels.GetPositions();
    pxr::UsdStageRefPtr stage = GetApplication()->GetStage();

    _SetupBVHInstancer(stage, voxels.GetTree());

    std::cout << "hash grid radius : " << (voxels.GetRadius() * 2.f) << std::endl;
    HashGrid grid(voxels.GetRadius() * 2.f);
    grid.Init({ (Geometry*)&voxels });

    size_t numPoints = positions.size();
    pxr::UsdGeomPoints points =
      pxr::UsdGeomPoints::Define(
        stage,
        stage->GetDefaultPrim().GetPath().AppendChild(pxr::TfToken("Voxels")));

    points.CreatePointsAttr().Set(pxr::VtValue(positions));

    pxr::VtArray<float> widths(numPoints);
    pxr::VtArray<pxr::GfVec3f> colors(numPoints);
    for (size_t p = 0; p < numPoints; ++p) {
      widths[p] = radius * 0.9f;
      colors[p] = grid.GetColor(positions[p]);
    }
    points.CreateWidthsAttr().Set(pxr::VtValue(widths));
    points.SetWidthsInterpolation(pxr::UsdGeomTokens->varying);
    //points.CreateNormalsAttr().Set(pxr::VtValue({pxr::GfVec3f(0, 1, 0)}));
    points.CreateDisplayColorAttr().Set(pxr::VtValue(colors));
    points.GetDisplayColorPrimvar().SetInterpolation(pxr::UsdGeomTokens->varying);

  }

}

static void _Smooth(int smoothIterations)
{
  pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh);
    size_t numPoints = mesh.GetNumPoints();
    const pxr::GfVec3f* positions = mesh.GetPositionsCPtr();
    pxr::VtFloatArray weights;
    Smooth<pxr::GfVec3f> smooth(numPoints, weights);
    const pxr::VtArray<pxr::VtArray<int>>& neighbors = mesh.GetNeighbors();

    for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
      smooth.SetDatas(pointIdx, positions[pointIdx]);
      smooth.SetNeighbors(pointIdx, neighbors[pointIdx].size(), &neighbors[pointIdx][0]);

    }

    smooth.Compute(smoothIterations);

    pxr::VtArray<GfVec3f> smoothed(numPoints);
    for(size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx)
      smoothed[pointIdx] = smooth.GetDatas(pointIdx);

    _SetMesh(usdMesh, smoothed, mesh.GetFaceCounts(), mesh.GetFaceConnects());
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

  static float randFactor = 0.1f;
  if (ImGui::Button("Randomize")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      mesh.Randomize(randFactor);
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }ImGui::SameLine();
  ImGui::InputFloat("Random", &randFactor);

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
      /*
      if (mesh.FlipEdge(mesh.GetEdge(edgeIdx))) {
        mesh.UpdateTopologyFromEdges();
        _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      }
      */
    }
  }

  static float factor = 0.95f;

  if (ImGui::Button("Collapse Edge")) {
    _CollapseEdges(factor);
  }
  ImGui::SameLine();
  ImGui::InputFloat("Radius", &factor);

  if (ImGui::Button("Add Light Edge")) {
  }


  if (ImGui::Button("Triangulate Face")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      mesh.TriangulateFace(0);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Triangle Pairs")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      mesh.ComputeTrianglePairs();
    }
  }

  if (ImGui::Button("Split Edge")) {
    pxr::UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh);
      mesh.SplitEdge(0);
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }

  static float voxelizeRadius = 0.5f;
  if (ImGui::Button("Voxelize X only")) {
    _Voxelize(voxelizeRadius, 1);
  }ImGui::SameLine();
  if (ImGui::Button("Voxelize Y only")) {
    _Voxelize(voxelizeRadius, 2);
  }ImGui::SameLine();
  if (ImGui::Button("Voxelize Z only")) {
    _Voxelize(voxelizeRadius, 4);
  };
  if (ImGui::Button("Voxelize XY only")) {
    _Voxelize(voxelizeRadius, 3);
  }ImGui::SameLine();
  if (ImGui::Button("Voxelize XZ only")) {
    _Voxelize(voxelizeRadius, 5);
  }ImGui::SameLine();
  if (ImGui::Button("Voxelize YZ only")) {
    _Voxelize(voxelizeRadius, 6);
  };
  if (ImGui::Button("Voxelize XYZ")) {
    _Voxelize(voxelizeRadius, 7);
  }ImGui::SameLine();
  ImGui::InputFloat("Radius", &voxelizeRadius);

  static int smoothIteration = 32;
  if (ImGui::Button("Smooth")) {
    _Smooth(smoothIteration);
  }ImGui::SameLine();
  ImGui::InputInt("##Iterations", &smoothIteration);

  
  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};
  

JVR_NAMESPACE_CLOSE_SCOPE