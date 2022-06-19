#ifndef JVR_PBD_TRIANGLE_H
#define JVR_PBD_TRIANGLE_H

#include <vector>
#include "../pbd/rigidbody.h"
#include "../pbd/particle.h"
#include "../pbd/constraint.h"

PXR_NAMESPACE_OPEN_SCOPE

	class PBDTriangle
	{
		public:
			PBDTriangle();
			virtual ~PBDTriangle();

		protected:
			/** offset which must be added to get the correct index in the particles array */
			unsigned int _indexOffset;
			/** Face mesh of particles which represents the simulation model */
			Mesh* _mesh;
			float _restitutionCoeff;
			float _frictionCoeff;

		public:
			Mesh* GetMesh() { return _mesh; }
			const Mesh* GetMesh() const { return _mesh; }

			void CleanUp();

			unsigned int GetIndexOffset() const; 

			void Init(const Mesh* mesh);
			void Update(const PBDParticle& p);

			FORCE_INLINE float GetRestitutionCoeff() const
			{
				return _restitutionCoeff;
			}

			FORCE_INLINE void SetRestitutionCoeff(float val)
			{
				_restitutionCoeff = val;
			}

			FORCE_INLINE float getFrictionCoeff() const
			{
				return _frictionCoeff;
			}

			FORCE_INLINE void setFrictionCoeff(float val)
			{
				_frictionCoeff = val;
			}
	};
}

#endif