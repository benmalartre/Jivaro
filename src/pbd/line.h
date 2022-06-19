#ifndef JVR_PBD_LINE_H
#define JVR_PBD_LINE_H

#include <vector>
#include "../pbd/rigidbody.h"
#include "../pbd/particle.h"
#include "../pbd/constraint.h"


PXR_NAMESPACE_OPEN_SCOPE

class PBDLine
{
    struct OrientedEdge
    {
        OrientedEdge(){}
        OrientedEdge(unsigned int p0, unsigned int p1, unsigned int q0)
        {
            m_vert[0] = p0;
            m_vert[1] = p1;
            m_quat = q0;
        }
        unsigned int m_vert[2];
        unsigned int m_quat;
    };

public:
    typedef std::vector<OrientedEdge> Edges;

    PBDLine();
    virtual ~PBDLine();

protected:
    /** offset which must be added to get the correct index in the particles array */
    unsigned int _indexOffset;
    /** offset which must be added to get the correct index in the quaternions array */
    unsigned int _indexOffsetQuaternions;
    unsigned int _nPoints, _nQuaternions;
    Edges _edges;
    Real _restitutionCoeff;
    Real _frictionCoeff;

public:
    void UpdateConstraints();

    Edges& GetEdges();

    unsigned int GetIndexOffset() const;
    unsigned int GetIndexOffsetQuaternions() const;

    void InitMesh(const unsigned int nPoints, const unsigned int nQuaternions, 
        const unsigned int indexOffset, const unsigned int indexOffsetQuaternions, 
        unsigned int* indices, unsigned int* indicesQuaternions);

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
        return m_frictionCoeff;
    }

    FORCE_INLINE void SetFrictionCoeff(float val)
    {
        _frictionCoeff = val;
    }
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif