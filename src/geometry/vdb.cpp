
#include "vdb.h"
#include <openvdb/tools/RayIntersector.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/GridTransformer.h>

#include <pxr/base/gf/matrix4f.h>

using namespace openvdb;
using namespace openvdb::tools;

JVR_NAMESPACE_OPEN_SCOPE

struct MeshWrapper {
  Mesh* mesh;
  size_t polygonCount() const {
    return mesh->GetNumFaces();
  }
  size_t pointCount() {
    return mesh->GetNumPoints();
  }
  size_t vertexCount(size_t n) const { // Vertex count for polygon n
    return mesh->GetFaceNumVertices(n);
  }

  // Return position pos in local grid index space for polygon n and vertex v
  void getIndexSpacePoint(size_t n, size_t v, openvdb::Vec3d& pos) const {
    GfVec3f pt = mesh->GetPosition(mesh->GetFaceVertexIndex(n, v));
    pos[0] = pt[0];
    pos[1] = pt[1];
    pos[2] = pt[2];
  }

  MeshWrapper(Mesh* m) : mesh(m) {}
};

void VDB::LoadMesh(Mesh& mesh, float resolution, int band) {
  MeshWrapper mesher(&mesh);
  for (auto & p : mesher.mesh->GetPositions()) {
    p /= resolution;
  }
  openvdb::math::Transform xfo;
  xfo.preScale(resolution);

  _grid = meshToVolume<openvdb::FloatGrid>(mesher, xfo, band, band);

  for (auto & p : mesher.mesh->GetPositions()) {
    p *= resolution;
  }

  _initialized = true;
}

void VDB::Offset(float amt) {
  openvdb::tools::LevelSetFilter<FloatGrid> filter(*_grid);
  filter.offset(amt);

  _initialized = true;
}

void VDB::Load(std::string filename) {
}

void VDB::DoUnion(VDB& vdb) {
  const openvdb::math::Transform
    &sourceXform = vdb._grid->transform(),
    &targetXform = _grid->transform();
  openvdb::FloatGrid::Ptr cGrid = 
    createLevelSet<openvdb::FloatGrid>(_grid->voxelSize()[0]);
  cGrid->transform() = _grid->transform();
  // Compute a source grid to target grid transform.
  openvdb::Mat4R xform =
    sourceXform.baseMap()->getAffineMap()->getMat4() *
    targetXform.baseMap()->getAffineMap()->getMat4().inverse();
  // Create the transformer.
  openvdb::tools::GridTransformer transformer(xform);

  // Resample using trilinear interpolation.
  transformer.transformGrid<openvdb::tools::BoxSampler, openvdb::FloatGrid>(
    *vdb._grid, *cGrid);

  openvdb::tools::csgUnion(*_grid, *cGrid);
  _initialized = true;
}

void VDB::DoDifference(VDB & vdb) {
  const openvdb::math::Transform
    &sourceXform = vdb._grid->transform(),
    &targetXform = _grid->transform();
  openvdb::FloatGrid::Ptr cGrid = 
    createLevelSet<openvdb::FloatGrid>(_grid->voxelSize()[0]);
  cGrid->transform() = _grid->transform();
  // Compute a source grid to target grid transform.
  openvdb::Mat4R xform =
    sourceXform.baseMap()->getAffineMap()->getMat4() *
    targetXform.baseMap()->getAffineMap()->getMat4().inverse();
  // Create the transformer.
  openvdb::tools::GridTransformer transformer(xform);

  // Resample using trilinear interpolation.
  transformer.transformGrid<openvdb::tools::BoxSampler, openvdb::FloatGrid>(
    *vdb._grid, *cGrid);
  openvdb::tools::csgDifference(*_grid, *cGrid);
  _initialized = true;
}

void VDB::DoUnion(FloatGrid::Ptr vdb) {
  openvdb::tools::csgUnion(*_grid, *(vdb));
  _initialized = true;
}

void VDB::DoIntersect(VDB & vdb) {
  //copy
  const math::Transform
    &sourceXform = vdb._grid->transform(),
    &targetXform = _grid->transform();
  FloatGrid::Ptr cGrid = 
    createLevelSet<FloatGrid>(_grid->voxelSize()[0]);
  cGrid->transform() = _grid->transform();
  // Compute a source grid to target grid transform.
  openvdb::Mat4R xform =
    sourceXform.baseMap()->getAffineMap()->getMat4() *
    targetXform.baseMap()->getAffineMap()->getMat4().inverse();
  // Create the transformer.
  openvdb::tools::GridTransformer transformer(xform);

  // Resample using trilinear interpolation.
  transformer.transformGrid<openvdb::tools::BoxSampler, openvdb::FloatGrid>(
    *vdb._grid, *cGrid);
  openvdb::tools::csgIntersection(*_grid, *cGrid);
  _initialized = false;
}

void VDB::Blur() {
  openvdb::tools::LevelSetFilter<FloatGrid> filter(*_grid);
  //filter.gaussian();
  filter.laplacian();
  _initialized = true;
}

void VDB::Taubin() {
  openvdb::tools::LevelSetFilter<FloatGrid> filter(*_grid);
  filter.gaussian();
  //filter.taubin();
  _initialized = true;
}

void VDB::Clear() {
  _grid->clear();
}

void VDB::ToMesh(Mesh* mesh) {
   //openvdb::tools::VolumeToMesh mesher(grid->getGridClass() == openvdb::GRID_LEVEL_SET ? 0.0 : 0.01);
  openvdb::tools::VolumeToMesh mesher(_isovalue);
  mesher(*_grid);

/*
  mesh.clear();

  openvdb::Coord ijk;

  for (Index64 n = 0, i = 0, N = mesher.pointListSize(); n < N; ++n) {
    const openvdb::Vec3s& p = mesher.pointList()[n];
    mesh.addVertex(ofVec3f(p[0], p[1], p[2]));
    mesh.addNormal(ofVec3f(0,0,0));
  }

  // Copy primitives
  openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();
  Index64 numQuads = 0;
  for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
    numQuads += polygonPoolList[n].numQuads();
  }

  ofVec3f norm, e1, e2;

  for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
    const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
    for (Index64 i = 0, I = polygons.numQuads(); i < I; ++i) {
      const openvdb::Vec4I& quad = polygons.quad(i);
      mesh.addIndex(quad[0]);
      mesh.addIndex(quad[2]);
      mesh.addIndex(quad[1]);
      mesh.addIndex(quad[0]);
      mesh.addIndex(quad[3]);
      mesh.addIndex(quad[2]);

      e1 = mesh.getVertex(quad[2]);
      e1 -= mesh.getVertex(quad[0]);
      e2 = mesh.getVertex(quad[3]);
      e2 -= mesh.getVertex(quad[1]);
      norm = e1.cross(e2);

      //const float length = norm.length();
      //if (length > 1.0e-6) norm /= length;
      //norm *= -1;
      for (int v = 0; v < 4; ++v) {
        mesh.setNormal(quad[v], mesh.getNormal(quad[v]) + norm);
        //mesh.setNormal(quad[v], norm);
      }
    }
  }
  */
  _xfo.SetIdentity();
  _initialized = false;
}

void VDB::FloodFill() {
  openvdb::tools::signedFloodFill(_grid->tree());
}


void VDB::Transform(const GfMatrix4f & mat) {
  /*
  _xfo *= mat;
  math::Mat4d vMat(mat.GetPtr());
  grid->transform().postMult(vMat);
  */
}

void VDB::Translate(const GfVec3f& dir) {
  /*
  tempTransform.translate(dir);
  grid->transform().postTranslate(Vec3d(dir.x, dir.y, dir.z));
  */
}

void VDB::Rotate(const GfVec3f& axis, float angle) {
  /*
  Mat4d mat;
  mat.setToRotation(Vec3R(axis.x, axis.y, axis.z), angle);
  tempTransform.rotateRad(angle, axis.x, axis.y, axis.z);
  grid->transform().postMult(mat);
  */
}

void VDB::Save(std::string filename) {

}

VDB::VDB() {
  _grid = openvdb::FloatGrid::create(1.2);
  _grid->setGridClass(openvdb::GRID_LEVEL_SET);
  //mesh.enableNormals();
  _isovalue = 0.0;
  _initialized = false;
}

VDB::VDB(Mesh& m, float resolution) {
  _grid = openvdb::FloatGrid::create(resolution*3);
  //mesh.enableNormals();
  _initialized = false;
  _isovalue = 0.0;
  LoadMesh(m, resolution);
}

VDB::VDB(const VDB & vdb) {
  _grid = vdb._grid->deepCopy();
  //mesh.enableNormals();
  _isovalue = vdb._isovalue;
  _initialized = false;
}

GfRange3f VDB::GetBBox() {
  
  openvdb::math::CoordBBox bbox = _grid->evalActiveVoxelBoundingBox();
  openvdb::Coord minC = bbox.getStart();
  openvdb::Coord maxC = bbox.getEnd();
  openvdb::Vec3d minPt = _grid->indexToWorld(minC);
  openvdb::Vec3d maxPt = _grid->indexToWorld(maxC);
  return GfRange3f(
    GfVec3f(minPt.x(), minPt.y(), minPt.z()), 
    GfVec3f(maxPt.x(), maxPt.y(), maxPt.z())
  );
}

bool VDB::IntersectRay(const float x, const float y, const float z, 
  const float dx, const float dy, const float dz, 
  float & ox, float &oy, float &oz) 
{
  openvdb::tools::LevelSetRayIntersector<FloatGrid> 
    intersector(*_grid, _isovalue);
  Vec3R pt;

  bool val = 
    intersector.intersectsWS(
      math::Ray<Real>(openvdb::Vec3R(x, y, z), 
      openvdb::Vec3R(dx, dy, dz)), pt
    );
  ox = pt[0];
  oy = pt[1];
  oz = pt[2];
  return val;
}

bool VDB::IntersectRay(const GfVec3f& pt, const GfVec3f& dir, 
  GfVec3f& out) 
{
	return IntersectRay(
    pt[0], pt[1], pt[2], 
    dir[0], dir[1], dir[2], 
    out[0], out[1], out[2]
  );
}

bool VDB::IntersectRay(const float x, const float y, const float z, 
  const float dx, const float dy, const float dz, 
  float& ox, float& oy, float& oz, float& t) 
{
  //if (grid->getGridClass() == GRID_LEVEL_SET) {
    openvdb::tools::LevelSetRayIntersector<FloatGrid> 
      intersector(*_grid, _isovalue);
    openvdb::Vec3R pt;
    openvdb::Real t0;
    bool val = intersector.intersectsWS(
      openvdb::math::Ray<Real>(openvdb::Vec3R(x, y, z), 
      openvdb::Vec3R(dx, dy, dz)), pt, t0
    );
    t = t0;
    ox = pt[0];
    oy = pt[1];
    oz = pt[2];
    return val;
    /*
    }

  else {
    VolumeRayIntersector<FloatGrid> intersector(*grid, isovalue);
    Vec3R pt;
    intersector.setWorldRay(math::Ray<Real>(Vec3R(x, y, z), Vec3R(dx, dy, dz)));
    Real t0 = 0, t1 = 0;
    while (intersector.march(t0,t1)) {
      t = t0;
      pt = intersector.getWorldPos(t0);
      ox = pt[0];
      oy = pt[1];
      oz = pt[2];
      return true;
    }
    return false;
  }*/
}

bool VDB::IntersectRay(const GfVec3f& pt, const GfVec3f& dir, 
  GfVec3f& out, float&t) {
  return IntersectRay(
    pt[0], pt[1], pt[2], 
    dir[0], dir[1], dir[2], 
    out[0], out[1], out[2], t
  );
}

struct MatAdd 
{
  float f;
  MatAdd(float _f) : f(_f) {}
  inline void operator()(const openvdb::FloatGrid::ValueOnIter& iter) const {
    iter.setValue(*iter + f);
  }
};


void VDB::SetThreshold(float threshold) 
{
  //foreach(grid->beginValueOn(), MatAdd(thresh-isovalue));
  _isovalue = threshold;
  _initialized = false;
}


void VDB::LoadVolume(std::ifstream & buf, int w, int h, int d, float resolution) {
  _grid = openvdb::FloatGrid::create(100);
  openvdb::FloatGrid::Accessor acc = _grid->getAccessor();
  openvdb::Coord ijk;
  _grid->setGridClass(GRID_LEVEL_SET);

  float threshold = .05;
  float f;
  float minV = 9e9;
  float maxV = -9e9;
  int &x = ijk[0], &y = ijk[1], &z = ijk[2];
  for (x = 0; x < w; ++x) {
    for (y = 0; y < h; ++y) {
      for (z = 0; z < d; ++z) {
        buf.read((char *)&f, sizeof(f));
        if (f < threshold) {
          f = 0;
        }
        else {
          f = -f;
        }
        minV = GfMin(minV, f);
        maxV = GfMax(maxV, f);
        acc.setValue(ijk, f);
      }
    }
  }
  std::cout << "Loaded volume. min value: " << minV << 
    " max value: " << maxV << std::endl;
  //grid-> = max(abs(maxV), abs(minV));
  _grid->pruneGrid();
  SetThreshold(-.5);
  math::Transform trans;
  trans.preScale(resolution);
  _grid->transform() = trans;
  _initialized = false;
}

void VDB::LevelSphere(const GfVec3f& center, float radius,
    float voxelSize, float width)
{
  // Create a FloatGrid and populate it with a narrow-band
  // signed distance field of a sphere.
  _grid =
    openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
      radius, openvdb::Vec3f(center[0], center[1], center[2]),
      voxelSize, width
    );
  // Associate some metadata with the grid.
  _grid->insertMeta("radius", openvdb::FloatMetadata(radius));
  // Name the grid "LevelSetSphere".
  _grid->setName("LevelSetSphere");
}

/*
void
makeVDBSphere()
{
  openvdb::initialize();
  
  openvdb::FloatGrid::Ptr grid =
        openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
            50.0, openvdb::Vec3f(1.5, 2, 3),
            0.5, 4.0);

  // Create an empty floating-point grid with background value 0.
  openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();

  // Populate the grid with a sparse, narrow-band level set representation
  // of a sphere with radius 50 voxels, located at (1.5, 2, 3) in index space.
  makeSphere(*grid, 50.0, openvdb::Vec3f(1.5, 2, 3));
  // Associate some metadata with the grid.
  grid->insertMeta("radius", openvdb::FloatMetadata(50.0));
  // Associate a scaling transform with the grid that sets the voxel size
  // to 0.5 units in world space.
  grid->setTransform(
      openvdb::math::Transform::createLinearTransform(0.5));
  // Identify the grid as a level set.
  grid->setGridClass(openvdb::GRID_LEVEL_SET);
  // Name the grid "LevelSetSphere".
  grid->setName("LevelSetSphere");
  
  // Propagate the outside/inside sign information from the narrow band
  // throughout the grid.
  //openvdb::tools::signedFloodFill(grid->tree());

  openvdb::tools::VolumeToMesh mesher;
  mesher(*grid);
  size_t numVertices = mesher.pointListSize();
  std::cout << "OPEN VDB NUM VERTICES " << numVertices << std::endl;
  size_t numPolygonPools = mesher.polygonPoolListSize();
  std::cout << "OPEN VDB NUM POLYGON POOLS " << numPolygonPools << std::endl;

}
*/
JVR_NAMESPACE_CLOSE_SCOPE