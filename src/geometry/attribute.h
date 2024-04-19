//======================================================
// ATTRIBUTE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_ATTRIBUTE_H
#define JVR_GEOMETRY_ATTRIBUTE_H


#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

template<typename T>
struct Attribute {
  pxr::TfToken    name;
  short           dataType;
  short           dataCtxt;
  short           dataStrct;
  pxr::VtArray<T> data;
  uint32_t id;

  Attribute() : id(INVALID_POINT_ID) {};
  Attribute(uint32_t index) : id(index) {};
  uint32_t GetIndex(){return id;};

  virtual bool Touch(const pxr::GfVec3f* points, 
    const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize) const = 0;
  virtual bool Raycast(const pxr::GfVec3f* points, const pxr::GfRay& ray, Location* hit) const = 0;
  virtual bool Closest(const pxr::GfVec3f* points, const pxr::GfVec3f& point, Location* hit) const = 0;

  virtual pxr::GfRange3f GetLocalBoundingBox(const Geometry* geometry) const = 0;
  virtual pxr::GfRange3f GetWorldBoundingBox(const Geometry* geometry) const = 0;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_ATTRIBUTE_H