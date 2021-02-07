#ifndef AMN_PBD_PARTICLE_H
#define AMN_PBD_PARTICLE_H

#include <vector>
#include "../common.h"
#include <pxr/base/gf/vec3f.h>

AMN_NAMESPACE_OPEN_SCOPE

class PBDParticle
{
  private:
    // Mass
    // If the mass is zero, the particle is static
    std::vector<float> _masses;
    std::vector<float> _invMasses;

    // Dynamic state
    std::vector<pxr::GfVec3f> _x0;
    std::vector<pxr::GfVec3f> _x;
    std::vector<pxr::GfVec3f> _v;
    std::vector<pxr::GfVec3f> _a;
    std::vector<pxr::GfVec3f> _oldX;
    std::vector<pxr::GfVec3f> _lastX;

  public:
    void addVertex(const pxr::GfVec3f& vertex)
    {
      _x0.push_back(vertex);
      _x.push_back(vertex);
      _oldX.push_back(vertex);
      _lastX.push_back(vertex);
      _masses.push_back(1.0);
      _invMasses.push_back(1.0);
      _v.push_back(Vector3r(0.0, 0.0, 0.0));
      _a.push_back(Vector3r(0.0, 0.0, 0.0));
    }

    pxr::GfVec3f& getPosition(const unsigned int i)
    {
      return _x[i];
    }

    const pxr::GfVec3f& getPosition(const unsigned int i) const 
    {
      return _x[i];
    }

    void setPosition(const unsigned int i, const pxr::GfVec3f& pos)
    {
      _x[i] = pos;
    }

    pxr::GfVec3f& getPosition0(const unsigned int i)
    {
      return _x0[i];
    }

    const pxr::GfVec3f& getPosition0(const unsigned int i) const
    {
      return _x0[i];
    }

    void setPosition0(const unsigned int i, const pxr::GfVec3f& pos)
    {
      _x0[i] = pos;
    }

    pxr::GfVec3f& getLastPosition(const unsigned int i)
    {
      return _lastX[i];
    }

    const pxr::GfVec3f& getLastPosition(const unsigned int i) const
    {
      return _lastX[i];
    }

    void setLastPosition(const unsigned int i, const pxr::GfVec3f& pos)
    {
      _lastX[i] = pos;
    }

    pxr::GfVec3f& getOldPosition(const unsigned int i)
    {
      return _oldX[i];
    }

    const pxr::GfVec3f& getOldPosition(const unsigned int i) const
    {
      return _oldX[i];
    }

    void setOldPosition(const unsigned int i, const pxr::GfVec3f& pos)
    {
      _oldX[i] = pos;
    }
    
    pxr::GfVec3f& getVelocity(const unsigned int i)
    {
      return _v[i];
    }

    const pxr::GfVec3f& getVelocity(const unsigned int i) const 
    {
      return _v[i];
    }

    void setVelocity(const unsigned int i, const pxr::GfVec3f& vel)
    {
      _v[i] = vel;
    }

    pxr::GfVec3f& getAcceleration(const unsigned int i)
    {
      return _a[i];
    }

    const pxr::GfVec3f& getAcceleration(const unsigned int i) const 
    {
      return _a[i];
    }

    void setAcceleration(const unsigned int i, const pxr::GfVec3f& accel)
    {
      _a[i] = accel;
    }

    const float getMass(const unsigned int i) const
    {
      return _masses[i];
    }

    float& getMass(const unsigned int i)
    {
      return _masses[i];
    }

    void setMass(const unsigned int i, const float mass)
    {
      _masses[i] = mass;
      if (mass != 0.0)
        _invMasses[i] = 1.f / mass;
      else
        _invMasses[i] = 0.0;
    }

    const float getInvMass(const unsigned int i) const
    {
      return _invMasses[i];
    }

    const unsigned int getNumberOfParticles() const
    {
      return (unsigned int) _x.size();
    }

    /** Resize the array containing the particle data.
      */
    void resize(const unsigned int newSize)
    {
      _masses.resize(newSize);
      _invMasses.resize(newSize);
      _x0.resize(newSize);
      _x.resize(newSize);
      _v.resize(newSize);
      _a.resize(newSize);
      _oldX.resize(newSize);
      _lastX.resize(newSize);
    }

    /** Reserve the array containing the particle data.
      */
    void reserve(const unsigned int newSize)
    {
      _masses.reserve(newSize);
      _invMasses.reserve(newSize);
      _x0.reserve(newSize);
      _x.reserve(newSize);
      _v.reserve(newSize);
      _a.reserve(newSize);
      _oldX.reserve(newSize);
      _lastX.reserve(newSize);
    }

    /** Release the array containing the particle data.
      */
    void release()
    {
      _masses.clear();
      _invMasses.clear();
      _x0.clear();
      _x.clear();
      _v.clear();
      _a.clear();
      _oldX.clear();
      _lastX.clear();
    }

    /** Release the array containing the particle data.
      */
    unsigned int size() const 
    {
      return (unsigned int) _x.size();
    }
};

