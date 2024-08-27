#include "camera.h"
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/range2d.h>

#include <pxr/imaging/cameraUtil/conformWindow.h>

JVR_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
Camera::Camera(const std::string& name, double fov) :
  _name(name),
  _fov(fov),
  _camera(pxr::GfCamera()),
  _orthographic(false),
  _near(DEFAULT_NEAR),
  _far(DEFAULT_FAR),
  _dirty(true),
  _pos(pxr::GfVec3d(0,0,5)),
  _lookat(pxr::GfVec3d(0,0,0)),
  _up(pxr::GfVec3d(0,1,0))
{
  _camera.SetPerspectiveFromAspectRatioAndFieldOfView(1.0, _fov, 
    pxr::GfCamera::FOVVertical);
  _camera.SetFocusDistance((_pos - _lookat).GetLength());
  _camera.SetClippingRange(pxr::GfRange1f(_near, _far));
  SetZIsUp(false);
}

void
Camera::SetZIsUp(bool isZUp)
{
  _zUpMatrix = pxr::GfMatrix4d().SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), isZUp ? 90 : 0));
  _zUpInverseMatrix = _zUpMatrix.GetInverse();
}

pxr::GfVec3d
Camera::GetPosition()
{
  return _pos;
}

pxr::GfRay
Camera::GetRay(float x, float y, float width, float height)
{
  return pxr::GfRay(
    _zUpMatrix.Transform(_pos),
    GetRayDirection(x, y, width, height));
}

pxr::GfVec3d 
Camera::GetRayDirection(float x, float y, float width, float height)
{
  // normalize device coordinates
  double nx = (2.0 * x) / width - 1.0;
  double ny = 1.0 - (2.0 * y) /height;
  double nz = 1.0;
  pxr::GfVec3d rayNds(nx, ny, nz);

  // homogenous clip coordinates
  pxr::GfVec3d rayClip(rayNds[0], rayNds[1], -1.f);

  // eye (camera) coordinates
  pxr::GfMatrix4d invProj = GetProjectionMatrix().GetInverse();
  pxr::GfVec3d rayEye = invProj.Transform(rayClip);
  rayEye[2] = -1.0;

  // world coordinates
  pxr::GfMatrix4d invView = GetViewInverseMatrix();
  pxr::GfVec3d rayWorld = invView.TransformDir(rayEye);

  return rayWorld.GetNormalized();
}

pxr::GfVec3d 
Camera::GetViewPlaneNormal()
{
  return _zUpMatrix.TransformDir((_lookat - _pos)).GetNormalized();
}

const pxr::GfMatrix4d Camera::GetTransform()
{
  return _camera.GetTransform();
}

const pxr::GfMatrix4d Camera::GetViewMatrix()
{
  return _camera.GetFrustum().ComputeViewMatrix();
}

const pxr::GfMatrix4d Camera::GetViewInverseMatrix()
{
  return _camera.GetFrustum().ComputeViewMatrix().GetInverse();
}

const pxr::GfMatrix4d Camera::GetProjectionMatrix()
{
  return _camera.GetFrustum().ComputeProjectionMatrix();
}

const std::vector<pxr::GfVec4f> Camera::GetClippingPlanes()
{
  return _camera.GetClippingPlanes();
}

void Camera::FrameSelection(const pxr::GfBBox3d &selBBox)
{
  pxr::GfBBox3d zUpBBox(selBBox.GetRange(), _zUpInverseMatrix);
  pxr::GfVec3d center = zUpBBox.ComputeCentroid();
  pxr::GfRange3d selRange = zUpBBox.ComputeAlignedRange();
  pxr::GfVec3d rangeSize = selRange.GetSize();
  
  float frameFit = 1.1f;
  float selSize = rangeSize[0];
  if (rangeSize[1] > selSize)selSize = rangeSize[1];
  if (rangeSize[2] > selSize)selSize = rangeSize[2];
 
  if (_orthographic) {
    _fov = selSize * frameFit;
    _dist = selSize + _near;
  }
  else {
    double halfFov = _fov * 0.5;
    double lengthToFit = selSize * frameFit;
    _dist = lengthToFit / std::tanf(pxr::GfDegreesToRadians(halfFov));

  }
  pxr::GfVec3d dir = (_pos - _lookat).GetNormalized();
  _lookat = center;
  _pos = center + dir * _dist;

  LookAt();
}

void Camera::_ResetClippingPlanes()
{
  // Set near and far back to their uncomputed defaults
  _near = DEFAULT_NEAR;
  _far = DEFAULT_FAR;
  _camera.SetClippingRange(pxr::GfRange1f(_near, _far)); 
}

void Camera::Set(const pxr::GfVec3d& pos, 
                    const pxr::GfVec3d& lookat, 
                    const pxr::GfVec3d& up)
{
  _pos = pos;
  _lookat = lookat;
  _up = up;

  LookAt();
  _GetSphericalCoordinates();
}

void Camera::SetWindow(int x, int y, int width, int height)
{
  ComputeFrustum();
} 

void Camera::LookAt()
{
  pxr::GfMatrix4d m(1);
  m.SetLookAt(
    _zUpMatrix.TransformAffine(_pos), 
    _zUpMatrix.TransformAffine(_lookat), 
    _zUpMatrix.TransformAffine(_up)
  );
  m = m.GetInverse();
  pxr::GfVec3d zUpPosition = _zUpMatrix.TransformAffine(_pos);
  m[3][0] = zUpPosition[0];
  m[3][1] = zUpPosition[1];
  m[3][2] = zUpPosition[2];
  _camera.SetTransform(m);
  ComputeFrustum();
}

void Camera::Orbit(double dx, double dy)
{
  double dist = (_pos - _lookat).GetLength();
  _pos = pxr::GfVec3d(0,0, dist);

  _polar -= dy * 0.5f;
  _azimuth -= dx * 0.5f;

  pxr::GfRotation pR(pxr::GfVec3d::XAxis(), _polar);
  _pos = pR.TransformDir(_pos);

  pxr::GfRotation aR( pxr::GfVec3d::YAxis(), _azimuth);
  _pos = aR.TransformDir(_pos);

  _pos += _lookat;

  // flip up vector if necessary
  double checkAngle = std::fabs(std::fmod(_polar, 360));
  if(checkAngle < 90 || checkAngle >= 270)_up = pxr::GfVec3d(0,1,0);
  else _up = pxr::GfVec3d(0,-1,0);

  // lookat
  LookAt();
}

void Camera::Dolly(double dx, double dy)
{
   double delta = (dx + dy) * 2.0;
   pxr::GfVec3d interpolated = _pos * (1-delta) + _lookat * delta;
   _pos = interpolated;

  // update camera transform
  LookAt();
}

void Camera::Walk(double dx, double dy)
{
  pxr::GfVec3d delta = _pos - _lookat;
  double dist = delta.GetLength() * _fov * 0.5 * DEGREES_TO_RADIANS;

  delta[0] = -dx * dist;
  delta[1] = dy * dist;
  delta[2] = 0;

  pxr::GfMatrix4d view = _camera.GetTransform() * _zUpMatrix.GetInverse();
  pxr::GfRotation rot = view.ExtractRotation();
  delta = rot.TransformDir(delta);
  _pos += delta;
  _lookat += delta;

  // update camera transform
  LookAt();
}

pxr::GfRay Camera::ComputeRay(const pxr::GfVec2d& pos) const
{
  return _frustum.ComputeRay(pos);
}

JVR_NAMESPACE_CLOSE_SCOPE
