#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../pbd/element.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Body : public Element
{

public:
  Body(Geometry* geom, size_t offset, size_t n, const pxr::GfVec3f& color,
    float mass=1.f, float radius=0.01f, float damping=0.1f)
  : Element(Element::BODY)
  , _geometry(geom)
  , _offset(offset)
  , _numPoints(n)
  , _mass(mass)
  , _radius(radius)
  , _damping(damping)
  , _color(color){}

  void SetOffset(size_t offset){_offset = offset;};
  void SetNumPoints(size_t numPoints){_numPoints = numPoints;};

  Geometry* GetGeometry() const {return _geometry;};
  size_t GetOffset() const {return _offset;};
  size_t GetNumPoints() const {return _numPoints;};
  float GetMass() const {return _mass;};
  float GetRadius() const {return _radius;};
  pxr::GfVec3f GetColor() const {return _color;};

protected:
  Geometry*     _geometry;
  size_t        _offset;
  size_t        _numPoints;
  float         _mass;
  float         _radius;
  float         _damping;
  pxr::GfVec3f  _color;
  //bool     _simulated;
};

enum BodyType
{
  RIGID,
  SOFT,
  CLOTH,
  HAIR
};

class Particles : public Element
{
public:
  enum State {MUTE, IDLE, ACTIVE};

  Particles() : Element(Element::PARTICLES){};

  size_t GetNumParticles() { return _position.size(); };
  void AddBody(Body* body, const pxr::GfMatrix4f& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  // setters
  inline void  SetState(size_t index, short state)                       { _state[index] = state;          };
  inline void  SetBody(size_t index, int body)                           { _body[index] = body;            };
  inline void  SetMass(size_t index, float mass)                         { _mass[index] = mass;            };
  inline void  SetInvMass(size_t index, float invMass)                   { _invMass[index] = invMass;      };
  inline void  SetRadius(size_t index, float radius)                     { _radius[index] = radius;        };
  inline void  SetRest(size_t index, const pxr::GfVec3f& rest)           { _rest[index] = rest;            };
  inline void  SetPrevious(size_t index, const pxr::GfVec3f& previous)   { _previous[index] = previous;    };
  inline void  SetPosition(size_t index, const pxr::GfVec3f& position )  { _position[index] = position;    };
  inline void  SetPredicted(size_t index, const pxr::GfVec3f& predicted) { _predicted[index] = predicted;  };
  inline void  SetVelocity(size_t index, const pxr::GfVec3f& velocity)   { _velocity[index] = velocity;    };
  inline void  SetColor(size_t index, const pxr::GfVec3f& color)         { _color[index] = color;          };

  // element getters
  inline const short&         GetState(size_t index)     const { return _state[index];     };
  inline const int&           GetBody(size_t index)      const { return _body[index];      };
  inline const float&         GetMass(size_t index)      const { return _mass[index];      };
  inline const float&         GetInvMass(size_t index)   const { return _invMass[index];   };
  inline const float&         GetRadius(size_t index)    const { return _radius[index];    };
  inline const pxr::GfVec3f&  GetRest(size_t index)      const { return _rest[index];      };
  inline const pxr::GfVec3f&  GetPrevious(size_t index)  const { return _previous[index];  };
  inline const pxr::GfVec3f&  GetPosition(size_t index)  const { return _position[index];  };
  inline const pxr::GfVec3f&  GetPredicted(size_t index) const { return _predicted[index]; };
  inline const pxr::GfVec3f&  GetVelocity(size_t index)  const { return _velocity[index];  };
  inline const pxr::GfVec3f&  GetColor(size_t index)     const { return _color[index];     };
 
  inline short&         GetState(size_t index)     { return _state[index];     };
  inline int&           GetBody(size_t index)      { return _body[index];      };
  inline float&         GetMass(size_t index)      { return _mass[index];      };
  inline float&         GetInvMass(size_t index)   { return _invMass[index];   };
  inline float&         GetRadius(size_t index)    { return _radius[index];    };
  inline pxr::GfVec3f&  GetRest(size_t index)      { return _rest[index];      };
  inline pxr::GfVec3f&  GetPrevious(size_t index)  { return _previous[index];  };
  inline pxr::GfVec3f&  GetPosition(size_t index)  { return _position[index];  };
  inline pxr::GfVec3f&  GetPredicted(size_t index) { return _predicted[index]; };
  inline pxr::GfVec3f&  GetVelocity(size_t index)  { return _velocity[index];  };
  inline pxr::GfVec3f&  GetColor(size_t index)     { return _color[index];     };
 
  // ptr getters
  inline const short*         GetStateCPtr(size_t offset=0)     const { return &_state[offset];     };
  inline const int*           GetBodyCPtr(size_t offset=0)      const { return &_body[offset];      };
  inline const float*         GetMassCPtr(size_t offset=0)      const { return &_mass[offset];      };
  inline const float*         GetInvMassCPtr(size_t offset=0)   const { return &_invMass[offset];   };
  inline const float*         GetRadiusCPtr(size_t offset=0)    const { return &_radius[offset];    };
  inline const pxr::GfVec3f*  GetRestCPtr(size_t offset=0)      const { return &_rest[offset];      };
  inline const pxr::GfVec3f*  GetPreviousCPtr(size_t offset=0)  const { return &_previous[offset];  };
  inline const pxr::GfVec3f*  GetPositionCPtr(size_t offset=0)  const { return &_position[offset];  };
  inline const pxr::GfVec3f*  GetPredictedCPtr(size_t offset=0) const { return &_predicted[offset]; };
  inline const pxr::GfVec3f*  GetVelocityCPtr(size_t offset=0)  const { return &_velocity[offset];  };
  inline const pxr::GfVec3f*  GetColorCPtr(size_t offset=0)     const { return &_color[offset];     };

  inline short*        GetStatePtr(size_t offset=0)     { return &_state[offset];     };
  inline int*          GetBodyPtr(size_t offset=0)      { return &_body[offset];      };
  inline float*        GetMassPtr(size_t offset=0)      { return &_mass[offset];      };
  inline float*        GetInvMassPtr(size_t offset=0)   { return &_invMass[offset];   };
  inline float*        GetRadiusPtr(size_t offset=0)    { return &_radius[offset];    };
  inline pxr::GfVec3f* GetRestPtr(size_t offset=0)      { return &_rest[offset];      };
  inline pxr::GfVec3f* GetPreviousPtr(size_t offset=0)  { return &_previous[offset];  };
  inline pxr::GfVec3f* GetPositionPtr(size_t offset=0)  { return &_position[offset];  };
  inline pxr::GfVec3f* GetPredictedPtr(size_t offset=0) { return &_predicted[offset]; };
  inline pxr::GfVec3f* GetVelocityPtr(size_t offset=0)  { return &_velocity[offset];  };
  inline pxr::GfVec3f* GetColorPtr(size_t offset=0)     { return &_color[offset];     };

private:
  pxr::VtArray<short>        _state;
  pxr::VtArray<int>          _body;
  pxr::VtArray<float>        _mass;
  pxr::VtArray<float>        _invMass;
  pxr::VtArray<float>        _radius;
  pxr::VtArray<pxr::GfVec3f> _rest;
  pxr::VtArray<pxr::GfVec3f> _previous;
  pxr::VtArray<pxr::GfVec3f> _position;
  pxr::VtArray<pxr::GfVec3f> _predicted;
  pxr::VtArray<pxr::GfVec3f> _velocity;
  pxr::VtArray<pxr::GfVec3f> _color;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