/** This class encapsulates the state of all orientations of a quaternion model.
* All parameters are stored in individual arrays.
*/
class PBDOrientation
{
private:
  // Mass
  // If the mass is zero, the particle is static
  std::vector<float> _masses;
  std::vector<float> _invMasses;

  // Dynamic state
  std::vector<pxr::GfQuatf> _q0;
  std::vector<pxr::GfQuatf> _q;
  std::vector<pxr::GfVec3f> _omega;
  std::vector<pxr::GfVec3f> _alpha;
  std::vector<pxr::GfQuatf> _oldQ;
  std::vector<pxr::GfQuatf> _lastQ;

public:
  void addQuaternion(const pxr::GfQuatf& vertex)
  {
    _q0.push_back(vertex);
    _q.push_back(vertex);
    _oldQ.push_back(vertex);
    _lastQ.push_back(vertex);
    _masses.push_back(1.0);
    _invMasses.push_back(1.0);
    _omega.push_back(Vector3r(0.0, 0.0, 0.0));
    _alpha.push_back(Vector3r(0.0, 0.0, 0.0));
  }

  pxr::GfQuatf& getQuaternion(const unsigned int i)
  {
    return _q[i];
  }

  const pxr::GfQuatf& getQuaternion(const unsigned int i) const
  {
    return _q[i];
  }

  void setQuaternion(const unsigned int i, const pxr::GfQuatf& pos)
  {
    _q[i] = pos;
  }

  pxr::GfQuatf& getQuaternion0(const unsigned int i)
  {
    return _q0[i];
  }

  const pxr::GfQuatf& getQuaternion0(const unsigned int i) const
  {
    return _q0[i];
  }

  void setQuaternion0(const unsigned int i, const pxr::GfQuatf& pos)
  {
    _q0[i] = pos;
  }

  pxr::GfQuatf& getLastQuaternion(const unsigned int i)
  {
    return _lastQ[i];
  }

  const pxr::GfQuatf& getLastQuaternion(const unsigned int i) const
  {
    return _lastQ[i];
  }

  void setLastQuaternion(const unsigned int i, const pxr::GfQuatf& pos)
  {
    _lastQ[i] = pos;
  }

  pxr::GfQuatf& getOldQuaternion(const unsigned int i)
  {
    return _oldQ[i];
  }

  const pxr::GfQuatf& getOldQuaternion(const unsigned int i) const
  {
    return _oldQ[i];
  }

  void setOldQuaternion(const unsigned int i, const pxr::GfQuatf& pos)
  {
    _oldQ[i] = pos;
  }

  pxr::GfVec3f& getVelocity(const unsigned int i)
  {
    return _omega[i];
  }

  const pxr::GfVec3f& getVelocity(const unsigned int i) const
  {
    return _omega[i];
  }

  void setVelocity(const unsigned int i, const pxr::GfVec3f& vel)
  {
    _omega[i] = vel;
  }

  pxr::GfVec3f& getAcceleration(const unsigned int i)
  {
    return _alpha[i];
  }

  const pxr::GfVec3f& getAcceleration(const unsigned int i) const
  {
    return _alpha[i];
  }

  void setAcceleration(const unsigned int i, const pxr::GfVec3f& accel)
  {
    _alpha[i] = accel;
  }

  const float getMass(const unsigned int i) const
  {
    return _masses[i];
  }

  float& getMass(const unsigned int i)
  {
    return _masses[i];
  }

  void setMass(const unsigned int i, const float mass)
  {
    _masses[i] = mass;
    if (mass != 0.f)
      _invMasses[i] = 1.f / mass;
    else
      _invMasses[i] = 0.f;
  }

  const float getInvMass(const unsigned int i) const
  {
    return _invMasses[i];
  }

  const unsigned int getNumberOfQuaternions() const
  {
    return (unsigned int)_q.size();
  }

  /** Resize the array containing the particle data.
  */
  void resize(const unsigned int newSize)
  {
    _masses.resize(newSize);
    _invMasses.resize(newSize);
    _q0.resize(newSize);
    _q.resize(newSize);
    _omega.resize(newSize);
    _alpha.resize(newSize);
    _oldQ.resize(newSize);
    _lastQ.resize(newSize);
  }

  /** Reserve the array containing the particle data.
  */
  void reserve(const unsigned int newSize)
  {
    _masses.reserve(newSize);
    _invMasses.reserve(newSize);
    _q0.reserve(newSize);
    _q.reserve(newSize);
    _omega.reserve(newSize);
    _alpha.reserve(newSize);
    _oldQ.reserve(newSize);
    _lastQ.reserve(newSize);
  }

  /** Release the array containing the particle data.
  */
  void release()
  {
    _masses.clear();
    _invMasses.clear();
    _q0.clear();
    _q.clear();
    _omega.clear();
    _alpha.clear();
    _oldQ.clear();
    _lastQ.clear();
  }

  /** Release the array containing the particle data.
  */
  unsigned int size() const
  {
    return (unsigned int)_q.size();
  }
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif