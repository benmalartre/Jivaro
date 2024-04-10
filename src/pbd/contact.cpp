#include "../pbd/contact.h"

JVR_NAMESPACE_OPEN_SCOPE

bool Contact::GetHit() const
{
  return _hit;
}

pxr::GfVec3f Contact::GetPosition() const
{
  return _position;
}

pxr::GfVec3f Contact::GetNormal() const
{
  return _normal;
}

pxr::GfVec3f Contact::GetResponse() const
{
  return _response;
}

void Contact::Set(const Contact& other) {
    ((Location*)this)->Set(other);
    _position = other._position;
    _normal = other._normal;
    _response = other._response;
  }

JVR_NAMESPACE_CLOSE_SCOPE