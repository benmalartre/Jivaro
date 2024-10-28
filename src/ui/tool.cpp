#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usdLux/lightAPI.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/xform.h>
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

UsdGeomMesh _GetSelectedMesh()
{
  UsdStageRefPtr stage = Application::Get()->GetModel()->GetStage();
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  if (selection->GetNumSelectedItems() > 0) {
    Selection::Item& item = selection->GetItem(0);
    UsdPrim prim = stage->GetPrimAtPath(item.path);
    if (prim.IsValid() && prim.IsA<UsdGeomMesh>()) {
      return UsdGeomMesh(prim);
    }
  }
  return UsdGeomMesh();
}

static void _SetRandomColor()
{
  UsdGeomMesh usdMesh = _GetSelectedMesh();
  UsdGeomPrimvarsAPI primvarsApi(usdMesh);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->constant);
  colorPrimvar.SetElementSize(1);
  pxr::VtArray<pxr::GfVec3f> colors = { {RANDOM_0_1, RANDOM_0_1, RANDOM_0_1} };
  colorPrimvar.Set(colors);

  SceneChangedNotice().Send();
}

static void
_SetupBVHInstancer(UsdStageRefPtr& stage, BVH* bvh)
{
  std::vector<const BVH::Cell*> cells;
  bvh->GetBranches(bvh->GetRoot(), cells);
  size_t numPoints = cells.size();
  VtArray<GfVec3f> points(numPoints);
  VtArray<GfVec3f> scales(numPoints);
  VtArray<int64_t> indices(numPoints);
  VtArray<int> protoIndices(numPoints);
  VtArray<GfQuath> rotations(numPoints);
  VtArray<GfVec3f> colors(numPoints);
  VtArray<int> curveVertexCount(1);
  curveVertexCount[0] = numPoints;
  VtArray<float> widths(numPoints);
  widths[0] = 1.f;

  for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
    points[pointIdx] = GfVec3f(cells[pointIdx]->GetMidpoint());
    scales[pointIdx] = GfVec3f(cells[pointIdx]->GetSize());
    protoIndices[pointIdx] = 0;
    indices[pointIdx] = pointIdx;
    colors[pointIdx] = bvh->GetMortonColor(GfVec3f(cells[pointIdx]->GetMidpoint()));
    rotations[pointIdx] = GfQuath::GetIdentity();
    widths[pointIdx] = RANDOM_0_1;
  }

  UsdGeomPointInstancer instancer =
    UsdGeomPointInstancer::Define(stage,
      stage->GetDefaultPrim().GetPath().AppendChild(TfToken("bvh_instancer")));

  UsdGeomCube proto =
    UsdGeomCube::Define(stage,
      instancer.GetPath().AppendChild(TfToken("proto_cube")));
  proto.CreateSizeAttr().Set(1.0);

  instancer.CreatePositionsAttr().Set(points);
  instancer.CreateProtoIndicesAttr().Set(protoIndices);
  instancer.CreateScalesAttr().Set(scales);
  instancer.CreateIdsAttr().Set(indices);
  instancer.CreateOrientationsAttr().Set(rotations);
  instancer.CreatePrototypesRel().AddTarget(proto.GetPath());
  UsdGeomPrimvarsAPI primvarsApi(instancer);
  UsdGeomPrimvar colorPrimvar =
    primvarsApi.CreatePrimvar(UsdGeomTokens->primvarsDisplayColor, SdfValueTypeNames->Color3fArray);
  colorPrimvar.SetInterpolation(UsdGeomTokens->varying);
  colorPrimvar.SetElementSize(1);
  colorPrimvar.Set(colors);



}


static void _SetMesh(UsdGeomMesh& mesh, const VtVec3fArray& points,
  const VtIntArray& faceCounts, const VtIntArray& faceConnects)
{
  mesh.CreatePointsAttr().Set(VtValue(points));
  mesh.CreateFaceVertexCountsAttr().Set(VtValue(faceCounts));
  mesh.CreateFaceVertexIndicesAttr().Set(VtValue(faceConnects));
  SceneChangedNotice().Send();
}

static void _CollapseEdges(float factor) 
{
  UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(UsdTimeCode::Default()));
    float l = mesh.ComputeAverageEdgeLength() * factor;
    std::cout << "average edge length = " << l << std::endl;
    Tesselator tesselator(&mesh);
    tesselator.Update(l);

    VtArray<GfVec3f> positions;
    VtArray<int> faceCounts, faceConnects;
    tesselator.GetPositions(positions);
    tesselator.GetTopology(faceCounts, faceConnects);
    _SetMesh(usdMesh, positions, faceCounts, faceConnects);

  }
}

static void _Voxelize(float radius, short axis)
{
  UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(UsdTimeCode::Default()));
    Voxels voxels;
    voxels.Init(&mesh, radius);
    if(axis & 1) voxels.Trace(0);
    if(axis & 2) voxels.Trace(1);
    if(axis & 4) voxels.Trace(2);
    if(axis & 8) voxels.Proximity();
    voxels.Build();


    const VtArray<GfVec3f>& positions = voxels.GetPositions();
    UsdStageRefPtr stage = Application::Get()->GetModel()->GetStage();

    _SetupBVHInstancer(stage, voxels.GetTree());

    std::cout << "hash grid radius : " << (voxels.GetRadius() * 2.f) << std::endl;
    HashGrid grid(voxels.GetRadius() * 2.f);
    //grid.Init({ (Geometry*)&voxels });

    size_t numPoints = positions.size();
    UsdGeomPoints points =
      UsdGeomPoints::Define(
        stage,
        stage->GetDefaultPrim().GetPath().AppendChild(TfToken("Voxels")));

    points.CreatePointsAttr().Set(VtValue(positions));

    VtArray<float> widths(numPoints);
    VtArray<GfVec3f> colors(numPoints);
    for (size_t p = 0; p < numPoints; ++p) {
      widths[p] = radius * 0.9f;
      colors[p] = grid.GetColor(positions[p]);
    }
    points.CreateWidthsAttr().Set(VtValue(widths));
    points.SetWidthsInterpolation(UsdGeomTokens->varying);
    //points.CreateNormalsAttr().Set(VtValue({GfVec3f(0, 1, 0)}));
    points.CreateDisplayColorAttr().Set(VtValue(colors));
    points.GetDisplayColorPrimvar().SetInterpolation(UsdGeomTokens->varying);

  }

}

static void _Smooth(int smoothIterations)
{
  UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    TfStopwatch sw;
    sw.Start();
    Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(UsdTimeCode::Default()), Mesh::ADJACENTS);
    sw.Stop();

    sw.Reset();
    sw.Start();
    size_t numPoints = mesh.GetNumPoints();
    const GfVec3f* positions = mesh.GetPositionsCPtr();
    VtFloatArray weights;
    Smooth<GfVec3f> smooth(numPoints, weights);

    for (size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx) {
      smooth.SetDatas(pointIdx, positions[pointIdx]);
      smooth.SetNeighbors(pointIdx, mesh.GetNumAdjacents(pointIdx), mesh.GetAdjacents(pointIdx));

    }
    sw.Stop();

    sw.Reset();
    sw.Start();
    smooth.Compute(smoothIterations);
    sw.Stop();


    VtArray<GfVec3f> smoothed(numPoints);
    for(size_t pointIdx = 0; pointIdx < numPoints; ++pointIdx)
      smoothed[pointIdx] = smooth.GetDatas(pointIdx);

    sw.Reset();
    sw.Start();
    UndoBlock block;
    _SetMesh(usdMesh, smoothed, mesh.GetFaceCounts(), mesh.GetFaceConnects());
    sw.Stop();
  }

}

void _CreateVertexColor()
{
  UsdGeomMesh usdMesh = _GetSelectedMesh();
  if (usdMesh.GetPrim().IsValid()) {
    UndoBlock block;
    Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(UsdTimeCode::Default()));
    size_t numPoints = mesh.GetNumPoints();
    UsdGeomMesh usdMesh(mesh.GetPrim());
    UsdGeomPrimvar colorPrimVar = usdMesh.GetDisplayColorPrimvar();
    if(!colorPrimVar) 
      colorPrimVar = usdMesh.CreateDisplayColorPrimvar(
        UsdGeomTokens->vertex, numPoints);

    colorPrimVar.SetInterpolation(UsdGeomTokens->vertex);

    VtArray<GfVec3f> colors(numPoints);
    for(auto& color: colors)color = GfVec3f(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
    UsdAttribute colorsAttr = colorPrimVar.GetAttr();
    colorsAttr.Set(colors);
  }
}

bool ToolUI::Draw()
{
  static bool opened = false;
  const GfVec2f pos(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  UsdStageRefPtr stage = Application::Get()->GetModel()->GetStage();
  Time* time = Time::Get();
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);

  if (ImGui::Button("Create Root Prim")) {
    UndoBlock block;
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    stage->SetDefaultPrim(root.GetPrim());
  } ImGui::SameLine();
  if (ImGui::Button("Create Random Mesh")) {
    Mesh mesh;
    mesh.Random2DPattern(32);
    UndoBlock block;
    //mesh.PolygonSoup(666, GfVec3f(-10.f), GfVec3f(10.f));
    UsdPrim defaultPrim = stage->GetDefaultPrim();
    if (defaultPrim.IsValid()) {
      UsdGeomMesh usdMesh = 
        UsdGeomMesh::Define(stage, 
          defaultPrim.GetPath().AppendChild(TfToken("RandomMesh")));
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
    else {
      UndoBlock block;
      UsdGeomMesh usdMesh = UsdGeomMesh::Define(stage, SdfPath("/RandomMesh"));
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
      stage->SetDefaultPrim(usdMesh.GetPrim());
    }
  } 

  static float randFactor = 0.1f;
  UsdTimeCode timeCode(time->GetActiveTime());
  if (ImGui::Button("Randomize")) {
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
      mesh.Randomize(randFactor);
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }ImGui::SameLine();
  ImGui::InputFloat("Random", &randFactor);

  if (ImGui::Button("Smooth Mesh")) {
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if(usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
      std::cout << "subdivide ..." << std::endl;
      _SubdivideMesh(&mesh, 1);
      std::cout << "subdiv done !" << std::endl;
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Create Vertex Color")) {
    _CreateVertexColor();
  }

  ImGui::SameLine();
  if (ImGui::Button("Random Display Color")) {
    _SetRandomColor();
  }

  if (ImGui::Button("Triangulate Mesh")) {
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
      mesh.Triangulate();
      _SetMesh(usdMesh, mesh.GetPositions(), mesh.GetFaceCounts(), mesh.GetFaceConnects());
    }
  }

  if (ImGui::Button("Flip Edge")) {
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
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
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
      mesh.TriangulateFace(0);
    }
  }

  if (ImGui::Button("Split Edge")) {
    UsdGeomMesh usdMesh = _GetSelectedMesh();
    if (usdMesh.GetPrim().IsValid()) {
      UndoBlock block;
      Mesh mesh(usdMesh, usdMesh.ComputeLocalToWorldTransform(timeCode));
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
  if (ImGui::Button("Voxelize Surface")) {
    _Voxelize(voxelizeRadius, 8);
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