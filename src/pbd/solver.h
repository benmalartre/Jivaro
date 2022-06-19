#ifndef JVR_PBD_SOLVER_H
#define JVR_PBD_SOLVER_H

#include <vector>
#include "../pbd/rigidbody.h"
#include "../pbd/particle.h"
#include "../pbd/triangle.h"
#include "../pbd/tet.h"
#include "../pbd/line.h"

PXR_NAMESPACE_OPEN_SCOPE

	class PBDConstraint;

	class PBDSolver
	{
		public:
			PBDSolver();
			PBDSolver(const PBDSolver&) = delete;
			PBDSolver& operator=(const PBDSolver&) = delete;
			virtual ~PBDSolver();

			void nit();

			typedef std::vector<PBDConstraint*> PBDConstraintVector;
			typedef std::vector<PBDRigidBodyContactConstraint> PBDRigidBodyContactConstraintVector;
			typedef std::vector<PBDParticleRigidBodyContactConstraint> PBDParticleRigidBodyContactConstraintVector;
			typedef std::vector<PBDParticleTetContactConstraint> PBDParticleSolidContactConstraintVector;
			typedef std::vector<PBDRigidBody*> PBDRigidBodyVector;
			typedef std::vector<PBDTriangle*> PBDTriangleVector;
			typedef std::vector<PBDTet*> PBDTetVector;
			typedef std::vector<PBDLine*> PBDLineVector;
			typedef std::vector<unsigned int> PBDConstraintGroup;
			typedef std::vector<PBDConstraintGroup> PBDConstraintGroupVector;


		protected:
			PBDRigidBodyVector                          _rigidBodies;
			PBDTriangleVector                           _triangles;
			PBDTetModelVector                           _tets;
			PBDLineModelVector                          _lines;
			PBDParticle                                 _particles;
			PBDOrientation                              _orientations;
			PBDConstraintVector                         _constraints;
			PBDRigidBodyContactConstraintVector         _rigidBodyContactConstraints;
			PBDParticleRigidBodyContactConstraintVector _particleRigidBodyContactConstraints;
			PBDParticleSolidContactConstraintVector     _particleSolidContactConstraints;
			PBDConstraintGroupVector                    _constraintGroups;

			float                                       _contactStiffnessRigidBody;
			float                                       _contactStiffnessParticleRigidBody;

	public:
			void Reset();			
			void Cleanup();

			PBDRigidBodyVector& GetRigidBodies();
			PBDParticle& GetParticles();
			PBDOrientation& GetOrientations();
			PBDTriangleVector& GetTriangles();
			PBDTetVector& GetTets();
			PBDLineVector& GetLines();
			PBDConstraintVector& GetConstraints();
			PBDRigidBodyContactConstraintVector& GetRigidBodyContactConstraints();
			PBDParticleRigidBodyContactConstraintVector& GetParticleRigidBodyContactConstraints();
			PBDParticleSolidContactConstraintVector& GetParticleSolidContactConstraints();
			PBDConstraintGroupVector& GetConstraintGroups();
			bool _groupsInitialized;

			void ResetContacts();

			void AddTriangleModel(
				const unsigned int nPoints,
				const unsigned int nFaces,
				pxr::GfVec3f* points,
				unsigned int* indices,
				const PBDTriangle::ParticleMesh::UVIndices& uvIndices,
				const PBDTriangle::ParticleMesh::UVs& uvs);

			void AddRegularTriangleModel(const int width, const int height,
				const Vector3r& translation = Vector3r::Zero(), 
				const Matrix3r& rotation = Matrix3r::Identity(), 
				const Vector2r& scale = Vector2r::Ones());

			void AddTetModel(
				const unsigned int nPoints, 
				const unsigned int nTets, 
				pxr::GfVec3f* points,
				unsigned int* indices);

			void AddRegularTetModel(const int width, const int height, const int depth,
				const pxr::GfVec3f& translation =  pxr::GfVec3f(0.f),
				const pxr::GfMatrix3f& rotation = pxr::GfMatrix3f::Identity(), 
				const pxr::GfVec3f& scale = pxr::GfVec3f(1.f));

			void AddLineModel(
				const unsigned int nPoints,
				const unsigned int nQuaternions,
				pxr::GfVec3f* points,
				pxr::GfQuaternionf* quaternions,
				unsigned int* indices,
				unsigned int* indicesQuaternions);

			void UpdateConstraints();
			void InitConstraintGroups();

			bool AddBallJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos);
			bool AddBallOnLineJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& dir);
			bool AddHingeJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
			bool AddTargetAngleMotorHingeJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
			bool AddTargetVelocityMotorHingeJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
			bool AddUniversalJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis1, const pxr::GfVec3f& axis2);
			bool AddSliderJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
			bool AddTargetPositionMotorSliderJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
			bool AddTargetVelocityMotorSliderJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
			bool AddRigidBodyParticleBallJoint(const unsigned int rbIndex, const unsigned int particleIndex);
			bool AddRigidBodySpring(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos1, const pxr::GfVec3f& pos2, const float stiffness);
			bool AddDistanceJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos1, const pxr::GfVec3f& pos2);
			bool AddDamperJoint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis, const float stiffness);
			bool AddRigidBodyContactConstraint(const unsigned int rbIndex1, const unsigned int rbIndex2, 
					const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2,	
					const pxr::GfVec3f& normal, const float dist, 
					const float restitutionCoeff, const float frictionCoeff);
			bool AddParticleRigidBodyContactConstraint(const unsigned int particleIndex, const unsigned int rbIndex, 
					const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2, 
					const pxr::GfVec3f& normal, const float dist,
					const float restitutionCoeff, const float frictionCoeff);

			bool AddParticleSolidContactConstraint(const unsigned int particleIndex, const unsigned int solidIndex,
				const unsigned int tetIndex, const pxr::GfVec3f& bary,
				const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2,
				const pxr::GfVec3f& normal, const float dist,
				const float restitutionCoeff, const float frictionCoeff);

			bool AddDistanceConstraint(const unsigned int particle1, const unsigned int particle2, const float stiffness);
			bool AddDistanceConstraint_XPBD(const unsigned int particle1, const unsigned int particle2, const float stiffness);
			bool AddDihedralConstraint(	const unsigned int particle1, const unsigned int particle2,
										const unsigned int particle3, const unsigned int particle4, const float stiffness);
			bool AddIsometricBendingConstraint(const unsigned int particle1, const unsigned int particle2,
										const unsigned int particle3, const unsigned int particle4, const float stiffness);
			bool AddIsometricBendingConstraint_XPBD(const unsigned int particle1, const unsigned int particle2,
										const unsigned int particle3, const unsigned int particle4, const float stiffness);
			bool AddFEMTriangleConstraint(const unsigned int particle1, const unsigned int particle2, const unsigned int particle3, 
				const float xxStiffness, const float yyStiffness, const float xyStiffness,
				const float xyPoissonRatio, const float yxPoissonRatio);
			bool AddStrainTriangleConstraint(const unsigned int particle1, const unsigned int particle2, 
				const unsigned int particle3, const float xxStiffness, const float yyStiffness, const float xyStiffness,
				const bool normalizeStretch, const bool normalizeShear);
			bool AddVolumeConstraint(const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, const float stiffness);
			bool AddVolumeConstraint_XPBD(const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, const float stiffness);
			bool AddFEMTetConstraint(const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, 
									const float stiffness, const float poissonRatio);
			bool AddStrainTetConstraint(const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, 
									const float stretchStiffness, const float shearStiffness,
									const bool normalizeStretch, const bool normalizeShear);
			bool AddShapeMatchingConstraint(const unsigned int numberOfParticles, const unsigned int particleIndices[], const unsigned int numClusters[], const float stiffness);
			bool AddStretchShearConstraint(const unsigned int particle1, const unsigned int particle2, 
				const unsigned int quaternion1, const float stretchingStiffness,
				const float shearingStiffness1, const float shearingStiffness2);
			bool AddBendTwistConstraint(const unsigned int quaternion1, const unsigned int quaternion2, 
				const float twistingStiffness, const float bendingStiffness1, const float bendingStiffness2);
			bool AddStretchBendingTwistingConstraint(const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const float averageRadius, const float averageSegmentLength, const float youngsModulus, const float torsionModulus);
			bool AddDirectPositionBasedSolverForStiffRodsConstraint(const std::vector<std::pair<unsigned int, unsigned int>> & jointSegmentIndices, const std::vector<Vector3r> &jointPositions, const std::vector<Real> &averageRadii, const std::vector<Real> &averageSegmentLengths, const std::vector<Real> &youngsModuli, const std::vector<Real> &torsionModuli);

			float GetContactStiffnessRigidBody() const { return _contactStiffnessRigidBody; }
			void GetContactStiffnessRigidBody(float val) { _contactStiffnessRigidBody = val; }
			float GetContactStiffnessParticleRigidBody() const { return _contactStiffnessParticleRigidBody; }
			void SetContactStiffnessParticleRigidBody(float val) { _contactStiffnessParticleRigidBody = val; }
		
			void AddClothConstraints(const PBDTriangle* tm, const unsigned int clothMethod, 
				const float distanceStiffness, const float xxStiffness, const float yyStiffness,
				const float xyStiffness, const float xyPoissonRatio, const float yxPoissonRatio,
				const bool normalizeStretch, const bool normalizeShear);
			void AddBendingConstraints(const PBDTriangle* tm, const unsigned int bendingMethod, const float stiffness);
			void AddSolidConstraints(const TetModel* tm, const unsigned int solidMethod, const float stiffness, 
				const float poissonRatio, const float volumeStiffness, 
				const bool normalizeStretch, const bool normalizeShear);

			template<typename ConstraintType, typename T, T ConstraintType::* MemPtr>
			void SetConstraintValue(const T v)
			{
				for (auto i = 0; i < m_constraints.size(); i++)
				{
					ConstraintType* c = dynamic_cast<ConstraintType*>(m_constraints[i]);
					if (c != nullptr)
						c->*MemPtr = v;
				}
			}
	};
}

#endif