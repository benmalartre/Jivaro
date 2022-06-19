#ifndef JVR_PBD_TET_H
#define JVR_PBD_TET_H

#include "../geometry/mesh.h"
#include "../geometry/tet.h"
#include "../pbd/particle.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class PBDTet 
{
    public:
        PBDTet();
        virtual ~PBDTet();

        struct Attachment
        {
            unsigned int _index;
            unsigned int _triIndex;
            float _bary[3];
            float _dist;
            float _minError;
        };

        pxr::GfVec3f& GetInitialX() { return _initialX; }
        void SetInitialX(const pxr::GfVec3f& val) { _initialX = val; }
        pxr::GfMatrix3f& GetInitialR() { return _initialR; }
        void SetInitialR(const pxr::GfMatrix3f& val) { _initialR = val; }
        pxr::GfVec3f& GetInitialScale() { return _initialScale; }
        void SetInitialScale(const pxr::GfVec3f& val) { _initialScale = val; }

protected:
        unsigned int                    _indexOffset;
        /** Tet mesh of particles which represents the simulation model */
        // ParticleMesh m_particleMesh;			
        // SurfaceMesh m_surfaceMesh;
        // VertexData m_visVertices;
        // SurfaceMesh m_visMesh;
        float                           _restitutionCoeff;
        float                           _frictionCoeff;
        std::vector<Attachment>         _attachments;
        pxr::GfVec3f                    _initialX;
        pxr::GfVec3f                    _initialScale;
        pxr::GfMatrix3f                 _initialR;
        
        //void createSurfaceMesh();
        void SolveQuadraticForZero(const pxr::GfVec3f& F, const pxr::GfVec3f& Fu, 
            const pxr::GfVec3f& Fv, const pxr::GfVec3f& Fuu,
            const pxr::GfVec3f&Fuv, const pxr::GfVec3f& Fvv, 
            Real& u, Real& v);
        bool PointInTriangle(const pxr::GfVec3f& p0, const pxr::GfVec3f& p1, const pxr::GfVec3f& p2, 
            const pxr::GfVec3f& p, pxr::GfVec3f& inter, pxr::GfVec3f &bary);


    public:
        //SurfaceMesh &getSurfaceMesh();
        //VertexData &getVisVertices();
        //SurfaceMesh &getVisMesh();
        //ParticleMesh& getParticleMesh() { return m_particleMesh; }
        //const ParticleMesh& getParticleMesh() const { return m_particleMesh; }
        void CleanUp();

        unsigned int GetIndexOffset() const;

        void InitMesh(const unsigned int nPoints, const unsigned int nTets, 
            const unsigned int indexOffset, unsigned int* indices);
        void UpdateMeshNormals(const PBDParticle& p);

        /** Attach a visualization mesh to the surface of the body.
         * Important: The vertex normals have to be updated before 
         * calling this function by calling updateMeshNormals(). 
         */
        void AttachVisMesh(const PBDParticle& p);

        /** Update the visualization mesh of the body.
        * Important: The vertex normals have to be updated before
        * calling this function by calling updateMeshNormals().
        */
        void UpdateVisMesh(const PBDParticle& p);

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
        
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif