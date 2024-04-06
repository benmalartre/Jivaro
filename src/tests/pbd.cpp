#include <pxr/base/vt/array.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformOp.h>
#include <pxr/imaging/hd/changeTracker.h>

#include "../geometry/sampler.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/triangle.h"
#include "../geometry/utils.h"
#include "../pbd/particle.h"
#include "../pbd/force.h"
#include "../pbd/constraint.h"
#include "../pbd/collision.h"
#include "../pbd/solver.h"
#include "../utils/timer.h"

#include "../app/scene.h"

#include "../tests/pbd.h"

JVR_NAMESPACE_OPEN_SCOPE

static void _InitControls(pxr::UsdStageRefPtr& stage)
{
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


pxr::UsdPrim _GenerateGroundBox(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path)
{

  pxr::UsdGeomCube ground =
    pxr::UsdGeomCube::Define(stage, path.AppendChild(pxr::TfToken("Ground")));

  ground.CreateSizeAttr().Set(1.0);

  ground.AddScaleOp(pxr::UsdGeomXformOp::PrecisionFloat).Set(pxr::GfVec3f(100.f, 1.f, 100.f));
  ground.AddTranslateOp(pxr::UsdGeomXformOp::PrecisionFloat).Set(pxr::GfVec3f(0.f, -0.5f, 0.f));


  pxr::UsdPrim usdPrim = ground.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);

  return ground.GetPrim();
}

pxr::UsdPrim _GenerateSolver(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, const pxr::TfToken& name)
{

  pxr::UsdGeomXform usdXform = pxr::UsdGeomXform::Define(stage, path.AppendChild(name));


  pxr::UsdPrim usdPrim = usdXform.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("SubSteps"), pxr::SdfValueTypeNames->Int).Set(20);
  usdPrim.CreateAttribute(pxr::TfToken("SleepThreshold"), pxr::SdfValueTypeNames->Float).Set(0.01f);
  usdPrim.CreateAttribute(pxr::TfToken("Gravity"), pxr::SdfValueTypeNames->Vector3f).Set(pxr::GfVec3f(0.f, -9.8f, 0.f));

  return usdPrim;
}

pxr::UsdGeomMesh _GenerateClothMesh(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  const pxr::TfToken& name, float size, const pxr::GfMatrix4f& m)
{
  Mesh mesh;
  mesh.TriangularGrid2D(10.f, 10.f, m, size);
  //mesh.Randomize(0.1f);
  pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define(stage, path.AppendChild(name));

  usdMesh.CreatePointsAttr().Set(mesh.GetPositions());
  usdMesh.CreateFaceVertexCountsAttr().Set(mesh.GetFaceCounts());
  usdMesh.CreateFaceVertexIndicesAttr().Set(mesh.GetFaceConnects());

  pxr::UsdPrim usdPrim = usdMesh.GetPrim();
  usdPrim.CreateAttribute(pxr::TfToken("StretchStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("BendStiffness"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Restitution"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Friction"), pxr::SdfValueTypeNames->Float).Set(RANDOM_0_1);
  usdPrim.CreateAttribute(pxr::TfToken("Serial"), pxr::SdfValueTypeNames->Bool).Set(false);

  return usdMesh;
}

pxr::UsdGeomSphere _GenerateCollideSphere(pxr::UsdStageRefPtr& stage, const pxr::SdfPath& path, 
  double radius, const pxr::GfMatrix4f& m)
{
  pxr::UsdGeomSphere usdSphere = pxr::UsdGeomSphere::Define(stage, path);

  usdSphere.CreateRadiusAttr().Set(radius);

  double real;
  usdSphere.GetRadiusAttr().Get(&real);

  pxr::UsdGeomXformOp op = usdSphere.MakeMatrixXform();
  op.Set(pxr::GfMatrix4d(m));

  return usdSphere;
}


void _GenerateSample(pxr::UsdGeomMesh& mesh, pxr::VtArray<Sample>* samples, float minRadius = 0.1, size_t density = 1024)
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

pxr::HdDirtyBits _HairEmit(pxr::UsdStageRefPtr& stage, Curve* curve, pxr::UsdGeomMesh& mesh, pxr::GfMatrix4d& xform, double time)
{
  uint64_t T = CurrentTime();
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
    const float tangentFactor = pxr::GfCos(position[2] * scale + time * frequency) * amplitude;


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

  return pxr::HdChangeTracker::DirtyTopology;
}


void TestPBD::InitExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  _solver = new Solver();
  pxr::UsdPrimRange primRange = stage->TraverseAll();
  pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());
  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("Solver"));

  pxr::GfQuatf rotate(45.f*DEGREES_TO_RADIANS, pxr::GfVec3f(0.f, 0.f, 1.f));
  rotate.Normalize();
  pxr::GfMatrix4f matrix = 
    pxr::GfMatrix4f(1.f).SetScale(pxr::GfVec3f(5.f));
  float size = .25f;

  _GenerateGroundBox(stage, rootId);
  
  
  for(size_t x = 0; x < 6; ++x) {
    std::string name = "cloth" + std::to_string(x);
    _GenerateClothMesh(stage, rootId, pxr::TfToken(name), size,
      matrix * pxr::GfMatrix4f(1.f).SetTranslate(pxr::GfVec3f(x*6.f, 5.f, 0.f)));
  }

  std::vector<pxr::UsdGeomSphere> spheres;
  Scene::_Sources sources;
  float mass = 0.1f;
  for (pxr::UsdPrim prim : primRange) {
    size_t offset = _solver->GetNumParticles();
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::UsdGeomMesh usdMesh(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Mesh* mesh = new Mesh(usdMesh, xform);
      _scene->AddMesh(prim.GetPath(), mesh);
      
      Body* body = _solver->AddBody((Geometry*)mesh, pxr::GfMatrix4f(xform), mass);
      //mass *= 2;
      _solver->AddConstraints(body);

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    } else if (prim.IsA<pxr::UsdGeomPoints>()) {
      pxr::UsdGeomPoints usdPoints(prim);
      pxr::GfMatrix4d xform = xformCache.GetLocalToWorldTransform(prim);
      Points* points = new Points(usdPoints, xform);
      _scene->AddPoints(prim.GetPath(), points);

      _solver->AddBody((Geometry*)points, pxr::GfMatrix4f(xform), RANDOM_LO_HI(0.5f, 5.f));

      sources.push_back({ prim.GetPath(), pxr::HdChangeTracker::Clean });
    }
  }
  _solver->AddForce(new GravitationalForce());
  _solver->WeightBoundaries();
  _solver->LockPoints();
  
  //_solver->AddForce(new DampingForce());
  
  pxr::GfVec3f pos;
  double radius;
  float restitution = 0.25;
  float friction = 0.5f;
  for (auto& sphere: spheres) {
    sphere.GetRadiusAttr().Get(&radius);
    pxr::GfMatrix4f m(sphere.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default()));
    _solver->AddCollision(new SphereCollision(restitution, friction, m, (float)radius));
  } 

  _solver->AddCollision(new PlaneCollision(1.f, 1.f, 
    pxr::GfVec3f(0.f, 1.f, 0.f), pxr::GfVec3f(0.f, -0.1f, 0.f)));

