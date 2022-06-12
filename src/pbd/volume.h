
#ifndef JVR_PBD_VOLUME_H
#define JVR_PBD_VOLUME_H

#include <vector>
#include <string>
#include <iostream>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix3f.h>

PXR_NAMESPACE_OPEN_SCOPE

class PBDVolume
{

private:
    int A;
    int B;
    int C;

    // projection integrals 
    float P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;
    // face integrals 
    float Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;
    // volume integrals 
    float T0;
    float T1[3];
    float T2[3];
    float TP[3];

public:

    PBDVolume(const unsigned int nVertices, const unsigned int nFaces, pxr::GfVec3f* const vertices, const unsigned int* indices);

    void ComputeInertiaTensor(float density);

    float GetMass() const { return _mass; }
    float GetVolume() const { return _volume; }
    pxr::GfMatrix3f const& GetInertia() const { return _theta; }
    pxr::GfVec3f const& GetCenterOfMass() const { return _r; }

private:

    void ComputeVolumeIntegrals();
    void ComputeFaceIntegrals(unsigned int i);

    /** Compute various integrations over projection of face.
    */
    void ComputeProjectionIntegrals(unsigned int i);


    std::vector<pxr::GfVec3f>   _faceNormals;
    std::vector<float>          _weights;
    unsigned int                _nVertices;
    unsigned int                _nFaces;
    std::vector<pxr::GfVec3f>   _vertices;
    const unsigned int*         _indices;

    float                       _mass, _volume;
    pxr::GfVec3f                _r;
    pxr::GfVec3f                _x;
    pxr::GfMatrix3f             _theta;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif 