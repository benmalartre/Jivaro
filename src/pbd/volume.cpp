
#include <stdio.h> 
#include "../pbd/volume.h"

PXR_NAMESPACE_OPEN_SCOPE


PBDVolume::PBDVolume(const unsigned int nVertices, const unsigned int nFaces, 
    pxr::GfVec3f * const vertices, const unsigned int* indices)
	: _nVertices(nVertices)
    , _nFaces(nFaces)
    , _indices(indices)
    , _faceNormals(nFaces)
    , _weights(nFaces)
{
    // compute center of mass
    _x = pxr::GfVec3f(0.f);
    for (unsigned int i(0); i < _nVertices; ++i)
        _x += vertices[i];
        _x /= (float)_nVertices;

        _vertices.resize(_nVertices);
        for (unsigned int i = 0; i < _nVertices; ++i)
        _vertices[i] = vertices[i] - _x;

        for (unsigned int i = 0; i < _nFaces; ++i)
        {
        const pxr::GfVec3f& a = _vertices[_indices[3 * i]];
        const pxr::GfVec3f& b = _vertices[_indices[3 * i + 1]];
        const pxr::GfVec3f& c = _vertices[_indices[3 * i + 2]];

        const pxr::GfVec3f d1 = b - a;
        const pxr::GfVec3f d2 = c - a;

        _faceNormals[i] = pxr::GfCross(d1, d2);
        if (pxr::GfIsClose(_faceNormals[i], pxr::GfVec3f(0.f), 1.e-10))) {
            _faceNormals[i] = pxr::GfVec3f(0.f);
        } else { 
            _faceNormals[i].Normalize();
        }

        _weights[i] = -pxr::GfDot(_faceNormals[i], a);
    }
}

void PBDVolume::ComputeInertiaTensor(float density)
{
    ComputeVolumeIntegrals();
    _volume = static_cast<float>(T0);

    _mass = static_cast<float>(density * T0);

    // compute center of mass
    _r[0] = static_cast<float>(T1[0] / T0);
    _r[1] = static_cast<float>(T1[1] / T0);
    _r[2] = static_cast<float>(T1[2] / T0);

    /* compute inertia tensor */
    _theta(0, 0) = static_cast<float>(density * (T2[1] + T2[2]));
    _theta(1, 1) = static_cast<float>(density * (T2[2] + T2[0]));
    _theta(2, 2) = static_cast<float>(density * (T2[0] + T2[1]));
    _theta(0, 1) = _theta(1, 0) = -density * static_cast<float>(TP[0]);
    _theta(1, 2) = _theta(2, 1) = -density * static_cast<float>(TP[1]);
    _theta(2, 0) = _theta(0, 2) = -density * static_cast<float>(TP[2]);

    /* translate inertia tensor to center of mass */
    _theta(0, 0) -= _mass * (_r[1]*_r[1] + _r[2]*_r[2]);
    _theta(1, 1) -= _mass * (_r[2]*_r[2] + _r[0]*_r[0]);
    _theta(2, 2) -= _mass * (_r[0]*_r[0] + _r[1]*_r[1]);
    _theta(0, 1) = _theta(1, 0) += _mass * _r[0] * _r[1];
    _theta(1, 2) = _theta(2, 1) += _mass * _r[1] * _r[2];
    _theta(2, 0) = _theta(0, 2) += _mass * _r[2] * _r[0];

	_r += _x;
}


void PBDVolume::ComputeProjectionIntegrals(unsigned int f)
{
    float a0, a1, da;
    float b0, b1, db;
    float a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
    float a1_2, a1_3, b1_2, b1_3;
    float C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
    float Cab, Kab, Caab, Kaab, Cabb, Kabb;

    P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

    for (int i = 0; i < 3; i++)
    {
		a0 = _vertices[_indices[3 * f + i]][A];
		b0 = _vertices[_indices[3 * f + i]][B];
		a1 = _vertices[_indices[3 * f + ((i + 1) % 3)]][A];
		b1 = _vertices[_indices[3 * f + ((i + 1) % 3)]][B];

        da = a1 - a0;
        db = b1 - b0;
        a0_2 = a0 * a0; a0_3 = a0_2 * a0; a0_4 = a0_3 * a0;
        b0_2 = b0 * b0; b0_3 = b0_2 * b0; b0_4 = b0_3 * b0;
        a1_2 = a1 * a1; a1_3 = a1_2 * a1;
        b1_2 = b1 * b1; b1_3 = b1_2 * b1;

        C1 = a1 + a0;
        Ca = a1*C1 + a0_2; Caa = a1*Ca + a0_3; Caaa = a1*Caa + a0_4;
        Cb = b1*(b1 + b0) + b0_2; Cbb = b1*Cb + b0_3; Cbbb = b1*Cbb + b0_4;
        Cab = 3 * a1_2 + 2 * a1*a0 + a0_2; Kab = a1_2 + 2 * a1*a0 + 3 * a0_2;
        Caab = a0*Cab + 4 * a1_3; Kaab = a1*Kab + 4 * a0_3;
        Cabb = 4 * b1_3 + 3 * b1_2*b0 + 2 * b1*b0_2 + b0_3;
        Kabb = b1_3 + 2 * b1_2*b0 + 3 * b1*b0_2 + 4 * b0_3;

        P1 += db*C1;
        Pa += db*Ca;
        Paa += db*Caa;
        Paaa += db*Caaa;
        Pb += da*Cb;
        Pbb += da*Cbb;
        Pbbb += da*Cbbb;
        Pab += db*(b1*Cab + b0*Kab);
        Paab += db*(b1*Caab + b0*Kaab);
        Pabb += da*(a1*Cabb + a0*Kabb);
    }

    P1 /= 2.0;
    Pa /= 6.0;
    Paa /= 12.0;
    Paaa /= 20.0;
    Pb /= -6.0;
    Pbb /= -12.0;
    Pbbb /= -20.0;
    Pab /= 24.0;
    Paab /= 60.0;
    Pabb /= -60.0;
}

