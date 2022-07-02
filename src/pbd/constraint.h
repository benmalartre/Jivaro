#ifndef JVR_PBD_CONSTRAINT_H
#define JVR_PBD_CONSTRAINT_H


#define _USE_MATH_DEFINES
#include "math.h"
#include <vector>
#include <array>
#include <list>
#include <memory>

	class PBDSolver;

	class PBDConstraint
	{
	public: 
		/** indices of the linked bodies */
		std::vector<unsigned int> _bodies;

		PBDConstraint(const unsigned int numberOfBodies) 
		{
			_bodies.resize(numberOfBodies); 
		}

		unsigned int NumberOfBodies() const { return static_cast<unsigned int>(_bodies.size()); }
		virtual ~PBDConstraint() {};
		virtual int &GetTypeId() const = 0;

		virtual bool InitConstraintBeforeProjection(PBDSolver& solver) { return true; };
		virtual bool UpdateConstraint(PBDSolver& solver) { return true; };
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter) { return true; };
		virtual bool SolveVelocityConstraint(PBDSolver& solver, const unsigned int iter) { return true; };
	};

	class PBDBallJoint : public PBDConstraint
	{
	public:
		static int TYPE_ID;
		//Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
        pxr::GfVec3f _jointInfo[4];

		PBDBallJoint() : PBDConstraint(2) {}
		virtual int &GetTypeId() const { return TYPE_ID; }

		bool InitConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos);
		virtual bool UpdateConstraint(PBDSolver& solver);
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};	

	class PBDBallOnLineJoint : public PBDConstraint
	{
	public:
		static int TYPE_ID;
		//Eigen::Matrix<Real, 3, 10, Eigen::DontAlign> m_jointInfo;
        pxr::GfVec3f _jointInfo[10];

		PBDBallOnLineJoint() : PBDConstraint(2) {} 
		virtual int &GetTypeId() const { return TYPE_ID; }

		bool InitConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& dir);
		virtual bool UpdateConstraint(PBDSolver& solver);
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};
 
	class PBDHingeJoint : public PBDConstraint
	{
	public:
		static int TYPE_ID;
		//Eigen::Matrix<Real, 4, 7, Eigen::DontAlign> m_jointInfo;
        pxr::GfVec4f _jointInfo[7];

		PBDHingeJoint() : PBDConstraint(2) {}
		virtual int &GetTypeId() const { return TYPE_ID; }

		bool InitConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
		virtual bool UpdateConstraint(PBDSolver& solver);
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};
 
	class PBDUniversalJoint : public PBDConstraint
	{
	public:
		static int TYPE_ID;
		//Eigen::Matrix<Real, 3, 8, Eigen::DontAlign> m_jointInfo;
        pxr::GfVec3f _jointInfo[8];

		PBDUniversalJoint() : PBDConstraint(2) {}
		virtual int &GetTypeId() const { return TYPE_ID; }

		bool InitConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis1, const pxr::GfVec3f& axis2);
		virtual bool UpdateConstraint(PBDSolver& solver);
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class PBDSliderJoint : public PBDConstraint
	{
	public:
		static int TYPE_ID;
		//Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
        pxr::GfVec4f _jointInfo[6];

		PBDSliderJoint() : PBDConstraint(2) {}
		virtual int &GetTypeId() const { return TYPE_ID; }

		bool InitConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
		virtual bool UpdateConstraint(PBDSolver& solver);
		virtual bool SolvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class PBDMotorJoint: public PBDConstraint
	{
	public:
		float _target;
		std::vector<float> _targetSequence;
		PBDMotorJoint() : PBDConstraint(2) { _target = 0.0; }

		virtual float GetTarget() const { return _target; }
		virtual void SetTarget(const float val) { _target = val; }

		virtual std::vector<float>& GetTargetSequence() { return _targetSequence; }
		virtual void SetTargetSequence(const std::vector<float>& val) { _targetSequence = val; }

		bool GetRepeatSequence() const { return _repeatSequence; }
		void SetRepeatSequence(bool val) { _repeatSequence = val; }

	private:
		bool _repeatSequence;
	};

	class PBDTargetPositionMotorSliderJoint : public PBDMotorJoint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;

		TargetPositionMotorSliderJoint() : MotorJoint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class TargetVelocityMotorSliderJoint : public MotorJoint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;

		TargetVelocityMotorSliderJoint() : MotorJoint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
		virtual bool solveVelocityConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class TargetAngleMotorHingeJoint : public MotorJoint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 4, 8, Eigen::DontAlign> m_jointInfo;
		TargetAngleMotorHingeJoint() : MotorJoint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual void setTarget(const Real val) 
		{ 
			const Real pi = (Real)M_PI;
			m_target = std::max(val, -pi);
			m_target = std::min(m_target, pi);
		}

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	private:
		std::vector<Real> m_targetSequence;
	};

	class TargetVelocityMotorHingeJoint : public MotorJoint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 4, 8, Eigen::DontAlign> m_jointInfo;
		TargetVelocityMotorHingeJoint() : MotorJoint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos, const pxr::GfVec3f& axis);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
		virtual bool solveVelocityConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class DamperJoint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
		Real m_lambda;

		DamperJoint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& axis, const Real stiffness);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};
 
	class RigidBodyParticleBallJoint : public Constraint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 3, 2, Eigen::DontAlign> m_jointInfo;

		RigidBodyParticleBallJoint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex, const unsigned int particleIndex);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class RigidBodySpring : public Constraint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
		Real m_restLength;
		Real m_stiffness;
		Real m_lambda;

		RigidBodySpring() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos1, const pxr::GfVec3f& pos2, const Real stiffness);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class DistanceJoint : public Constraint
	{
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
		Real m_restLength;

		DistanceJoint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, const pxr::GfVec3f& pos1, const pxr::GfVec3f& pos2);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};
 
	class DistanceConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restLength;
		Real m_stiffness;

		DistanceConstraint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2, const Real stiffness);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class DistanceConstraint_XPBD : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restLength;
		Real m_lambda;
		Real m_stiffness;

		DistanceConstraint_XPBD() : Constraint(2) {}
		virtual int& getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel& model, const unsigned int particle1, const unsigned int particle2, const Real stiffness);
		virtual bool solvePositionConstraint(SimulationModel& model, const unsigned int iter);
	};

	class DihedralConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restAngle;
		Real m_stiffness;

		DihedralConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, const Real stiffness);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};
	
	class IsometricBendingConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Matrix4r m_Q;

		IsometricBendingConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, const Real stiffness);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class IsometricBendingConstraint_XPBD : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Matrix4r m_Q;
		Real m_lambda;

		IsometricBendingConstraint_XPBD() : Constraint(4) {}
		virtual int& getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel& model, const unsigned int particle1, const unsigned int particle2,
					const unsigned int particle3, const unsigned int particle4, const Real stiffness);
		virtual bool solvePositionConstraint(SimulationModel& model, const unsigned int iter);
	};

	class FEMTriangleConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_area;
		Matrix2r m_invRestMat;
		Real m_xxStiffness;
		Real m_xyStiffness;
		Real m_yyStiffness;
		Real m_xyPoissonRatio;
		Real m_yxPoissonRatio;

		FEMTriangleConstraint() : Constraint(3) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
			const unsigned int particle3, const Real xxStiffness, const Real yyStiffness, const Real xyStiffness, 
			const Real xyPoissonRatio, const Real yxPoissonRatio);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class StrainTriangleConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Matrix2r m_invRestMat;
		Real m_xxStiffness;
		Real m_xyStiffness;
		Real m_yyStiffness;
		bool m_normalizeStretch;
		bool m_normalizeShear;

		StrainTriangleConstraint() : Constraint(3) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
			const unsigned int particle3, const Real xxStiffness, const Real yyStiffness, const Real xyStiffness, 
			const bool normalizeStretch, const bool normalizeShear);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class VolumeConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Real m_restVolume;

		VolumeConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
								const unsigned int particle3, const unsigned int particle4, const Real stiffness);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class VolumeConstraint_XPBD : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Real m_restVolume;
		Real m_lambda;

		VolumeConstraint_XPBD() : Constraint(4) {}
		virtual int& getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel& model, const unsigned int particle1, const unsigned int particle2,
			const unsigned int particle3, const unsigned int particle4, const Real stiffness);
		virtual bool solvePositionConstraint(SimulationModel& model, const unsigned int iter);
	};

	class FEMTetConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Real m_poissonRatio;
		Real m_volume;
		Matrix3r m_invRestMat;

		FEMTetConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4, 
									const Real stiffness, const Real poissonRatio);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class StrainTetConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stretchStiffness;
		Real m_shearStiffness;
		Matrix3r m_invRestMat;
		bool m_normalizeStretch;
		bool m_normalizeShear;

		StrainTetConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2,
			const unsigned int particle3, const unsigned int particle4, 
			const Real stretchStiffness, const Real shearStiffness, 
			const bool normalizeStretch, const bool normalizeShear);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class ShapeMatchingConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_stiffness;
		Vector3r m_restCm;
		Real *m_w;
		Vector3r *m_x0;
		Vector3r *m_x;
		Vector3r *m_corr;
		unsigned int *m_numClusters;

		ShapeMatchingConstraint(const unsigned int numberOfParticles) : Constraint(numberOfParticles)
		{
			m_x = new Vector3r[numberOfParticles];
			m_x0 = new Vector3r[numberOfParticles];
			m_corr = new Vector3r[numberOfParticles];
			m_w = new Real[numberOfParticles];
			m_numClusters = new unsigned int[numberOfParticles];
		}
		virtual ~ShapeMatchingConstraint() 
		{ 
			delete[] m_x; 
			delete[] m_x0;
			delete[] m_corr;
			delete[] m_w;
			delete[] m_numClusters;
		}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particleIndices[], const unsigned int numClusters[], const Real stiffness);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class RigidBodyContactConstraint 
	{
	public:
		static int TYPE_ID;
		/** indices of the linked bodies */
		std::array<unsigned int, 2> m_bodies;
		Real m_stiffness; 
		Real m_frictionCoeff;
		Real m_sum_impulses;
		Eigen::Matrix<Real, 3, 5, Eigen::DontAlign> m_constraintInfo;

		RigidBodyContactConstraint() {}
		~RigidBodyContactConstraint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int rbIndex1, const unsigned int rbIndex2, 
			const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2, 
			const pxr::GfVec3f& normal, const Real dist, 
			const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff);
		virtual bool solveVelocityConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class ParticleRigidBodyContactConstraint
	{
	public:
		static int TYPE_ID;
		/** indices of the linked bodies */
		std::array<unsigned int, 2> m_bodies;
		Real m_stiffness;
		Real m_frictionCoeff;
		Real m_sum_impulses;
		Eigen::Matrix<Real, 3, 5, Eigen::DontAlign> m_constraintInfo;

		ParticleRigidBodyContactConstraint() {}
		~ParticleRigidBodyContactConstraint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int particleIndex, const unsigned int rbIndex,
			const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2,
			const pxr::GfVec3f& normal, const Real dist,
			const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff);
		virtual bool solveVelocityConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class ParticleTetContactConstraint
	{
	public:
		static int TYPE_ID;
		/** indices of the linked bodies */
		std::array<unsigned int, 2> m_bodies;
		unsigned int m_solidIndex;
		unsigned int m_tetIndex; 
		Vector3r m_bary;
		Real m_lambda;
		Real m_frictionCoeff;
		Eigen::Matrix<Real, 3, 3, Eigen::DontAlign> m_constraintInfo;
		Real m_invMasses[4];
		std::array<Vector3r, 4> m_x;
		std::array<Vector3r, 4> m_v;

		ParticleTetContactConstraint() { }
		~ParticleTetContactConstraint() {}
		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int particleIndex, const unsigned int solidIndex,
			const unsigned int tetindex, const pxr::GfVec3f& bary,
			const pxr::GfVec3f& cp1, const pxr::GfVec3f& cp2,
			const pxr::GfVec3f& normal, const Real dist,
			const Real frictionCoeff);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
		virtual bool solveVelocityConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class StretchShearConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restLength;
		Real m_shearingStiffness1;
		Real m_shearingStiffness2;
		Real m_stretchingStiffness;

		StretchShearConstraint() : Constraint(3) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int particle1, const unsigned int particle2, 
			const unsigned int quaternion1, const Real stretchingStiffness, 
			const Real shearingStiffness1, const Real shearingStiffness2);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class BendTwistConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Quaternionr m_restDarbouxVector;
		Real m_bendingStiffness1;
		Real m_bendingStiffness2;
		Real m_twistingStiffness;

		BendTwistConstraint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(PBDSolver& solver, const unsigned int quaternion1, 
			const unsigned int quaternion2, const Real twistingStiffness,
			const Real bendingStiffness1, const Real bendingStiffness2);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	class StretchBendingTwistingConstraint : public Constraint
	{
		using Matrix6r = Eigen::Matrix<Real, 6, 6, Eigen::DontAlign>;
		using Vector6r = Eigen::Matrix<Real, 6, 1, Eigen::DontAlign>;
	public:
		static int TYPE_ID;
		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_constraintInfo;

		Real m_averageRadius;
		Real m_averageSegmentLength;
		Vector3r m_restDarbouxVector;
		Vector3r m_stiffnessCoefficientK;
		Vector3r m_stretchCompliance;
		Vector3r m_bendingAndTorsionCompliance;
		Vector6r m_lambdaSum;		

		StretchBendingTwistingConstraint() : Constraint(2){}

		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver, const unsigned int segmentIndex1, const unsigned int segmentIndex2, const pxr::GfVec3f& pos,
			const Real averageRadius, const Real averageSegmentLength, Real youngsModulus, Real torsionModulus);
		virtual bool initConstraintBeforeProjection(PBDSolver& solver);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);
	};

	struct Node;
	struct Interval;
	class SimulationModel;
	using Vector6r = Eigen::Matrix<Real, 6, 1, Eigen::DontAlign>;

	class DirectPositionBasedSolverForStiffRodsConstraint : public Constraint
	{
		class RodSegmentImpl : public RodSegment
		{			
		public:
			RodSegmentImpl(PBDSolver& solver, unsigned int idx) :
				m_model(model), m_segmentIdx(idx) {};

			virtual bool isDynamic();
			virtual Real Mass();
			virtual const pxr::GfVec3f&  InertiaTensor();
			virtual const pxr::GfVec3f&  Position();
			virtual const Quaternionr & Rotation();

			SimulationModel &m_model;
			unsigned int m_segmentIdx;
		};

		class RodConstraintImpl : public RodConstraint
		{
		public:
			std::vector<unsigned int> m_segments;
			Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_constraintInfo;

			Real m_averageRadius;
			Real m_averageSegmentLength;
			Vector3r m_restDarbouxVector;
			Vector3r m_stiffnessCoefficientK;
			Vector3r m_stretchCompliance;
			Vector3r m_bendingAndTorsionCompliance;
			
			virtual unsigned int segmentIndex(unsigned int i){
				if (i < static_cast<unsigned int>(m_segments.size()))
					return m_segments[i];
				return 0u;
			};

			virtual Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> & getConstraintInfo(){ return m_constraintInfo; }
			virtual Real getAverageSegmentLength(){ return m_averageSegmentLength; }
			virtual pxr::GfVec3f& getRestDarbouxVector(){ return m_restDarbouxVector; }
			virtual pxr::GfVec3f& getStiffnessCoefficientK() { return m_stiffnessCoefficientK; };
			virtual pxr::GfVec3f&  getStretchCompliance(){ return m_stretchCompliance; }
			virtual pxr::GfVec3f&  getBendingAndTorsionCompliance(){ return m_bendingAndTorsionCompliance; }
		};

	public:
		static int TYPE_ID;

		DirectPositionBasedSolverForStiffRodsConstraint() :  Constraint(2),
			root(NULL), numberOfIntervals(0), intervals(NULL), forward(NULL), backward(NULL){}
		~DirectPositionBasedSolverForStiffRodsConstraint();

		virtual int &getTypeId() const { return TYPE_ID; }

		bool initConstraint(PBDSolver& solver,
			const std::vector<std::pair<unsigned int, unsigned int>> & constraintSegmentIndices,
			const std::vector<Vector3r> &constraintPositions,
			const std::vector<Real> &averageRadii,
			const std::vector<Real> &averageSegmentLengths,
			const std::vector<Real> &youngsModuli,
			const std::vector<Real> &torsionModuli);

		virtual bool initConstraintBeforeProjection(PBDSolver& solver);
		virtual bool updateConstraint(PBDSolver& solver);
		virtual bool solvePositionConstraint(PBDSolver& solver, const unsigned int iter);

	protected:
		
		/** root node */
		Node *root;
		/** intervals of constraints */
		Interval *intervals;
		/** number of intervals */
		int numberOfIntervals;		
		/** list to process nodes with increasing row index in the system matrix H (from the leaves to the root) */
		std::list <Node*> *forward;
		/** list to process nodes starting with the highest row index to row index zero in the matrix H (from the root to the leaves) */
		std::list <Node*> *backward;

		std::vector<RodConstraintImpl> m_Constraints;
		std::vector<RodConstraint*> m_rodConstraints;

		std::vector<RodSegmentImpl> m_Segments;
		std::vector<RodSegment*> m_rodSegments;

		std::vector<Vector6r> m_rightHandSide;
		std::vector<Vector6r> m_lambdaSums;
		std::vector<std::vector<Matrix3r>> m_bendingAndTorsionJacobians;
		std::vector<Vector3r> m_corr_x;
		std::vector<Quaternionr> m_corr_q;

		void deleteNodes();
	};
}

#endif