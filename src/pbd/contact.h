#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../common.h"
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE

class Contact : public Location
{
public:
    // Constructors
  Contact() : Location() {};
  
  Contact(const Contact& other)
    : Location(other)
    , _position(other._position)
    , _normal(other._normal)
    , _response(other._response) {};

  Contact(int geomId, int elemId, const pxr::GfVec4f& coords)
    : Location(geomId, elemId, coords) {};

  Contact(int geomId, int elemId, const pxr::GfVec4f& coords,
    const pxr::GfVec3f& position, const pxr::GfVec3f& normal, 
    const pxr::GfVec3f& response)
    : Location(geomId, elemId, coords)
    , _position(position)
    , _normal(normal)
    , _response(response) {};

  bool GetHit() const;
  pxr::GfVec3f GetPosition() const;
  pxr::GfVec3f GetNormal() const;
  pxr::GfVec3f GetResponse() const;

  void Set(const Contact& other);
  void SetHit(bool hit) { _hit = hit; };
  void SetPosition(const pxr::GfVec3f& position) { _position = position; };
  void SetNormal(const pxr::GfVec3f& normal) { _normal = normal; };
  void SetResponse(const pxr::GfVec3f& response) { _response = response; };

protected:
  bool          _hit;
  pxr::GfVec3f  _position;
  pxr::GfVec3f  _normal;
  pxr::GfVec3f  _response;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_ACCUM_H
