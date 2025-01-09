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
  GfVec3d GetPosition();
  GfVec3d GetViewPlaneNormal();
  GfVec3d GetRayDirection(float x, float y, float width, float height);

  // get matrices
  const GfMatrix4d GetTransform();
  const GfMatrix4d GetViewMatrix();
  const GfMatrix4d GetViewInverseMatrix();
  const GfMatrix4d GetProjectionMatrix();
  const std::vector<GfVec4f> GetClippingPlanes();

  // ray
  GfRay GetRay(float x, float y, float width, float height);

  // scene axis
  void SetZIsUp(bool isZUp);
  const GfMatrix4d& GetZUpMatrix() { return _zUpMatrix; };
  const GfMatrix4d& GetZUpInverseMatrix() { return _zUpInverseMatrix; };

  // frame selection
  void FrameSelection(const GfBBox3d &selBBox);

  // compute underlying GfCamera frustum
  GfFrustum _GetFrustum(){return _camera.GetFrustum();};

  // aspect ratio
  float GetAspectRatio() { return _camera.GetAspectRatio(); };

  // get spherical cooridnates
  void _GetSphericalCoordinates() {
    GfVec3d r = _pos - _lookat;
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
  GfRay ComputeRay(const GfVec2d& pos) const;

  // set position, lookat and up as once
  void Set( const GfVec3d& pos, 
            const GfVec3d& lookat, 
            const GfVec3d& up=GfVec3d::YAxis());

  // set position, lookat and up as once
  void SetWindow(int x, int y, int width, int height);

  // orbit around lookat point from mouse delta
  void Orbit(double x, double y);

  // dolly along view direction
  void Dolly(double x, double y);

  // walk parallel to camera plane
  void Walk(double x, double y);

  // getters
  GfCamera* Get(){return &_camera;};
  double GetFov(){return _fov;};
        
private:
  bool                  _orthographic;
  GfCamera         _camera;
  GfFrustum        _frustum;
  double                _near;
  double                _far;
  double                _fov;
  double                _dist;
  GfVec3d          _lookat;
  GfVec3d          _pos;
  GfVec3d          _up;
  double                _polar;
  double                _azimuth;
  bool                  _dirty;
  GfMatrix4d       _zUpMatrix;
  GfMatrix4d       _zUpInverseMatrix;

  std::string _name;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_CAMERA_H
