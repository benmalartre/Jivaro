#ifndef JVR_PBD_RIGIDBODY_H
#define JVR_PBD_RIGIDBODY_H

#include <vector>
#include "../common.h"
#include "../objects/mesh.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matri3f.h>
#include <pxr/base/gf/quatf.h>
#include "eigenSolver.h"

//#include "RigidBodyGeometry.h"
//#include "Utils/VolumeIntegration.h"

PXR_NAMESPACE_OPEN_SCOPE

// This class encapsulates the state of a rigid body.
//
class RigidBody
{
  private:
    // mass
    float _mass;
    // inverse mass
    float _invMass;
    // center of mass
    pxr::GfVec3f _x;
    pxr::GfVec3f _lastX;
    pxr::GfVec3f _oldX;
    pxr::GfVec3f _x0;
    // center of mass velocity
    pxr::GfVec3f _v;
    pxr::GfVec3f _v0;
    // acceleration (by external forces)
    pxr::GfVec3f _a;

    // Inertia tensor in the principal axis system: \n
    // After the main axis transformation the inertia tensor is a diagonal matrix.
    // So only three values are required to store the inertia tensor. These values
    // are constant over time.
    //
    pxr::GfVec3f _inertiaTensor;
    // Inverse inertia tensor in body space
    pxr::GfVec3f _inertiaTensorInverse;
    // 3x3 matrix, inverse of the inertia tensor in world space
    pxr::GfMatrix3f _inertiaTensorInverseW;
    // Quaternion that describes the rotation of the body in world space
    pxr::GfQuatf _q;
    pxr::GfQuatf _lastQ;
    pxr::GfQuatf _oldQ;
    pxr::GfQuatf _q0;
    // Quaternion representing the rotation of the main axis transformation
    // that is performed to get a diagonal inertia tensor
    pxr::GfQuatf _q_mat;
    // Quaternion representing the initial rotation of the geometry
    pxr::GfQuatf _q_initial;
    // difference of the initial translation and the translation of the main axis transformation
    pxr::GfVec3f _x0_mat;
    // rotationMatrix = 3x3 matrix. 
    // Important for the transformation from world in body space and vice versa.
    // When using quaternions the rotation matrix is computed out of the quaternion.
    //
    pxr::GfMatrix3f _rot;
    // Angular velocity, defines rotation axis and velocity (magnitude of the vector)
    pxr::GfVec3f _omega;
    pxr::GfVec3f _omega0;
    // external torque
    pxr::GfVec3f _torque;

    float _restitutionCoeff;
    float _frictionCoeff;

    PBDGeometry _geometry;

    // transformation required to transform a point to local space or vice vera
    pxr::GfMatrix3f _transformation_R;
    pxr::GfVec3f _transformation_v1;
    pxr::GfVec3f _transformation_v2;
    pxr::GfVec3f _transformation_R_X_v1;
    
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
      _v.SetZero();
      _v0.SetZero();
      _a.SetZero();

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
      m_mass = 1.0;
      m_inertiaTensor = Vector3r(1.0, 1.0, 1.0);
      m_x = x;
      m_x0 = x;
      m_lastX = x;
      m_oldX = x;
      m_v.setZero();
      m_v0.setZero();
      m_a.setZero();

      m_q = rotation;
      m_q0 = rotation;
      m_lastQ = rotation;
      m_oldQ = rotation;
      m_rot = m_q.matrix();
      rotationUpdated();
      m_omega.setZero();
      m_omega0.setZero();
      m_torque.setZero();

      m_restitutionCoeff = static_cast<Real>(0.6);
      m_frictionCoeff = static_cast<Real>(0.2);

