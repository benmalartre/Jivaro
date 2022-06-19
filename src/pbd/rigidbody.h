#ifndef JVR_PBD_RIGIDBODY_H
#define JVR_PBD_RIGIDBODY_H

#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/quatf.h>

#include "../common.h"
#include "../objects/mesh.h"

#include "../pbd/eigenSolver.h"
#include "../pbd/volume.h"

//#include "RigidBodyGeometry.h"
//#include "Utils/VolumeIntegration.h"

PXR_NAMESPACE_OPEN_SCOPE

// This class encapsulates the state of a rigid body.
//
class RigidBody
{
  private:
    // mass
    float                   _mass;
    // inverse mass
    float                   _invMass;
    // center of mass
    pxr::GfVec3f            _x;
    pxr::GfVec3f            _lastX;
    pxr::GfVec3f            _oldX;
    pxr::GfVec3f            _x0;
    // center of mass velocity
    pxr::GfVec3f            _v;
    pxr::GfVec3f            _v0;
    // acceleration (by external forces)
    pxr::GfVec3f            _a;

    // Inertia tensor in the principal axis system: \n
    // After the main axis transformation the inertia tensor is a diagonal matrix.
    // So only three values are required to store the inertia tensor. These values
    // are constant over time.
    //
    pxr::GfVec3f            _inertiaTensor;
    // Inverse inertia tensor in body space
    pxr::GfVec3f            _inertiaTensorInverse;
    // 3x3 matrix, inverse of the inertia tensor in world space
    pxr::GfMatrix3f         _inertiaTensorInverseW;
    // Quaternion that describes the rotation of the body in world space
    pxr::GfQuatf            _q;
    pxr::GfQuatf            _lastQ;
    pxr::GfQuatf            _oldQ;
    pxr::GfQuatf            _q0;
    // Quaternion representing the rotation of the main axis transformation
    // that is performed to get a diagonal inertia tensor
    pxr::GfQuatf            _q_mat;
    // Quaternion representing the initial rotation of the geometry
    pxr::GfQuatf            _q_initial;
    // difference of the initial translation and the translation of the main axis transformation
    pxr::GfVec3f            _x0_mat;
    // rotationMatrix = 3x3 matrix. 
    // Important for the transformation from world in body space and vice versa.
    // When using quaternions the rotation matrix is computed out of the quaternion.
    //
    pxr::GfMatrix3f         _rot;
    // Angular velocity, defines rotation axis and velocity (magnitude of the vector)
    pxr::GfVec3f            _omega;
    pxr::GfVec3f            _omega0;
    // external torque
    pxr::GfVec3f            _torque;

    float                   _restitutionCoeff;
    float                   _frictionCoeff;

    PBDGeometry             _geometry;

    // transformation required to transform a point to local space or vice vera
    pxr::GfMatrix3f         _transformation_R;
    pxr::GfVec3f            _transformation_v1;
    pxr::GfVec3f            _transformation_v2;
    pxr::GfVec3f            _transformation_R_X_v1;
    
  public:
    void InitBody(const float mass, const pxr::GfVec3f& x, 
      const pxr::GfVec3f& inertiaTensor, const pxr::GfQuatf& rotation, 
      const Mesh& mesh, const pxr::GfVec3f& scale(1.f))
    {
      SetMass(mass);
      _x = x; 
      _x0 = x;
      _lastX = x;
      _oldX = x;
      _v = _v0 = _a = pxr::GfVec3f(0.f);

      SetInertiaTensor(inertiaTensor);
      _q = rotation;
      _q0 = rotation;
      _lastQ = rotation;
      _oldQ = rotation;
      _rot = pxr::GfMatrix3f(_q);
      _q_mat = pxr::GfQuatf(1.0, 0.0, 0.0, 0.0);
      _q_initial = pxr::GfQuatf(1.0, 0.0, 0.0, 0.0);
      _x0_mat.SetZero();
      RotationUpdated();
      _omega.SetZero();
      _omega0.SetZero();
      _torque.SetZero();

      _restitutionCoeff = 0.6f;
      _frictionCoeff = 0.2f;

      GetGeometry().InitMesh(vertices.size(), mesh.numFaces(), &vertices.getPosition(0), mesh.getFaces().data(), mesh.getUVIndices(), mesh.getUVs(), scale);
      GetGeometry().UpdateMeshTransformation(getPosition(), getRotationMatrix());
    }

