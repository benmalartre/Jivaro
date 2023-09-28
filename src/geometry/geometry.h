#ifndef JVR_GEOMETRY_GEOMETRY_H
#define JVR_GEOMETRY_GEOMETRY_H

#include <float.h>

#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

enum GeomInterpolation : short {
  GeomInterpolationConstant = 0,
  GeomInterpolationUniform,
  GeomInterpolationVarying,
  GeomInterpolationVertex,
  GeomInterpolationFaceVarying,

  GeomInterpolationCount
};

class Hit;

class Geometry {
public:
  enum Type {
    INVALID,
    SPHERE,
    CUBE,
    CONE, 
    CAPSULE,
    POINT,
    CURVE,
    MESH,
    STROKE,
    INSTANCER,
    VOXEL
  };

  Geometry();
  Geometry(short type, const pxr::GfMatrix4d& world);
  Geometry(const Geometry* other, short type, bool normalize);
  virtual ~Geometry() {};

  short GetType() const { return _type; };
  virtual size_t GetNumPoints() const {return 1;};

  void SetWirecolor(const pxr::GfVec3f& wirecolor){_wirecolor=wirecolor;};
  const pxr::GfVec3f& GetWirecolor() { return _wirecolor; };

  void SetMatrix(const pxr::GfMatrix4d& matrix);
  const pxr::GfMatrix4d& GetMatrix() const { return _matrix; };
  const pxr::GfMatrix4d& GetInverseMatrix() const { return _invMatrix; };

  virtual void ComputeBoundingBox() = 0;
  pxr::GfBBox3d& GetBoundingBox() { return _bbox; };
  const pxr::GfBBox3d& GetBoundingBox() const { return _bbox; };

  bool IsInitialized(){return _initialized;};
  void SetInitialized(bool initialized){_initialized = initialized;};

  // query 3d position on geometry
  virtual bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance=-1.0, double* minDistance=NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const = 0;

protected:
  // infos
  short                               _type;

  // bounding box
  pxr::GfMatrix4d                     _matrix;
  pxr::GfMatrix4d                     _invMatrix;
  pxr::GfBBox3d                       _bbox;
  bool                                _initialized;
  pxr::GfVec3f                        _wirecolor;
};

class Location {
public:
  virtual void GetPosition(const Geometry* geom, pxr::GfVec3f* pos,
    bool worldSpace=true) const = 0;
  virtual void GetNormal(const Geometry* geom, pxr::GfVec3f* nrm,
    bool worldSpace=true) const = 0;
  friend class Geometry;
};

class SphereLocation : public Location {
public:
  SphereLocation(float longitude, float latitude)
    : _longitude(longitude), _latitude(latitude) {};

  void GetPosition(const Geometry* geom, pxr::GfVec3f* pos,
    bool worldSpace=true) const override;
  void GetNormal(const Geometry* geom, pxr::GfVec3f* nrm,
    bool worldSpace=true) const override;

private:
  float _longitude;
  float _latitude;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_GEOMETRY_H