      getGeometry().initMesh(vertices.size(), mesh.numFaces(), &vertices.getPosition(0), mesh.getFaces().data(), mesh.getUVIndices(), mesh.getUVs(), scale);
      determineMassProperties(density);
      getGeometry().updateMeshTransformation(getPosition(), getRotationMatrix());
    }

    void reset()
    {
      getPosition() = getPosition0();
      getOldPosition() = getPosition0();
      getLastPosition() = getPosition0();

      getRotation() = getRotation0();
      getOldRotation() = getRotation0();
      getLastRotation() = getRotation0();

      getVelocity() = getVelocity0();
      getAngularVelocity() = getAngularVelocity0();

      getAcceleration().setZero();
      getTorque().setZero();

      rotationUpdated();
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
      // apply initial rotation
      VertexData &vd = m_geometry.getVertexDataLocal();
      
      Utilities::VolumeIntegration vi(m_geometry.getVertexDataLocal().size(), m_geometry.getMesh().numFaces(), &m_geometry.getVertexDataLocal().getPosition(0), m_geometry.getMesh().getFaces().data());
      vi.compute_inertia_tensor(density);

      // Diagonalize Inertia Tensor
      NI
      Eigen::SelfAdjointEigenSolver<Matrix3r> es(vi.getInertia());
      Vector3r inertiaTensor = es.eigenvalues();
      Matrix3r R = es.eigenvectors();

      SetMass(vi.getMass());
      SetInertiaTensor(inertiaTensor);

      if (R.GetDeterminant() < 0.0)
        R = -R;

      for (unsigned int i = 0; i < vd.size(); i++)
        vd.getPosition(i) = m_rot * vd.getPosition(i) + m_x0;

      Vector3r x_MAT = vi.getCenterOfMass();
      R = m_rot * R;
      x_MAT = m_rot * x_MAT + m_x0;

      // rotate vertices back				
      for (unsigned int i = 0; i < vd.size(); i++)
        vd.getPosition(i) = R.transpose() * (vd.getPosition(i) - x_MAT);

      // set rotation
      Quaternionr qR = Quaternionr(R);
      qR.normalize();
      m_q_mat = qR;
      m_q_initial = m_q0;
      m_x0_mat = m_x0 - x_MAT;

      m_q0 = qR;
      m_q = m_q0;
      m_lastQ = m_q0;
      m_oldQ = m_q0;
      rotationUpdated();

      // set translation
      m_x0 = x_MAT;
      m_x = m_x0;
      m_lastX = m_x0;
      m_oldX = m_x0;
      updateInverseTransformation();
    }

    const Matrix3r &getTransformationR() { return m_transformation_R;  }
    const Vector3r &getTransformationV1() { return m_transformation_v1; }
    const Vector3r &getTransformationV2() { return m_transformation_v2; }
    const Vector3r &getTransformationRXV1() { return m_transformation_R_X_v1; }

    FORCE_INLINE Real &getMass()
    {
      return m_mass;
    }

    FORCE_INLINE const Real &getMass() const
    {
      return m_mass;
    }

    FORCE_INLINE void setMass(const Real &value)
    {
      m_mass = value;
      if (m_mass != 0.0)
        m_invMass = static_cast<Real>(1.0) / m_mass;
      else
        m_invMass = 0.0;
    }

    FORCE_INLINE const Real &getInvMass() const
    {
      return m_invMass;
    }

    FORCE_INLINE Vector3r &getPosition()
    {
      return m_x;
    }

    FORCE_INLINE const Vector3r &getPosition() const 
    {
      return m_x;
    }

    FORCE_INLINE void setPosition(const Vector3r &pos)
    {
      m_x = pos;
    }

    FORCE_INLINE Vector3r &getLastPosition()
    {
      return m_lastX;
    }

    FORCE_INLINE const Vector3r &getLastPosition() const
    {
      return m_lastX;
    }

    FORCE_INLINE void setLastPosition(const Vector3r &pos)
    {
      m_lastX = pos;
    }

    FORCE_INLINE Vector3r &getOldPosition()
    {
      return m_oldX;
    }

    FORCE_INLINE const Vector3r &getOldPosition() const
    {
      return m_oldX;
    }

    FORCE_INLINE void setOldPosition(const Vector3r &pos)
    {
      m_oldX = pos;
    }

    FORCE_INLINE Vector3r &getPosition0()
    {
      return m_x0;
    }

    FORCE_INLINE const Vector3r &getPosition0() const
    {
      return m_x0;
    }

    FORCE_INLINE void setPosition0(const Vector3r &pos)
    {
      m_x0 = pos;
    }

    FORCE_INLINE Vector3r &getPositionInitial_MAT()
    {
      return m_x0_mat;
    }

    FORCE_INLINE const Vector3r &getPositionInitial_MAT() const
    {
      return m_x0_mat;
    }

    FORCE_INLINE void setPositionInitial_MAT(const Vector3r &pos)
    {
      m_x0_mat = pos;
    }

    FORCE_INLINE Vector3r &getVelocity()
    {
      return m_v;
    }

    FORCE_INLINE const Vector3r &getVelocity() const
    {
      return m_v;
    }

    FORCE_INLINE void setVelocity(const Vector3r &value)
    {
      m_v = value;
    }			

    FORCE_INLINE Vector3r &getVelocity0()
    {
      return m_v0;
    }

    FORCE_INLINE const Vector3r &getVelocity0() const
    {
      return m_v0;
    }

    FORCE_INLINE void setVelocity0(const Vector3r &value)
    {
      m_v0 = value;
    }

    FORCE_INLINE Vector3r &getAcceleration()
    {
      return m_a;
    }

    FORCE_INLINE const Vector3r &getAcceleration() const 
    {
      return m_a;
    }

    FORCE_INLINE void setAcceleration(const Vector3r &accel)
    {
      m_a = accel;
    }

    FORCE_INLINE const Vector3r &getInertiaTensor() const
    {
      return m_inertiaTensor;
    }

    FORCE_INLINE void setInertiaTensor(const Vector3r &value)
    {
      m_inertiaTensor = value;
      m_inertiaTensorInverse = Vector3r(static_cast<Real>(1.0) / value[0], static_cast<Real>(1.0) / value[1], static_cast<Real>(1.0) / value[2]);
    }

    FORCE_INLINE const Vector3r &getInertiaTensorInverse() const
    {
      return m_inertiaTensorInverse;
    }

    FORCE_INLINE Matrix3r &getInertiaTensorInverseW()
    {
      return m_inertiaTensorInverseW;
    }

    FORCE_INLINE const Matrix3r &getInertiaTensorInverseW() const
    {
      return m_inertiaTensorInverseW;
    }

    FORCE_INLINE void setInertiaTensorInverseW(const Matrix3r &value)
    {
      m_inertiaTensorInverseW = value;
    }

    FORCE_INLINE Quaternionr &getRotation()
    {
      return m_q;
    }

    FORCE_INLINE const Quaternionr &getRotation() const
    {
      return m_q;
    }

    FORCE_INLINE void setRotation(const Quaternionr &value)
    {
      m_q = value;
    }

    FORCE_INLINE Quaternionr &getLastRotation()
    {
      return m_lastQ;
    }

    FORCE_INLINE const Quaternionr &getLastRotation() const
    {
      return m_lastQ;
    }

    FORCE_INLINE void setLastRotation(const Quaternionr &value)
    {
      m_lastQ = value;
    }

    FORCE_INLINE Quaternionr &getOldRotation()
    {
      return m_oldQ;
    }

    FORCE_INLINE const Quaternionr &getOldRotation() const
    {
      return m_oldQ;
    }

    FORCE_INLINE void setOldRotation(const Quaternionr &value)
    {
      m_oldQ = value;
    }

    FORCE_INLINE Quaternionr &getRotation0()
    {
      return m_q0;
    }

    FORCE_INLINE const Quaternionr &getRotation0() const
    {
      return m_q0;
    }

    FORCE_INLINE void setRotation0(const Quaternionr &value)
    {
      m_q0 = value;
    }

    FORCE_INLINE Quaternionr &getRotationMAT()
    {
      return m_q_mat;
    }

    FORCE_INLINE const Quaternionr &getRotationMAT() const
    {
      return m_q_mat;
    }

    FORCE_INLINE void setRotationMAT(const Quaternionr &value)
    {
      m_q_mat = value;
    }

    FORCE_INLINE Quaternionr &getRotationInitial()
    {
      return m_q_initial;
    }

    FORCE_INLINE const Quaternionr &getRotationInitial() const
    {
      return m_q_initial;
    }

    FORCE_INLINE void setRotationInitial(const Quaternionr &value)
    {
      m_q_initial = value;
    }

    FORCE_INLINE Matrix3r &getRotationMatrix()
    {
      return m_rot;
    }

    FORCE_INLINE const Matrix3r &getRotationMatrix() const
    {
      return m_rot;
    }

    FORCE_INLINE void setRotationMatrix(const Matrix3r &value)
    {
      m_rot = value;
    }

    FORCE_INLINE Vector3r &getAngularVelocity()
    {
      return m_omega;
    }

    FORCE_INLINE const Vector3r &getAngularVelocity() const
    {
      return m_omega;
    }

    FORCE_INLINE void setAngularVelocity(const Vector3r &value)
    {
      m_omega = value;
    }

    FORCE_INLINE Vector3r &getAngularVelocity0()
    {
      return m_omega0;
    }

    FORCE_INLINE const Vector3r &getAngularVelocity0() const
    {
      return m_omega0;
    }

    FORCE_INLINE void setAngularVelocity0(const Vector3r &value)
    {
      m_omega0 = value;
    }

    FORCE_INLINE Vector3r &getTorque()
    {
      return m_torque;
    }

    FORCE_INLINE const Vector3r &getTorque() const
    {
      return m_torque;
    }

    FORCE_INLINE void setTorque(const Vector3r &value)
    {
      m_torque = value;
    }

    FORCE_INLINE Real getRestitutionCoeff() const 
    { 
      return m_restitutionCoeff; 
    }

    FORCE_INLINE void setRestitutionCoeff(Real val) 
    { 
      m_restitutionCoeff = val; 
    }

    FORCE_INLINE Real getFrictionCoeff() const 
    { 
      return m_frictionCoeff; 
    }

    FORCE_INLINE void setFrictionCoeff(Real val) 
    { 
      m_frictionCoeff = val; 
    }

    RigidBodyGeometry& getGeometry()
    {
      return m_geometry;
    }
};
}

#endif