#include <pxr/base/gf/vec3f.h>
#include "../acceleration/intersector.h"
#include "../geometry/component.h"
#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"


JVR_NAMESPACE_OPEN_SCOPE

bool
Intersector::Element::Touch(const pxr::GfVec3f* points,
  const pxr::GfVec3f& center, const pxr::GfVec3f& halfSize)
{
  Component* component = (Component*)_ptr;
  return component->Touch(points, center, halfSize);

}


bool
Intersector::Element::Closest(const pxr::GfVec3f* points,
  const pxr::GfVec3f& query, Hit* hit, double maxDistance)
{
  Component* component = (Component*)_ptr;
  return component->Closest(points, query, hit, maxDistance);
}

bool
Intersector::Element::Raycast(const pxr::GfVec3f* points,
  const pxr::GfRay& ray, Hit* hit, double maxDistance, double* minDistance)
{
  Component* component = (Component*)_ptr;
  return component->Raycast(points, ray, hit, maxDistance, minDistance);
}



JVR_NAMESPACE_CLOSE_SCOPE