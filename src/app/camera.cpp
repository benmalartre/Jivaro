#include "camera.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
AmnCamera::AmnCamera(
  const std::string& name, 
  double fov):
_name(name),
_fov(fov)
{
  _camera.SetPerspectiveFromAspectRatioAndFieldOfView(
            1.0, fov, pxr::GfCamera::FOVVertical);
  _ResetClippingPlanes();
  _center = pxr::GfVec3d(5,10,32);
  _dist = 1000.0;
  _camera.SetFocusDistance(_dist);
}

void AmnCamera::_PushToCameraTransform()
{
  if(!_cameraTransformDirty) return;
  _camera.SetTransform(
      pxr::GfMatrix4d().SetTranslate(pxr::GfVec3d::ZAxis() * _dist) *
      pxr::GfMatrix4d(1.0).SetRotate(
        pxr::GfRotation(pxr::GfVec3d::ZAxis(), -_rotPsi)
      ) *
      pxr::GfMatrix4d(1.0).SetRotate(
        pxr::GfRotation(pxr::GfVec3d::XAxis(), -_rotPhi)
      ) *
      pxr::GfMatrix4d(1.0).SetRotate(
        pxr::GfRotation(pxr::GfVec3d::YAxis(), -_rotTheta)
      ) *
      pxr::GfMatrix4d().SetTranslate(_center));
  _camera.SetFocusDistance(_dist);
  _cameraTransformDirty = false;
}

void AmnCamera::_PullFromCameraTransform()
{
  // reads the transform set on the camera and updates all the other
  // parameters.  This is the inverse of _pushToCameraTransform
  pxr::GfMatrix4d camTransform = _camera.GetTransform();
  float dist = _camera.GetFocusDistance();
  pxr::GfFrustum frustum = _camera.GetFrustum();
  pxr::GfVec3d camPos = frustum.GetPosition();
  pxr::GfVec3d camAxis = frustum.ComputeViewDirection();

  // compute translational parts
  _dist = dist;
  _selSize = _dist / 10.0;
  _center = camPos + _dist * camAxis;

  // compute rotational part
  pxr::GfMatrix4d transform = camTransform;
  transform.Orthonormalize();
  pxr::GfRotation rotation = transform.ExtractRotation();

  // decompose and set angles
  pxr::GfVec3d angles = 
    -rotation.Decompose(
      pxr::GfVec3d::YAxis(),
      pxr::GfVec3d::XAxis(),
      pxr::GfVec3d::ZAxis()
    );
  _rotTheta = angles[0];
  _rotPhi = angles[1];
  _rotPsi = angles[2];

  _cameraTransformDirty = true;
}

pxr::GfVec2d 
AmnCamera::_RangeOfBoxAlongRay(
  const pxr::GfRay& camRay, 
  const pxr::GfBBox3d& bbox, 
  bool debugClipping
)
{
  double maxDist = -std::numeric_limits<double>::max();
  double minDist = std::numeric_limits<double>::max();
  pxr::GfRange3d boxRange = bbox.GetRange();
  pxr::GfMatrix4d boxXform = bbox.GetMatrix();
  for(int i=0; i < 8; ++i)
  {
    // for each corner of the bounding box, transform to world
    // space and project
    pxr::GfVec3d point = boxXform.Transform(boxRange.GetCorner(i));
    double pointDist = camRay.FindClosestPoint(point)[1];

    // find the projection of that point of the camera ray
    // and find the farthest and closest point.
    if(pointDist > maxDist)
      maxDist = pointDist;
    if(pointDist < minDist)
        minDist = pointDist;
  }
      

  if(debugClipping)
      std::cout << "Projected bounds near/far: "<< minDist << ", " << maxDist << std::endl;

  // if part of the bbox is behind the ray origin (i.e. camera),
  // we clamp minDist to be positive.  Otherwise, reduce minDist by a bit
  // so that geometry at exactly the edge of the bounds won't be clipped -
  // do the same for maxDist, also!
  if(minDist < DEFAULT_NEAR) minDist = DEFAULT_NEAR;
  else minDist *= 0.99;
  maxDist *= 1.01;

  if(debugClipping)
      std::cout << "Contracted bounds near/far: " << minDist << ", " << maxDist << std::endl;

  return pxr::GfVec2d(minDist, maxDist);
}

void AmnCamera::_ResetClippingPlanes()
{
  // Set near and far back to their uncomputed defaults
  float near = _overrideNear > 0.0 ? _overrideNear : DEFAULT_NEAR;
  float far  = _overrideFar > 0.0 ? _overrideFar : DEFAULT_FAR;
  _camera.SetClippingRange(pxr::GfRange1f(near, far)); 
}

void AmnCamera::Truck(double deltaRight, double deltaUp)
{
  // need to update the camera transform before we access the frustum
  _PushToCameraTransform();
  pxr::GfFrustum frustum = _camera.GetFrustum();
  pxr::GfVec3d camUp = frustum.ComputeUpVector();
  pxr::GfVec3d camRight = pxr::GfCross(frustum.ComputeViewDirection(), camUp);
  _center += (deltaRight * camRight + deltaUp * camUp);
  _cameraTransformDirty = true;
  _PullFromCameraTransform();
  //self.signalFrustumChanged.emit()
}

void AmnCamera::PanTilt(double dPan, double dTilt)
{
  _camera.SetTransform(
    pxr::GfMatrix4d(1.0).SetRotate(pxr::GfRotation(pxr::GfVec3d::XAxis(), dTilt)) *
    pxr::GfMatrix4d(1.0).SetRotate(pxr::GfRotation(pxr::GfVec3d::YAxis(), dPan)) *
    _camera.GetTransform()
  );
  _PullFromCameraTransform();

  // When we Pan/Tilt, we don't want to roll the camera so we just zero it
  // out here.
  _rotPsi = 0.0;
  _cameraTransformDirty = true;
  //self.signalFrustumChanged.emit()
}
  
void AmnCamera::Walk(double dForward, double dRight)
{
  _PushToCameraTransform();
  pxr::GfFrustum frustum = _camera.GetFrustum();
  pxr::GfVec3d camUp = frustum.ComputeUpVector().GetNormalized();
  pxr::GfVec3d camForward = frustum.ComputeViewDirection().GetNormalized();
  pxr::GfVec3d camRight = pxr::GfCross(camForward, camUp);
  pxr::GfVec3d delta = dForward * camForward + dRight * camRight;
  _center += delta;
  _cameraTransformDirty = true;
  _PullFromCameraTransform();
  //self.signalFrustumChanged.emit()
}

// helpers conversion to ISPCCamera (embree)
void AmnCamera::GetFrom(float& x, float& y, float& z)
{
  pxr::GfVec3d pos = _camera.GetFrustum().GetPosition();
  x = (float)pos[0];
  y = (float)pos[1];
  z = (float)pos[2];
}

void AmnCamera::GetTo(float& x, float& y, float& z)
{
  pxr::GfVec3d to = _camera.GetFrustum().ComputeLookAtPoint();
  x = (float)to[0];
  y = (float)to[1];
  z = (float)to[2];
}

void AmnCamera::GetUp(float& x, float& y, float& z)
{
  pxr::GfVec3d up = _camera.GetFrustum().ComputeUpVector();
  x = (float)up[0];
  y = (float)up[1];
  z = (float)up[2];
}

AMN_NAMESPACE_CLOSE_SCOPE
