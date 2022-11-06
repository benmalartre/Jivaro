#ifndef JVR_APPLICATION_CAMERA_H
#define JVR_APPLICATION_CAMERA_H

#pragma once

#include "../common.h"
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>

// translated from FreeCamera from UsdView

JVR_NAMESPACE_OPEN_SCOPE

#define DEFAULT_NEAR 1.0
#define DEFAULT_FAR 2000000.0

class Camera
{
public:
  Camera(const std::string& name, double fov=60.0);
  ~Camera(){};

  // get data
  pxr::GfVec3d GetPosition();
  pxr::GfVec3d GetViewPlaneNormal();
  pxr::GfVec3d GetRayDirection(float x, float y, float width, float height);

  // get matrices
  const pxr::GfMatrix4d GetTransform();
  const pxr::GfMatrix4d GetViewMatrix();
  const pxr::GfMatrix4d GetViewInverseMatrix();
  const pxr::GfMatrix4d GetProjectionMatrix();
  const std::vector<pxr::GfVec4f> GetClippingPlanes();

  // ray
  pxr::GfRay GetRay(float x, float y, float width, float height);

  // scene axis
  void SetZIsUp(bool isZUp);
  const pxr::GfMatrix4d& GetZUpMatrix() { return _zUpMatrix; };
  const pxr::GfMatrix4d& GetZUpInverseMatrix() { return _zUpInverseMatrix; };

  // frame selection
  void FrameSelection(const pxr::GfBBox3d &selBBox);

  // compute underlying GfCamera frustum
  pxr::GfFrustum _GetFrustum(){return _camera.GetFrustum();};

  // aspect ratio
  float GetAspectRatio() { return _camera.GetAspectRatio(); };

  // get spherical cooridnates
  void _GetSphericalCoordinates() {
    pxr::GfVec3d r = _pos - _lookat;
    double d = r.GetLength();
    _polar = (-acosf(r[1]/d)) * RADIANS_TO_DEGREES;
    _azimuth = (atanf(r[0]/r[2])) * RADIANS_TO_DEGREES;
  };

  // set near and far back to their uncomputed defaults
  void _ResetClippingPlanes();

  void ComputeFrustum(){_frustum = _camera.GetFrustum();};

  // lookat from position, lookat and up 
  void LookAt();

  // compute ray from normalized xy position
  // don't forget to update the camera frustum before calling this.
  pxr::GfRay ComputeRay(const pxr::GfVec2d& pos) const;

  // set position, lookat and up as once
  void Set( const pxr::GfVec3d& pos, 
            const pxr::GfVec3d& lookat, 
            const pxr::GfVec3d& up=pxr::GfVec3d::YAxis());

  // set position, lookat and up as once
  void SetWindow(int x, int y, int width, int height);

  // orbit around lookat point from mouse delta
  void Orbit(double x, double y);

  // dolly along view direction
  void Dolly(double x, double y);

  // walk parallel to camera plane
  void Walk(double x, double y);

  // getters
  pxr::GfCamera* Get(){return &_camera;};
  double GetFov(){return _fov;};
        
private:
  bool                  _orthographic;
  pxr::GfCamera         _camera;
  pxr::GfFrustum        _frustum;
  double                _near;
  double                _far;
  double                _fov;
  double                _dist;
  pxr::GfVec3d          _lookat;
  pxr::GfVec3d          _pos;
  pxr::GfVec3d          _up;
  double                _polar;
  double                _azimuth;
  bool                  _dirty;
  pxr::GfMatrix4d       _zUpMatrix;
  pxr::GfMatrix4d       _zUpInverseMatrix;

  std::string _name;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_CAMERA_H