    void InitBody(const float density, const pxr::GfVec3f& x, 
      const pxr::GfQuatf& rotation, const VertexData &vertices, const Utilities::IndexedFaceMesh &mesh, const Vector3r &scale = Vector3r(1.0, 1.0, 1.0))
    {
      _mass = 1.0;
      _inertiaTensor = pxr::GfVec3f(1.0, 1.0, 1.0);
      _x = x;
      _x0 = x;
      _lastX = x;
      _oldX = x;
      _v = _v0 = _a = pxr::GfVec3f(0.f);

      _q = rotation;
      _q0 = rotation;
      _lastQ = rotation;
      _oldQ = rotation;
      _rot = _q.matrix();
      RotationUpdated();
      _omega.setZero();
      _omega0.setZero();
      _torque.setZero();

      _restitutionCoeff = static_cast<float>(0.6);
      _frictionCoeff = static_cast<float>(0.2);

      GetGeometry().InitMesh(vertices.size(), mesh.numFaces(), &vertices.getPosition(0), mesh.getFaces().data(), mesh.getUVIndices(), mesh.getUVs(), scale);
      DetermineMassProperties(density);
      GetGeometry().UpdateMeshTransformation(GetPosition(), GetRotationMatrix());
    }

    void Reset()
    {
      GetPosition() = GetPosition0();
      GetOldPosition() = GetPosition0();
      GetLastPosition() = GetPosition0();

      GetRotation() = GetRotation0();
      GetOldRotation() = GetRotation0();
      GetLastRotation() = GetRotation0();

      GetVelocity() = GetVelocity0();
      GetAngularVelocity() = GetAngularVelocity0();

      GetAcceleration() = pxr::GfVec3f(0.f);
      GetTorque() = pxr::GfVec3f(0.f);

      RotationUpdated();
    }

    void UpdateInverseTransformation()
    {
      // remove the rotation of the main axis transformation that is performed
      // to get a diagonal inertia tensor since the distance function is 
      // evaluated in local coordinates
      //
      // transformation world to local:
      // p_local = R_initial^T ( R_MAT R^T (p_world - x) - x_initial + x_MAT)
      // 
      // transformation local to world:
      // p_world = R R_MAT^T (R_initial p_local + x_initial - x_MAT) + x
      //
      _transformation_R = (
        GetRotationInitial().GetInverse() * 
        GetRotationMAT() * 
        GetRotation().GetInverse()).matrix();
      _transformation_v1 = 
        -GetRotationInitial().GetInverse().matrix() * 
        GetPositionInitial_MAT();
      _transformation_v2 = (
        GetRotation() * 
        GetRotationMAT().GetInverse()).matrix() * 
        GetPositionInitial_MAT() + GetPosition();
      _transformation_R_X_v1 = 
        -_transformation_R * GetPosition() + _transformation_v1;
    }

    void RotationUpdated()
    {
      if (_mass != 0.0)
      {
        _rot = pxr::GfMatrix3f(_q);
        UpdateInverseInertiaW();
        UpdateInverseTransformation();
      }
    }

    void UpdateInverseInertiaW()
    {
      if (_mass != 0.0)
      {
        _inertiaTensorInverseW = 
          _rot * _inertiaTensorInverse.GetDiagonal() * _rot.GetTranspose();
      }
    }

    // Determine mass and inertia tensor of the given geometry.
    //
    void DetermineMassProperties(const float density)
    {
      pxr::VtArray<pxr::GfVec3f>& points = _geometry.GetPositions();
      
      PBDVolume volume(points.size(), m_geometry.getMesh().numFaces(), &m_geometry.getVertexDataLocal().getPosition(0), m_geometry.getMesh().getFaces().data());
      volume.ComputeInertiaTensor(density);

      // Diagonalize Inertia Tensor
      pxr::GfMatrix3f vi = volume.GetInertia();
      
      pxr::GfVec3f inertiaTensor;
      pxr::GfMatrix3f R;

      NISymmetricEigensolver3x3 solver;
      solver(vi[0][0], vi[0][1], vi[0][2], vi[1][1], vi[1][2], vi[2][2], 1, inertiaTensor, R);
     
      SetMass(volume.GetMass());
      SetInertiaTensor(inertiaTensor);

      if (R.GetDeterminant() < 0.0)
        R = -R;

      for (unsigned int i = 0; i < points.size(); i++)
        points[i] = _rot * points[i] + _x0;

      pxr::GfVec3f x_MAT = volume.GetCenterOfMass();
      R = _rot * R;
      x_MAT = _rot * x_MAT + _x0;

      // rotate vertices back				
      for (unsigned int i = 0; i < points.size(); i++)
        points[i] = R.Transpose() * (points[i] - x_MAT);

      // set rotation
      pxr::GfQuaternionf qR = pxr::GfQuaternionf(R);
      qR.Normalize();
      _q_mat = qR;
      _q_initial = m_q0;
      _x0_mat = m_x0 - x_MAT;

      _q0 = qR;
      _q = _q0;
      _lastQ = _q0;
      _oldQ = _q0;
      RotationUpdated();

      // set translation
      _x0 = x_MAT;
      _x = _x0;
      _lastX = _x0;
      _oldX = _x0;
      UpdateInverseTransformation();
    }

    const pxr::GfMatrix3f& GetTransformationR() { return _transformation_R;  }
    const pxr::GfVec3f& GetTransformationV1() { return _transformation_v1; }
    const pxr::GfVec3f& GetTransformationV2() { return _transformation_v2; }
    const pxr::GfVec3f& GetTransformationRXV1() { return _transformation_R_X_v1; }

    FORCE_INLINE float &GetMass()
    {
      return _mass;
    }

    FORCE_INLINE const float &GetMass() const
    {
      return _mass;
    }

    FORCE_INLINE void setMass(const float &value)
    {
      _mass = value;
      if (_mass != 0.0)
        _invMass = static_cast<float>(1.0) / _mass;
      else
        _invMass = 0.0;
    }

    FORCE_INLINE const float& GetInvMass() const
    {
      return _invMass;
    }

    FORCE_INLINE pxr::GfVec3f& GetPosition()
    {
      return _x;
    }

    FORCE_INLINE const pxr:GfVec3f& GetPosition() const 
    {
      return _x;
    }

    FORCE_INLINE void SetPosition(const pxr::GfVec3f& pos)
    {
      _x = pos;
    }

    FORCE_INLINE pxr::GfVec3f& GetLastPosition()
    {
      return _lastX;
    }

    FORCE_INLINE const pxr::GfVec3f& GetLastPosition() const
    {
      return _lastX;
    }

    FORCE_INLINE void SetLastPosition(const pxr::GfVec3f& pos)
    {
      _lastX = pos;
    }

    FORCE_INLINE pxr::GfVec3f& GetOldPosition()
    {
      return _oldX;
    }

    FORCE_INLINE const pxr::GfVec3f& GetOldPosition() const
    {
      return _oldX;
    }

    FORCE_INLINE void SetOldPosition(const pxr::GfVec3f& pos)
    {
      _oldX = pos;
    }

    FORCE_INLINE pxr::GfVec3f& GetPosition0()
    {
      return _x0;
    }

    FORCE_INLINE const pxr::GfVec3f& GetPosition0() const
    {
      return _x0;
    }

    FORCE_INLINE void SetPosition0(const pxr::GfVec3f& pos)
    {
      _x0 = pos;
    }

    FORCE_INLINE pxr::GfVec3f& GetPositionInitial_MAT()
    {
      return _x0_mat;
    }

    FORCE_INLINE const pxr::GfVec3f& GetPositionInitial_MAT() const
    {
      return _x0_mat;
    }

    FORCE_INLINE void SetPositionInitial_MAT(const pxr::GfVec3f& pos)
    {
      _x0_mat = pos;
    }

    FORCE_INLINE pxr::GfVec3f& GetVelocity()
    {
      return _v;
    }

    FORCE_INLINE const pxr::GfVec3f& GetVelocity() const
    {
      return _v;
    }

    FORCE_INLINE void SetVelocity(const pxr::GfVec3f& value)
    {
      _v = value;
    }			

    FORCE_INLINE pxr::GfVec3f& GetVelocity0()
    {
      return _v0;
    }

    FORCE_INLINE const pxr::GfVec3f& GetVelocity0() const
    {
      return _v0;
    }

    FORCE_INLINE void SetVelocity0(const pxr:GfVec3f& value)
    {
      _v0 = value;
    }

    FORCE_INLINE pxr::GfVec3f& GetAcceleration()
    {
      return _a;
    }

    FORCE_INLINE const pxr::GfVec3f& GetAcceleration() const 
    {
      return _a;
    }

    FORCE_INLINE void SetAcceleration(const pxr::GfVec3f& accel)
    {
      _a = accel;
    }

    FORCE_INLINE const pxr::GfVec3f& GetInertiaTensor() const
    {
      return _inertiaTensor;
    }

    FORCE_INLINE void SetInertiaTensor(const pxr::GfVec3f& value)
    {
      _inertiaTensor = value;
      _inertiaTensorInverse = pxr::GfVec3f(
        static_cast<float>(1.0) / value[0], 
        static_cast<float>(1.0) / value[1], 
        static_cast<float>(1.0) / value[2]);
    }