/*
  pxr::SdfPath pointsPath(rootId.AppendChild(pxr::TfToken("Particles")));
  _sourcesMap[pointsPath] = sources;
  Points* points = _scene->AddPoints(pointsPath);
  Particles* particles = _solver->GetParticles();
  const size_t numParticles = _solver->GetNumParticles();
  points->SetPositions(&particles->position[0], numParticles);
  points->SetRadii(&particles->radius[0], numParticles);
  points->SetColors(&particles->color[0], numParticles);

  pxr::SdfPath collisionsPath(rootId.AppendChild(pxr::TfToken("Collisions")));
  _sourcesMap[collisionsPath] = sources;
  Points* collisions = _scene->AddPoints(collisionsPath);
  collisions->SetPositions(&particles->position[0], numParticles);
  collisions->SetRadii(&particles->radius[0], numParticles);
  collisions->SetColors(&particles->color[0], numParticles);
*/
}


void TestPBD::UpdateExec(pxr::UsdStageRefPtr& stage, double time, double startTime)
{
  if (pxr::GfIsClose(time, startTime, 0.01))
    _solver->Reset();
  else
    _solver->Step(false);
  
  pxr::UsdGeomXformCache xformCache(time);

  const size_t numParticles = _solver->GetNumParticles();
  const pxr::GfVec3f hitColor(1.f, 0.2f, 0.3f);
  Geometry* geom;
  
  for(Body* body: _solver->GetBodies()) {
    std::cout << "damping : " << body->damping << std::endl;
    std::cout << "radius : " << body->radius << std::endl;
    std::cout << "mass : " << body->mass << std::endl;
  }
 
  for (auto& execPrim : _scene->GetPrims()) {
    pxr::UsdPrim prim = stage->GetPrimAtPath(execPrim.first);
    if(prim.IsA<pxr::UsdGeomMesh>()) {

    }
    /*
    if (execPrim.first.GetNameToken() == pxr::TfToken("Particles")) {
      Points* points = (Points*)_scene->GetGeometry(execPrim.first);

      points->SetPositions(&_solver->GetParticles()->position[0], numParticles);
      points->SetColors(&_solver->GetParticles()->color[0], numParticles);
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Collisions")) {
      const pxr::VtArray<Constraint*>& contacts = _solver->GetContacts();
      if (!contacts.size())continue;

      Points* points = (Points*)_scene->GetGeometry(execPrim.first);

      pxr::VtArray<pxr::GfVec3f>& positions = points->GetPositions();
      pxr::VtArray<pxr::GfVec3f>& colors = points->GetColors();
      pxr::VtArray<float>& radii = points->GetRadius();

      memset(&radii[0], 0.f, numParticles * sizeof(float));

      for (auto& contact : contacts) {
        const size_t offsetIdx = contact->GetBody(0)->offset;
        
        const pxr::VtArray<int>& elements = contact->GetElements();
        for(int elem: elements) {
          positions[elem + offsetIdx] = _solver->GetParticles()->position[elem + offsetIdx];
          radii[elem + offsetIdx] = 0.2f;
          colors[elem + offsetIdx] = hitColor;
        }
      }
    } else if (execPrim.first.GetNameToken() == pxr::TfToken("Constraints")) {
      
    }
    */

    execPrim.second.bits =
      pxr::HdChangeTracker::Clean |
      pxr::HdChangeTracker::DirtyPoints |
      pxr::HdChangeTracker::DirtyWidths |
      pxr::HdChangeTracker::DirtyPrimvar;
  }
}

void TestPBD::TerminateExec(pxr::UsdStageRefPtr& stage)
{
  if (!stage) return;

  pxr::UsdPrim rootPrim = stage->GetDefaultPrim();
  pxr::SdfPath rootId = rootPrim.GetPath().AppendChild(pxr::TfToken("test"));

  pxr::UsdPrimRange primRange = stage->TraverseAll();

  for (pxr::UsdPrim prim : primRange) {
    if (prim.IsA<pxr::UsdGeomMesh>()) {
      pxr::TfToken meshName(prim.GetName().GetString() + "RT");
      pxr::SdfPath meshPath(rootId.AppendChild(meshName));
      _scene->Remove(meshPath);
    }
  }
  delete _solver;

}

JVR_NAMESPACE_CLOSE_SCOPE