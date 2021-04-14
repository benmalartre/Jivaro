
#include "vdb.h"

void
makeVDBSphere()
{
  openvdb::initialize();
  /*
  openvdb::FloatGrid::Ptr grid =
        openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
            50.0, openvdb::Vec3f(1.5, 2, 3),
            0.5, 4.0);
  */

  // Create a FloatGrid and populate it with a narrow-band
  // signed distance field of a sphere.
  openvdb::FloatGrid::Ptr grid =
      openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
          /*radius=*/50.0, /*center=*/openvdb::Vec3f(1.5, 2, 3),
          /*voxel size=*/0.5, /*width=*/4.0);
  // Associate some metadata with the grid.
  grid->insertMeta("radius", openvdb::FloatMetadata(50.0));
  // Name the grid "LevelSetSphere".
  grid->setName("LevelSetSphere");
  
  /*
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
  */
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