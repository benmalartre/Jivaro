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

  // set near and far back to their uncomputed defaults
  void _ResetClippingPlanes();
  
  // updates the camera's transform matrix, that is, the matrix that brings
  // the camera to the origin, with the camera view pointing down:
  // +Y if this is a Zup camera, or
  // -Z if this is a Yup camera .
  void _PushToCameraTransform();

  // updates parameters (center, rotTheta, etc.) from the camera transform.
  void _PullFromCameraTransform();

  pxr::GfVec2d _RangeOfBoxAlongRay(const pxr::GfRay& camRay, 
                                    const pxr::GfBBox3d& bbox, 
                                    bool debugClipping);

  // moves the camera by (deltaRight, deltaUp) in worldspace coordinates. 
  // this is similar to a camera Truck/Pedestal.
  void Truck(double deltaRight, double deltaUp);

  // rotates the camera around the current camera base (approx. the film
  // plane).  Both parameters are in degrees.
  // this moves the center point that we normally tumble around.
  // this is similar to a camera Pan/Tilt.
  void PanTilt(double dPan, double dTilt);

  // Specialized camera movement that moves it on the "horizontal" plane
  void Walk(double dForward, double dRight);

  // helpers conversion to ISPCCamera (embree)
  void GetFrom(float& x, float& y, float& z);
  void GetTo(float& x, float& y, float& z);
  void GetUp(float& x, float& y, float& z);
  double GetFov(){return _fov;};
        
private:
  pxr::GfCamera _camera;
  double _overrideNear;
  double _overrideFar;
  bool _cameraTransformDirty;
  double _fov;
  double _rotTheta;
  double _rotPhi;
  double _rotPsi;

  pxr::GfVec3d _center;
  pxr::GfMatrix4d _YZUpMatrix;
  pxr::GfMatrix4d _YZUpInvMatrix;
  
  double _dist;
  double _closestVisibleDist;
  double _lastFramedDist;
  double _lastFramedClosestDist;
  double _selSize;

  std::string _name;
};

AMN_NAMESPACE_CLOSE_SCOPE