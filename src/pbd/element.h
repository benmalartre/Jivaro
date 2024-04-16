#ifndef JVR_PBD_ELEMENT_H
#define JVR_PBD_ELEMENT_H

#include <pxr/imaging/hd/changeTracker.h>
#include <pxr/usd/usd/prim.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE


class Geometry;
class Element
{
public:
  enum Type { PARTICLES, BODY, COLLISION, FORCE, CONSTRAINT, CONTACT };
  short GetType() {return _type;};

  virtual void UpdateParameters(pxr::UsdPrim& prim, float time) {};
  Element(const  Element& other) = delete;

  Element(short type) : _type(type) {};
  virtual ~Element() {};

private:
  short             _type;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_ELEMENT_H