    FORCE_INLINE const pxr::GfVec3f& GetInertiaTensorInverse() const
    {
      return _inertiaTensorInverse;
    }

    FORCE_INLINE pxr::GfMatrix3f& GetInertiaTensorInverseW()
    {
      return _inertiaTensorInverseW;
    }

    FORCE_INLINE const pxr::GfMatrix3f& GetInertiaTensorInverseW() const
    {
      return _inertiaTensorInverseW;
    }

    FORCE_INLINE void setInertiaTensorInverseW(const pxr::GfMatrix3f& value)
    {
      _inertiaTensorInverseW = value;
    }

    FORCE_INLINE pxr::GfQuaternionf& GetRotation()
    {
      return _q;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetRotation() const
    {
      return _q;
    }

    FORCE_INLINE void SetRotation(const pxr::GfQuaternionf& value)
    {
      _q = value;
    }

    FORCE_INLINE pxr::GfQuaternionf& GetLastRotation()
    {
      return _lastQ;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetLastRotation() const
    {
      return _lastQ;
    }

    FORCE_INLINE void SetLastRotation(const pxr::GfQuaternionf& value)
    {
      _lastQ = value;
    }

    FORCE_INLINE pxr::GfQuaternionf& GetOldRotation()
    {
      return _oldQ;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetOldRotation() const
    {
      return _oldQ;
    }

    FORCE_INLINE void SetOldRotation(const pxr::GfQuaternionf& value)
    {
      _oldQ = value;
    }

    FORCE_INLINE pxr::GfQuaterniof& GetRotation0()
    {
      return _q0;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetRotation0() const
    {
      return _q0;
    }

    FORCE_INLINE void SetRotation0(const pxr::GfQuaterniong& value)
    {
      _q0 = value;
    }

    FORCE_INLINE pxr::GfQuaternionf& GetRotationMAT()
    {
      return _q_mat;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetRotationMAT() const
    {
      return _q_mat;
    }

    FORCE_INLINE void SetRotationMAT(const pxr::GfQuaternionf& value)
    {
      _q_mat = value;
    }

    FORCE_INLINE pxr::GfQuaternionf& GetRotationInitial()
    {
      return _q_initial;
    }

    FORCE_INLINE const pxr::GfQuaternionf& GetRotationInitial() const
    {
      return _q_initial;
    }

    FORCE_INLINE void SetRotationInitial(const pxr::GfQuaternionf& value)
    {
      _q_initial = value;
    }

    FORCE_INLINE pxr::GfMatrix3f& GetRotationMatrix()
    {
      return _rot;
    }

    FORCE_INLINE const pxr::GfMatrix3f& GetRotationMatrix() const
    {
      return _rot;
    }

    FORCE_INLINE void SetRotationMatrix(const pxr::GfMatrix3f& value)
    {
      _rot = value;
    }

    FORCE_INLINE pxr::GfVec3f& GetAngularVelocity()
    {
      return _omega;
    }

    FORCE_INLINE const pxr::GfVec3f& GetAngularVelocity() const
    {
      return _omega;
    }

    FORCE_INLINE void SetAngularVelocity(const pxr::GfVec3f& value)
    {
      _omega = value;
    }

    FORCE_INLINE pwr::GfVec3f& GetAngularVelocity0()
    {
      return _omega0;
    }

    FORCE_INLINE const pxr::GfVec3f& GetAngularVelocity0() const
    {
      return _omega0;
    }

    FORCE_INLINE void SetAngularVelocity0(const pxr::GfVec3f& value)
    {
      _omega0 = value;
    }

    FORCE_INLINE pxr::GfVec3f& GetTorque()
    {
      return _torque;
    }

    FORCE_INLINE const pxr::GfVec3f& GetTorque() const
    {
      return _torque;
    }

    FORCE_INLINE void SetTorque(const pxr::GfVec3f& value)
    {
      _torque = value;
    }

    FORCE_INLINE float GetRestitutionCoeff() const 
    { 
      return _restitutionCoeff; 
    }

    FORCE_INLINE void SetRestitutionCoeff(float val) 
    { 
      _restitutionCoeff = val; 
    }

    FORCE_INLINE float GetFrictionCoeff() const 
    { 
      return _frictionCoeff; 
    }

    FORCE_INLINE void SetFrictionCoeff(float val) 
    { 
      _frictionCoeff = val; 
    }

    Geometry* GetGeometry()
    {
      return _geometry;
    }
};
}

#endif