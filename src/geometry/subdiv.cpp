#include "../geometry/subdiv.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE

using namespace OpenSubdiv;

void _SubdivideMesh(Mesh* mesh, int refineLevel)
{
  // Populate a topology descriptor with our raw data
  Sdc::SchemeType type = Sdc::SCHEME_CATMARK;

  Sdc::Options options;
  options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

  Far::TopologyDescriptor desc;
  desc.numVertices = mesh->GetNumPoints();
  desc.numFaces = mesh->GetNumFaces();
  desc.numVertsPerFace = &mesh->GetFaceCounts()[0];
  desc.vertIndicesPerFace = &mesh->GetFaceConnects()[0];


  // Instantiate a Far::TopologyRefiner from the descriptor
  Far::TopologyRefiner* refiner =
    Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(desc,
      Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(type, options));

  // Uniformly refine the topology up to 'refineLevel'
  refiner->RefineUniform(Far::TopologyRefiner::UniformOptions(refineLevel));


  // Allocate a buffer for vertex primvar data. The buffer length is set to
  // be the sum of all children vertices up to the highest level of refinement.
  std::vector<_Vertex> vBuffer(refiner->GetNumVerticesTotal());
  _Vertex* verts = &vBuffer[0];


  // Initialize coarse mesh positions
  int nCoarseVerts = mesh->GetNumPoints();
  const pxr::GfVec3f* points = mesh->GetPositionsCPtr();
  for (int i = 0; i < nCoarseVerts; ++i) {
    verts[i].SetPosition(points[i][0], points[i][1], points[i][2]);
  }

  // Interpolate vertex primvar data
  Far::PrimvarRefiner primvarRefiner(*refiner);

  _Vertex* src = verts;
  size_t firstVert = 0;
  
  for (int level = 1; level <= refineLevel; ++level) {
    /*
    float variance = (refineLevel - level) / 10.f;
    Far::TopologyLevel const& refLevel = refiner->GetLevel(level);
    size_t numVerts = refLevel.GetNumVertices();
    for (int vert = 0; vert < numVerts; ++vert) {
      float const* pos = verts[firstVert + vert].GetPosition();

      verts[firstVert + vert].SetPosition(
        pos[0] + RANDOM_LO_HI(-variance, variance), 
        pos[1] + RANDOM_LO_HI(-variance, variance), 
        pos[2] + RANDOM_LO_HI(-variance, variance)
      );
    }
    firstVert += numVerts;
    if(level > 0) {
    */
      _Vertex* dst = src + refiner->GetLevel(level - 1).GetNumVertices();
      primvarRefiner.Interpolate(level, src, dst);
      src = dst;
    //}
  }

  { // Output of the highest level refined -----------
    Far::TopologyLevel const & refLastLevel = refiner->GetLevel(refineLevel);

    int numVerts = refLastLevel.GetNumVertices();
    int numFaces = refLastLevel.GetNumFaces();

    pxr::VtArray<pxr::GfVec3f> positions(numVerts);
    pxr::VtArray<int> faceCounts(numFaces);
    pxr::VtArray<int> faceConnects;

    // vertex positions
    int firstOfLastVerts = refiner->GetNumVerticesTotal() - numVerts;

    for (int vert = 0; vert < numVerts; ++vert) {
        float const * pos = verts[firstOfLastVerts + vert].GetPosition();
        positions[vert][0] = pos[0];
        positions[vert][1] = pos[1];
        positions[vert][2] = pos[2];
    }

    // faces
    for (int face = 0; face < numFaces; ++face) {

        Far::ConstIndexArray faceVerts = refLastLevel.GetFaceVertices(face);

        // all refined Catmark faces should be quads
        assert(faceVerts.size()==4);

        faceCounts[face] = faceVerts.size();
        for (int vert=0; vert<faceVerts.size(); ++vert) {
          faceConnects.push_back(faceVerts[vert]);
        }
    }
    std::cout << "set mesh : " << mesh << std::endl;
    mesh->Set(positions, faceCounts, faceConnects);
    std::cout << "mesh set !" << std::endl;
  }

  delete refiner;
}

JVR_NAMESPACE_CLOSE_SCOPE
