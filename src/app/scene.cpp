
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

#include "../utils/strings.h"
#include "../utils/files.h"
#include "../utils/timer.h"
#include "../geometry/utils.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/voxels.h"
#include "../geometry/sampler.h"
#include "../pbd/force.h"
#include "../pbd/solver.h"
#include "../pbd/collision.h"
#include "../pbd/constraint.h"
#include "../app/scene.h"
#include "../app/application.h"
#include "../app/commands.h"

JVR_NAMESPACE_OPEN_SCOPE

Scene::Scene()
{
}

Scene::~Scene()
{
}

void 
Scene::Init(const pxr::UsdStageRefPtr& stage)
{

}

void
Scene::Update(const pxr::UsdStageRefPtr& stage, double time)
{
}

void
Scene::Save(const std::string& filename)
{

}

void 
Scene::Export(const std::string& filename)
{
  
}



Mesh* Scene::AddMesh(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  _prims[path] = { new Mesh() };
  return (Mesh*)_prims[path].geom;
}

Voxels* Scene::AddVoxels(const pxr::SdfPath& path, Mesh* mesh, float radius)
{
  _prims[path] = { new Voxels() };
  Voxels* voxels = (Voxels*)_prims[path].geom;
  voxels->Init(mesh, radius);
  voxels->Trace(0);
  voxels->Trace(1);
  voxels->Trace(2);
  voxels->Build();
  return voxels;
}

Curve* Scene::AddCurve(const pxr::SdfPath & path, const pxr::GfMatrix4d & xfo)
{
  _prims[path] = { new Curve() };
  return (Curve*)_prims[path].geom;
}

Points* Scene::AddPoints(const pxr::SdfPath& path, const pxr::GfMatrix4d& xfo)
{
  _prims[path] = { new Points() };
  return (Points*)_prims[path].geom;
}

void Scene::Remove(const pxr::SdfPath & path)
{
  const auto& primIt = _prims.find(path);
  if (primIt != _prims.end()) {
    Geometry* geometry = primIt->second.geom;
    _prims.erase(primIt);
    delete geometry;
  }
}

Geometry*
Scene::GetGeometry(const pxr::SdfPath& path)
{
  if (_prims.find(path) != _prims.end()) {
    return _prims[path].geom;
  }
  return NULL;
}

// -----------------------------------------------------------------------//
/// \name Rprim Aspects
// -----------------------------------------------------------------------//

pxr::HdMeshTopology
Scene::GetMeshTopology(pxr::SdfPath const& id)
{
  if(IsMesh(id)) {
    Mesh* mesh = (Mesh*)_prims[id].geom;
    return pxr::HdMeshTopology(
      pxr::UsdGeomTokens->catmullClark,
      pxr::UsdGeomTokens->rightHanded,
      mesh->GetFaceCounts(),
      mesh->GetFaceConnects());
  }
  return pxr::HdMeshTopology();
}

pxr::HdBasisCurvesTopology
Scene::GetBasisCurvesTopology(pxr::SdfPath const& id)
{  
  if (IsCurves(id)) {
    Curve* curve = (Curve*)_prims[id].geom;
    return pxr::HdBasisCurvesTopology(
      pxr::HdTokens->linear,
      pxr::TfToken(),
      pxr::HdTokens->nonperiodic,
      curve->GetCvCounts(), pxr::VtArray<int>());
  }
  return pxr::HdBasisCurvesTopology();
}

pxr::GfRange3d 
Scene::GetExtent(pxr::SdfPath const& id)
{
  pxr::GfRange3d range;
  pxr::VtVec3fArray points;
  if (_prims.find(id) != _prims.end()) {
    points = _prims[id].geom->GetPositions();
  }
 
  TF_FOR_ALL(it, points) {
    range.UnionWith(*it);
  }
  return range;
}


pxr::GfMatrix4d
Scene::GetTransform(pxr::SdfPath const & id)
{
    return pxr::GfMatrix4d(1);
}

bool 
Scene::IsMesh(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::MESH);
}

bool
Scene::IsCurves(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::CURVE);
}

bool
Scene::IsPoints(const pxr::SdfPath& id)
{
  return (_prims.find(id) != _prims.end() && _prims[id].geom->GetType() == Geometry::POINT);
}

pxr::TfToken
Scene::GetRenderTag(pxr::SdfPath const& id)
{
  if(_prims.find(id)!=_prims.end()) {
    return pxr::HdRenderTagTokens->geometry;
  }
  return pxr::HdRenderTagTokens->hidden;
}

pxr::VtValue
Scene::Get(pxr::SdfPath const& id, pxr::TfToken const& key)
{
  if (key == pxr::HdTokens->points) {
    return pxr::VtValue(_prims[id].geom->GetPositions());
  } else if (key == pxr::HdTokens->displayColor) {
    pxr::VtArray<pxr::GfVec3f>& colors(_prims[id].geom->GetColors());
    if(colors.size())
      return pxr::VtValue(colors);
    else {
      const pxr::GfVec3f& wirecolor = _prims[id].geom->GetWirecolor();
      colors = { wirecolor };
      return pxr::VtValue(colors);
    }
  } else if (key == pxr::HdTokens->widths) {
    return pxr::VtValue(_prims[id].geom->GetRadius());
  }
  return pxr::VtValue();
}

static void _InitControls()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  pxr::UsdPrim controlPrim = stage->DefinePrim(rootPrim.GetPath().AppendChild(pxr::TfToken("Controls")));
  controlPrim.CreateAttribute(pxr::TfToken("Density"), pxr::SdfValueTypeNames->Int).Set(10000);
  controlPrim.CreateAttribute(pxr::TfToken("Radius"), pxr::SdfValueTypeNames->Float).Set(0.1f);
  controlPrim.CreateAttribute(pxr::TfToken("Length"), pxr::SdfValueTypeNames->Float).Set(4.f);
  controlPrim.CreateAttribute(pxr::TfToken("Scale"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Amplitude"), pxr::SdfValueTypeNames->Float).Set(0.5f);
  controlPrim.CreateAttribute(pxr::TfToken("Frequency"), pxr::SdfValueTypeNames->Float).Set(1.f);
  controlPrim.CreateAttribute(pxr::TfToken("Width"), pxr::SdfValueTypeNames->Float).Set(0.1f);
}

void _GenerateSample(pxr::UsdGeomMesh& mesh, pxr::VtArray<Sample>* samples, float minRadius=0.1, size_t density=1024)
{
  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<pxr::GfVec3f> normals;
  pxr::VtArray<int> counts;
  pxr::VtArray<int> indices;
  pxr::VtArray<Triangle> triangles;

  mesh.GetPointsAttr().Get(&positions);
  mesh.GetFaceVertexCountsAttr().Get(&counts);
  mesh.GetFaceVertexIndicesAttr().Get(&indices);

  TriangulateMesh(counts, indices, triangles);
  ComputeVertexNormals(positions, counts, indices, triangles, normals);
}

void _GenerateClothMesh(const pxr::SdfPath& path, float size, const pxr::GfMatrix4f& m)
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  Mesh mesh;
  mesh.TriangularGrid2D(10.f, 10.f, m, size);
  std::cout << "triangular grid infos : " << std::endl;
  std::cout << "num points : " << mesh.GetNumPoints() << std::endl;
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path);

  usdMesh.CreatePointsAttr().Set(mesh.GetPositions());
  usdMesh.CreateFaceVertexCountsAttr().Set(mesh.GetFaceCounts());
  usdMesh.CreateFaceVertexIndicesAttr().Set(mesh.GetFaceConnects());
}

static pxr::HdDirtyBits _HairEmit(Curve* curve, pxr::UsdGeomMesh& mesh, pxr::GfMatrix4d& xform, double time)
{
  uint64_t T = CurrentTime();
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();

  pxr::HdDirtyBits bits = pxr::HdChangeTracker::Clean;
  int density;
  float length, amplitude, frequency, width, scale, radius;
  pxr::UsdPrim controlPrim = rootPrim.GetChild(pxr::TfToken("Controls"));

  controlPrim.GetAttribute(pxr::TfToken("Density")).Get(&density);
  controlPrim.GetAttribute(pxr::TfToken("Radius")).Get(&radius);
  controlPrim.GetAttribute(pxr::TfToken("Length")).Get(&length);
  controlPrim.GetAttribute(pxr::TfToken("Scale")).Get(&scale);
  controlPrim.GetAttribute(pxr::TfToken("Amplitude")).Get(&amplitude);
  controlPrim.GetAttribute(pxr::TfToken("Frequency")).Get(&frequency);
  controlPrim.GetAttribute(pxr::TfToken("Width")).Get(&width);

  pxr::VtArray<pxr::GfVec3f> positions;
  pxr::VtArray<pxr::GfVec3f> normals;
  pxr::VtArray<int> counts;
  pxr::VtArray<int> indices;
  pxr::VtArray<Triangle> triangles;
  pxr::VtArray<Sample> samples;

  mesh.GetPointsAttr().Get(&positions);
  mesh.GetFaceVertexCountsAttr().Get(&counts);
  mesh.GetFaceVertexIndicesAttr().Get(&indices);

  //uint64_t T1 = CurrentTime() - T;
  //T = CurrentTime();
  TriangulateMesh(counts, indices, triangles);
  //uint64_t T2 = CurrentTime() - T;
  //T = CurrentTime();
  ComputeVertexNormals(positions, counts, indices, triangles, normals);
  //uint64_t T3 = CurrentTime() - T;
  //T = CurrentTime();
  PoissonSampling(radius, density, positions, normals, triangles, samples);
  //uint64_t T4 = CurrentTime() - T;
  //T = CurrentTime();  

  size_t numCVs = 4 * samples.size();

  pxr::VtArray<pxr::GfVec3f> points(numCVs);
  pxr::VtArray<float> radii(numCVs);
  pxr::VtArray<int> cvCounts(samples.size());
  
  for (size_t sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
    
    const pxr::GfVec3f& normal = samples[sampleIdx].GetNormal(&normals[0]);
    const pxr::GfVec3f& tangent = samples[sampleIdx].GetTangent(&positions[0], &normals[0]);
    const pxr::GfVec3f bitangent = (normal ^ tangent).GetNormalized();
    const pxr::GfVec3f& position = samples[sampleIdx].GetPosition(&positions[0]);
    const float tangentFactor =  pxr::GfCos(position[2] * scale + time * frequency)* amplitude;
    
    
    points[sampleIdx * 4] = xform.Transform(position);
    points[sampleIdx * 4 + 1] = xform.Transform(position + normal * 0.33 * length + bitangent * tangentFactor * 0.2);
    points[sampleIdx * 4 + 2] = xform.Transform(position + normal * 0.66 * length + bitangent * tangentFactor * 0.6);
    points[sampleIdx * 4 + 3] = xform.Transform(position + normal * length + bitangent * tangentFactor);

    radii[sampleIdx * 4] = width * 1.f;
    radii[sampleIdx * 4 + 1] = width * 0.8f;
    radii[sampleIdx * 4 + 2] = width * 0.4f;
    radii[sampleIdx * 4 + 3] = width * 0.2f;
    
    cvCounts[sampleIdx] = 4;
  }
  //uint64_t T5 = CurrentTime() - T; 
  //T = CurrentTime();

  curve->SetTopology(points, radii, cvCounts);

  //uint64_t T6 = CurrentTime() - T;
  //T = CurrentTime();

/*
  if (!((int)pxr::GfAbs(time*60) % 60)) {
    std::cout << "----------------- stochatics: " << std::endl;
    std::cout << "nb samples " << samples.size() << std::endl;
    std::cout << "read : " << (double)(T1 * 1e-9) << " seconds" << std::endl;
    std::cout << "triangulate : " << (double)(T2 * 1e-9) << " seconds" << std::endl;
    std::cout << "normals : " << (double)(T3 * 1e-9) << " seconds" << std::endl;
    std::cout << "samples : " << (double)(T4 * 1e-9) << " seconds" << std::endl;
    std::cout << "generate : " << (double)(T5 * 1e-9) << " seconds" << std::endl;
    std::cout << "write : " << (double)(T6 * 1e-9) << " seconds" << std::endl;

    std::cout << "-----------------  results: " << std::endl;
    std::cout << "total time : " << (double)((T1 + T2 + T3 + T4 + T5 + T6) * 1e-9) << " seconds" << std::endl;
  }
*/
  return pxr::HdChangeTracker::DirtyTopology;
}