void PBDVolume::ComputeFaceIntegrals(unsigned int f)
{
  float w;
  pxr::GfVec3f n;
  float k1, k2, k3, k4;

  ComputeProjectionIntegrals(f);

  w = _weights[f];
  n = _faceNormals[f];
  k1 = (n[C] == 0) ? 0 : 1 / n[C];
  k2 = k1 * k1; k3 = k2 * k1; k4 = k3 * k1;

  Fa = k1 * Pa;
  Fb = k1 * Pb;
  Fc = -k2 * (n[A]*Pa + n[B]*Pb + w*P1);

  Faa = k1 * Paa;
  Fbb = k1 * Pbb;
  Fcc = k3 * (pxr::GfPow(n[A], 2.f)*Paa + 2*n[A]*n[B]*Pab + pxr::GfPow(n[B], 2.f)*Pbb
     + w*(2*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faaa = k1 * Paaa;
  Fbbb = k1 * Pbbb;
  Fccc = -k4 * (pxr::GfPow(n[A], 3.f)*Paaa + 3*pxr::GfPow(n[A], 2.f)*n[B]*Paab 
       + 3*n[A]*pxr::GfPow(n[B], 2.f)*Pabb + pxr::GfPow(n[B], 3.f)*Pbbb
       + 3*w*(pxr::GfPow(n[A], 2.f)*Paa + 2*n[A]*n[B]*Pab + pxr::GfPow(n[B], 2.f)*Pbb)
       + w*w*(3*(n[A]*Pa + n[B]*Pb) + w*P1));

  Faab = k1 * Paab;
  Fbbc = -k2 * (n[A]*Pabb + n[B]*Pbbb + w*Pbb);
  Fcca = k3 * (pxr::GfPow(n[A], 2.f)*Paaa + 2*n[A]*n[B]*Paab + pxr::GfPow(n[B], 2.f)*Pabb
     + w*(2*(n[A]*Paa + n[B]*Pab) + w*Pa));
}

void PBDVolume::ComputeVolumeIntegrals()
{
    float nx, ny, nz;

    T0  = T1[0] = T1[1] = T1[2] 
        = T2[0] = T2[1] = T2[2] 
        = TP[0] = TP[1] = TP[2] = 0;

	for (unsigned int i = 0; i < _nFaces; ++i)
    {
        pxr::GfVec3f const& n = _faceNormals[i];
        nx = std::abs(n[0]);
        ny = std::abs(n[1]);
        nz = std::abs(n[2]);
        if (nx > ny && nx > nz) 
            C = 0;
        else 
            C = (ny > nz) ? 1 : 2;
        A = (C + 1) % 3;
        B = (A + 1) % 3;

        ComputeFaceIntegrals(i);

        T0 += n[0] * ((A == 0) ? Fa : ((B == 0) ? Fb : Fc));

        T1[A] += n[A] * Faa;
        T1[B] += n[B] * Fbb;
        T1[C] += n[C] * Fcc;
        T2[A] += n[A] * Faaa;
        T2[B] += n[B] * Fbbb;
        T2[C] += n[C] * Fccc;
        TP[A] += n[A] * Faab;
        TP[B] += n[B] * Fbbc;
        TP[C] += n[C] * Fcca;
    }

    T1[0] /= 2; T1[1] /= 2; T1[2] /= 2;
    T2[0] /= 3; T2[1] /= 3; T2[2] /= 3;
    TP[0] /= 2; TP[1] /= 2; TP[2] /= 2;
}