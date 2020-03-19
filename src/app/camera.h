#pragma once

#include "../default.h"

// translated from FreeCamera from UsdView

AMN_NAMESPACE_OPEN_SCOPE

#define DEFAULT_NEAR 1.0
#define DEFAULT_FAR 2000000.0

// experimentally on Nvidia M6000, if Far/Near is greater than this,
// then geometry in the back half of the volume will disappear
#define MAX_SAFE_Z_RESOLUTION 1.0e6

// experimentally on Nvidia M6000, if Far/Near is greater than this,
// then we will often see Z-fighting artifacts even for geometry that
// is close to camera, when rendering for picking
#define MAX_GOOD_Z_RESOLUTION 5.0e4

class AmnCamera
{
public:
  AmnCamera(const std::string& name, double fov=60.0);
  ~AmnCamera(){};

  // compute underlying GfCamera frustum
  pxr::GfFrustum _GetFrustum(){return _camera.GetFrustum();};

  // Computes and sets automatic clipping plane distances using the
  // camera's position and orientation, the bouding box
  // surrounding the stage, and the distance to the closest rendered
  // object in the central view of the camera (closestVisibleDist).
  // If either of the "override" clipping attributes are not None,
  // we use those instead'''
  void _SetClippingPlanes(const pxr::GfBBox3d& stageBBox);

  // set near and far back to their uncomputed defaults
  void _ResetClippingPlanes();
  
  // updates the camera's transform matrix, that is, the matrix that brings
  // the camera to the origin, with the camera view pointing down:
  // +Y if this is a Zup camera, or
  // -Z if this is a Yup camera .
  void _PushToCameraTransform();

  // updates parameters (center, rotTheta, etc.) from the camera transform.
  void _PullFromCameraTransform();

  pxr::GfRange1f _RangeOfBoxAlongRay(const pxr::GfRay& camRay, 
                                    const pxr::GfBBox3d& bbox, 
                                    bool debugClipping);

  void _SetClosestVisibleDistFromPoint(const pxr::GfVec3d& point);

  // makes sure the FreeCamera's computed parameters are up-to-date, and
  // returns the GfCamera object.  If 'autoClip' is True, then compute
  // "optimal" positions for the near/far clipping planes based on the
  // current closestVisibleDist, in order to maximize Z-buffer resolution"""
  pxr::GfCamera& ComputeGfCamera(const pxr::GfBBox3d& stageBBox, bool autoClip=false);

  // computes the ratio that converts pixel distance into world units.
  // It treats the pixel distances as if they were projected to a plane going
  // through the camera center.'''
  double _ComputePixelsToWorldFactor(double viewportHeight);
  
  // tumbles the camera around the center point by (dTheta, dPhi) degrees.
  void Tumble(double dTheta, double dPhi);
    
  // scales the distance of the freeCamera from it's center typically by
  // scaleFactor unless it puts the camera into a "stuck" state.
  void AdjustDistance(double scaleFactor);
        
  // moves the camera by (deltaRight, deltaUp) in worldspace coordinates. 
  // this is similar to a camera Truck/Pedestal.
  void Truck(double deltaRight, double deltaUp);

  // rotates the camera around the current camera base (approx. the film
  // plane).  Both parameters are in degrees.
  // this moves the center point that we normally tumble around.
  // this is similar to a camera Pan/Tilt.
  void PanTilt(double dPan, double dTilt);

  // specialized camera movement that moves it on the "horizontal" plane
  void Walk(double dForward, double dRight);

  void ComputeFrustum(){_frustum = _camera.GetFrustum();};
  // compute ray from normalized xy position
  // don't forget to update the camera frustum before calling this.
  pxr::GfRay ComputeRay(const pxr::GfVec2d& pos) const;

  // update transform from frustum matrix
  void UpdateTransform()
  {
    pxr::GfMatrix4d m = _frustum.ComputeViewMatrix();
    _camera.SetTransform(m);
  };

  // set camera position
  void SetPosition(const pxr::GfVec3d& pos){
    _frustum = _camera.GetFrustum();
    _frustum.SetPosition(pos);
    UpdateTransform();
  };

  // set lookat point
  void SetLookAt(const pxr::GfVec3d& lookAt) {
    _frustum = _camera.GetFrustum();
    pxr::GfVec3d delta = _frustum.GetPosition() - lookAt;
    pxr::GfRotation oldRot = _frustum.GetRotation();
    //_frustum.SetLookAt(pos);
    UpdateTransform();
  };

  void SetTransform(const pxr::GfMatrix4d& m){
    _camera.SetTransform(m);
  };

  void SetDistance(float distance) {
    _frustum = _camera.GetFrustum();
    _frustum.SetViewDistance(distance);
    UpdateTransform();
  };

  // get spherical cooridnates
  void _GetSphericalCoordinates() {
    /*
    pxr::GfVec3d r = 
    Protected r.v3f32
    Vector3::Sub(r,*Me\pos,*Me\lookat)
    Protected d.f = Vector3::Length(r)
    *Me\polar = -ACos(r\y/d)*#F32_RAD2DEG
    *Me\azimuth = ATan(r\x/r\z)*#F32_RAD2DEG
    */
  };
    
  // helpers conversion to ISPCCamera (embree)
  void GetFrom(float& x, float& y, float& z);
  void GetTo(float& x, float& y, float& z);
  void GetUp(float& x, float& y, float& z);
  double GetFov(){return _fov;};

  pxr::GfCamera* Get(){return &_camera;};
        
private:
  bool                  _orthographic;
  pxr::GfCamera         _camera;
  double                _overrideNear;
  double                _overrideFar;
  bool                  _cameraTransformDirty;
  double                _fov;
  double                _rotTheta;
  double                _rotPhi;
  double                _rotPsi;
  pxr::GfFrustum        _frustum;
  pxr::GfVec3d          _center;
  double                _polar;
  double                _azimuth;

  
  double _dist;
  double _closestVisibleDist;
  double _lastFramedDist;
  double _lastFramedClosestDist;
  double _selSize;

  std::string _name;
};

AMN_NAMESPACE_CLOSE_SCOPE