void
Scene::InitExec()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  _solver = new Solver();
  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("Solver"));

  pxr::GfQuatf rotation(90.f*DEGREES_TO_RADIANS, pxr::GfVec3f(0, 0, 1));
  pxr::GfMatrix4f matrix = 
    pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(0.f, 1.f, 0.f)) *
    pxr::GfMatrix4f(1.f).SetScale(pxr::GfVec3f(10.f));
  float size = 0.2f;
  
  pxr::SdfPath clothPath = rootId.AppendChild(pxr::TfToken( "cloth"));
  _GenerateClothMesh(clothPath, size, matrix);
  
  
  _Sources sources;
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      Mesh mesh(usdMesh, xformCache.GetLocalToWorldTransform(prim));
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Body* body = _solver->AddBody((Geometry*)&mesh, pxr::GfMatrix4f(xform));
      _solver->AddConstraints(body);

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      Points points(usdPoints, xformCache.GetLocalToWorldTransform(prim));
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      _solver->AddBody((Geometry*)&points, pxr::GfMatrix4f(xform));

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    }
  }
  _solver->AddForce(new GravitationalForce());
  _solver->LockPoints();
  
  //_solver->AddForce(new DampingForce());
  /*
  pxr::GfVec3f pos;
  for (size_t x = 0; x < 12; ++x) {
    pxr::GfMatrix4f m(1.f);
    m.SetTranslate(pxr::GfVec3f(0.f, RANDOM_0_X(5) - 2.5, (2.f * x) - 6.f));
    _solver->AddCollision(new SphereCollision(m, 5.f));
  }
  */
 
  _solver->AddCollision(new PlaneCollision());
 

  pxr::SdfPath pointsPath(rootId.AppendChild(pxr::TfToken("Display")));
  _sourcesMap[pointsPath] = sources;
  Points* points = AddPoints(pointsPath);
  Particles* particles = _solver->GetParticles();
  const size_t numParticles = _solver->GetNumParticles();
  points->SetPositions(&particles->position[0], numParticles);
  points->SetRadii(&particles->radius[0], numParticles);

  

  pxr::VtArray<Constraint*> constraints;
  _solver->GetConstraintsByType(Constraint::STRETCH, constraints);
  if (constraints.size()) {
    std::cout << "we have some bend constraints lets draw them !!" << std::endl;
    pxr::SdfPath bendPath(rootId.AppendChild(pxr::TfToken("Bend")));
    _sourcesMap[bendPath] = sources;
    Curve* curve = AddCurve(bendPath);
    pxr::VtArray<pxr::GfVec3f> positions;
    pxr::VtArray<float> radii;
    pxr::VtArray<int> cvCounts;
    for (const auto& constraint : constraints) {
      pxr::VtArray<pxr::GfVec3f> points;
      constraint->GetPoints(_solver->GetParticles(), points);
      for (auto& point : points) {
        positions.push_back(point);
        radii.push_back(0.02f);
      }
      for (size_t e = 0; e < points.size() / 2; ++e) {
        cvCounts.push_back(2);
      }
    }
    curve->SetTopology(positions, radii, cvCounts);
  }

  /*
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

  _InitControls();

  size_t numStrands = 0;
  pxr::UsdPrimRange primRange = stage->TraverseAll();
  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken curveName(prim.GetName().GetString() + "RT");
      pxr::SdfPath curvePath(rootId.AppendChild(curveName));
      _sourcesMap[curvePath] = { prim.GetPath(), pxr::HdChangeTracker::Clean };
      Curve* curve = AddCurve(curvePath);
      pxr::UsdGeomMesh usdMesh(prim);

      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      pxr::HdDirtyBits bits = _HairEmit(curve, usdMesh, xform, 0);
    }
  }
  */
}


void 
Scene::UpdateExec(double time)
{
  if (pxr::GfIsClose(time, GetApplication()->GetTime().GetStartTime(), 0.01))
    _solver->Reset();
  else
    _solver->Step(false);
  
  pxr::UsdStageRefPtr stage = GetApplication()->GetStage();
  pxr::UsdGeomXformCache xformCache(time);
  
  for (auto& execPrim : _prims) {
    /*
    pxr::UsdPrim usdPrim = stage->GetPrimAtPath(_sourcesMap[execPrim.first].first);
    pxr::UsdGeomMesh usdMesh(usdPrim);

    pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(usdPrim);
    _HairEmit((Curve*)execPrim.second.geom, usdMesh, xform, time);
    */
    if (execPrim.first.GetNameToken() == pxr::TfToken("Display")) {
      Points* points = (Points*)GetGeometry(execPrim.first);
      points->SetPositions(
        &_solver->GetParticles()->position[0], 
        _solver->GetNumParticles()
      );
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Bend")) {
      /*
      pxr::VtArray<Constraint*> constraints;
      _solver->GetConstraintsByType(Constraint::BEND, constraints);
      Curve* curve = (Curve*)GetGeometry(execPrim.first);
      pxr::VtArray<pxr::GfVec3f> positions;
      pxr::VtArray<float> radii;
      pxr::VtArray<int> cvCounts;
      for (const auto& constraint : constraints) {
        pxr::VtArray<pxr::GfVec3f> points;
        constraint->GetPoints(_solver->GetParticles(), points);
        for (auto& point : points) {
          positions.push_back(point);
          radii.push_back(0.02f);
        }
        for (size_t e = 0; e < points.size() / 2; ++e) {
          cvCounts.push_back(2);
        }
      }
      curve->SetTopology(positions, radii, cvCounts);
      */
    }

    execPrim.second.bits = /*pxr::HdChangeTracker::DirtyTopology;*/
      pxr::HdChangeTracker::Clean |
      pxr::HdChangeTracker::DirtyPoints |
      pxr::HdChangeTracker::DirtyWidths |
      pxr::HdChangeTracker::DirtyPrimvar;
  }
}

void 
Scene::TerminateExec()
{
  Application* app = GetApplication();
  pxr::UsdStageWeakPtr stage = app->GetStage();
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      Remove(meshPath);
    }
  }
  delete _solver;
}

JVR_NAMESPACE_CLOSE_SCOPE