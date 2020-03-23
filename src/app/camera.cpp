#include "camera.h"
#include "../utils/utils.h"
#include <pxr/imaging/cameraUtil/conformWindow.h>

AMN_NAMESPACE_OPEN_SCOPE

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
  _camera.SetPerspectiveFromAspectRatioAndFieldOfView(
            1.0, _fov, pxr::GfCamera::FOVVertical);
  _camera.SetFocusDistance((_pos - _lookat).GetLength());
  _camera.SetClippingRange(pxr::GfRange1f(_near, _far));
  //_ResetClippingPlanes();
}

/*
void Camera::_PushToCameraTransform()
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
    pxr::GfMatrix4d().SetTranslate(_center)
  );
  _camera.SetFocusDistance(_dist);
  _cameraTransformDirty = false;
}

void Camera::_PullFromCameraTransform()
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

pxr::GfRange1f 
Camera::_RangeOfBoxAlongRay(
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

  return pxr::GfRange1f(minDist, maxDist);
}

pxr::GfCamera& Camera::ComputeGfCamera(const pxr::GfBBox3d&  stageBBox, bool autoClip)
{
  _PushToCameraTransform();
  if(autoClip)
      _SetClippingPlanes(stageBBox);
  else
      _ResetClippingPlanes();
  return _camera;
}

void Camera::_SetClippingPlanes(const pxr::GfBBox3d& stageBBox)
{
  _camera.SetClippingRange(pxr::GfRange1f(DEFAULT_NEAR, DEFAULT_FAR));
  return;

  //debugClipping = Tf.Debug.IsDebugSymbolNameEnabled(DEBUG_CLIPPING)
  double computedNear;
  double computedFar;
  
  // If the scene bounding box is empty, or we are fully on manual
  //(override, then just initialize to defaults.
  if(stageBBox.GetRange().IsEmpty() || (_overrideNear>0 && _overrideFar>0))
  {
    computedNear = DEFAULT_NEAR;
    computedFar = DEFAULT_FAR;
  }
      
  else
  {
    // The problem: We want to include in the camera frustum all the
    // geometry the viewer should be able to see, i.e. everything within
    // the inifinite frustum starting at distance epsilon from the
    // camera itself.  However, the further the imageable geometry is
    // from the near-clipping plane, the less depth precision we will
    // have to resolve nearly colinear/incident polygons (which we get
    // especially with any doubleSided geometry).  We can run into such
    // situations astonishingly easily with large sets when we are
    // focussing in on just a part of a set that spans 10^5 units or
    // more.
    //
    // Our solution: Begin by projecting the endpoints of the imageable
    // world's bounds onto the ray piercing the center of the camera
    // frustum, and take the near/far clipping distances from its
    // extent, clamping at a positive value for near.  To address the
    // z-buffer precision issue, we rely on someone having told us how
    // close the closest imageable geometry actually is to the camera,
    // by having called setClosestVisibleDistFromPoint(). This gives us
    // the most liberal near distance we can use and not clip the
    // geometry we are looking at.  We actually choose some fraction of
    // that distance instead, because we do not expect the someone to
    // recompute the closest point with every camera manipulation, as
    // it can be expensive (we do emit signalFrustumChanged to notify
    // them, however).  We only use this if the current range of the
    // bbox-based frustum will have precision issues.
    _frustum = _camera.GetFrustum();
    pxr::GfVec3d camPos = _frustum.GetPosition();

    std::cout << "_SET CLIPPING RANGE : POSITION " << camPos[0] << "," << camPos[1] << "," << camPos[2] << std::endl; 

    pxr::GfRay camRay(camPos, _frustum.ComputeViewDirection());
    pxr::GfRange1f computedNearFar = _RangeOfBoxAlongRay(camRay, stageBBox, false);
    computedNear = computedNearFar.GetMin();
    computedFar = computedNearFar.GetMax();

    double precisionNear = computedFar / MAX_GOOD_Z_RESOLUTION;

    if(_closestVisibleDist)
    {
      // Because of our concern about orbit/truck causing
      // clipping, make sure we don't go closer than half the
      // distance to the closest visible point
      double halfClose = _closestVisibleDist / 2.0;

      if(_closestVisibleDist < _lastFramedClosestDist)
      {
        // This can happen if we have zoomed in closer since
        // the last time setClosestVisibleDistFromPoint() was called.
        // Clamp to precisionNear, which gives a balance between
        // clipping as we zoom in, vs bad z-fighting as we zoom in.
        // See AdjustDistance() for comment about better solution.
        halfClose = std::max({precisionNear, halfClose, computedNear});

      }

      if(halfClose < computedNear)
      {
        // If there's stuff very very close to the camera, it
        // may have been clipped by computedNear.  Get it back!
        computedNear = halfClose;

      }
      else if(precisionNear > computedNear)
      {
        computedNear = std::min((precisionNear + halfClose) / 2.0, halfClose);
      }
    }
  }
  
  double near = _overrideNear > 0 ? _overrideNear : computedNear;
  double far  = _overrideFar > 0 ? _overrideFar : computedFar;

  // Make sure far is greater than near
  far = std::max(near+1, far);

  _camera.SetClippingRange(pxr::GfRange1f(near, far));
}       

void Camera::_ResetClippingPlanes()
{
  std::cout << "RESET CLIPPING PLANE ..." << std::endl;
  // Set near and far back to their uncomputed defaults
  float near = _overrideNear > 0.0 ? _overrideNear : DEFAULT_NEAR;
  float far  = _overrideFar > 0.0 ? _overrideFar : DEFAULT_FAR;
  _camera.SetClippingRange(pxr::GfRange1f(near, far)); 
}

void Camera::_SetClosestVisibleDistFromPoint(const pxr::GfVec3d& point)
{
  _frustum = _camera.GetFrustum();
  pxr::GfVec3d camPos = _frustum.GetPosition();
  pxr::GfRay camRay = pxr::GfRay(camPos, _frustum.ComputeViewDirection());
  _closestVisibleDist = camRay.FindClosestPoint(point)[1];
  _lastFramedDist = _dist;
  _lastFramedClosestDist = _closestVisibleDist;
}

double Camera::_ComputePixelsToWorldFactor(double viewportHeight)
{
  _PushToCameraTransform();
  if(_orthographic)
    return _fov / viewportHeight;
  else
  {
    double frustumHeight = _camera.GetFrustum().GetWindow().GetSize()[1];
    return frustumHeight * _dist / viewportHeight;
  }   
}

void Camera::Tumble( double dTheta, double dPhi)
{
  _frustum = _camera.GetFrustum();

  pxr::GfVec3f from, to;
  GetFrom(from[0], from[1], from[2]);
  GetTo(to[0], to[1], to[2]);

  _rotTheta += dTheta;
  _rotPhi += dPhi;
  _cameraTransformDirty = true;
  //_PushToCameraTransform();
  //self.signalFrustumChanged.emit()
}    

void  Camera::AdjustDistance(double scaleFactor)
{
  // When dist gets very small, you can get stuck and not be able to
  // zoom back out, if you just keep multiplying.  Switch to addition
  // in that case, choosing an incr that works for the scale of the
  // framed geometry.
  if(scaleFactor > 1 && _dist < 2)
  {
    double selBasedIncr = _selSize / 25.0;
    scaleFactor -= 1.0;
    _dist += (selBasedIncr < scaleFactor ? selBasedIncr : scaleFactor);
  }
  else _dist *= scaleFactor;

  // Make use of our knowledge that we are changing distance to camera
  // to also adjust _closestVisibleDist to keep it useful.  Make sure
  // not to recede farther than the last *computed* closeDist, since that
  // will generally cause unwanted clipping of close objects.
  // XXX:  This heuristic does a good job of preventing undesirable
  // clipping as we zoom in and out, but sacrifices the z-buffer
  // precision we worked hard to get.  If Hd/UsdImaging could cheaply
  // provide us with the closest-point from the last-rendered image,
  // we could use it safely here to update _closestVisibleDist much
  // more accurately than this calculation.
  if(_closestVisibleDist)
  {
    if(_dist > _lastFramedDist)
      _closestVisibleDist = _lastFramedClosestDist;
    else
      _closestVisibleDist = _lastFramedClosestDist - _lastFramedDist + _dist;
  }
  std::cout << "FUCKIN ADJUST DISTANCE FUCK!!! " << std::endl;
  _PushToCameraTransform();
}

void Camera::Truck(double deltaRight, double deltaUp)
{
  // need to update the camera transform before we access the frustum
  _PushToCameraTransform();
  _frustum = _camera.GetFrustum();
  pxr::GfVec3d camUp = _frustum.ComputeUpVector();
  pxr::GfVec3d camRight = pxr::GfCross(_frustum.ComputeViewDirection(), camUp);
  _center += (deltaRight * camRight + deltaUp * camUp);
  _cameraTransformDirty = true;
  _PullFromCameraTransform();
  //self.signalFrustumChanged.emit()
}

void Camera::PanTilt(double dPan, double dTilt)
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
  
void Camera::Walk(double dForward, double dRight)
{
  _PushToCameraTransform();
  _frustum = _camera.GetFrustum();
  pxr::GfVec3d camUp = _frustum.ComputeUpVector().GetNormalized();
  pxr::GfVec3d camForward = _frustum.ComputeViewDirection().GetNormalized();
  pxr::GfVec3d camRight = pxr::GfCross(camForward, camUp);
  pxr::GfVec3d delta = dForward * camForward + dRight * camRight;
  _center += delta;
  _cameraTransformDirty = true;
  _PullFromCameraTransform();
  //self.signalFrustumChanged.emit()
}
*/

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
  m.SetLookAt (_pos, _lookat, _up);
  _camera.SetTransform(m.GetInverse() * pxr::GfMatrix4d().SetTranslate(_pos));
  ComputeFrustum();

  /*
  pxr::GfFrustum frustum = _camera.GetFrustum();
  frustum.SetPosition(_pos);
  
  pxr::GfVec3d dir = (_pos - _lookat).GetNormalized();
  pxr::GfVec3d side = GfCross(dir, _up).GetNormalized();
  pxr::GfVec3d up = GfCross(side, dir).GetNormalized();

  pxr::GfMatrix3d rm(1);
  rm.SetRow(0, side);
  rm.SetRow(1, up);
  rm.SetRow(2, dir);
  pxr::GfRotation rot = rm.ExtractRotation();
  frustum.SetRotation(rot);

  _camera.SetTransform(frustum.ComputeViewMatrix());
  */
}

void Camera::Orbit(double dx, double dy)
{
  double dist = (_pos - _lookat).GetLength();
  _pos = pxr::GfVec3d(0,0, dist);

  _polar -= dy;
  _azimuth -= dx;

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

  pxr::GfMatrix4d view = _camera.GetTransform();
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

AMN_NAMESPACE_CLOSE_SCOPE
