#pragma once

#include <sys/platform.h>
#include <sys/ref.h>
#include <math/math.h>
#include <math/vec3.h>
#include <math/affinespace.h>
#include <sstream>

namespace AMN
{
  /* camera settings */
  struct Camera
  {
    enum Handedness {
      LEFT_HANDED,
      RIGHT_HANDED
    };
    
    struct ISPCCamera
    {
    public:
      ISPCCamera (const embree::AffineSpace3fa& xfm)
      : xfm(xfm) {}

    public:
      embree::AffineSpace3fa xfm;
    };

  public:

    Camera ()
    : from(0.0001f,0.0001f,-3.0f), to(0,0,0), up(0,1,0), fov(90), handedness(RIGHT_HANDED) {}

    Camera (embree::Vec3fa& from, embree::Vec3fa& to, embree::Vec3fa& up, 
      float fov, Handedness handedness)
    : from(from), to(to), up(up), fov(fov), handedness(handedness) {}

    std::string str() const 
    {
      std::stringstream stream;
      stream.precision(10);
      stream << "--vp " << from.x    << " " << from.y    << " " << from.z    << " " 
             << "--vi " << to.x << " " << to.y << " " << to.z << " " 
             << "--vu " << up.x     << " " << up.y     << " " << up.z     << " " 
             << "--fov " << fov << " "
             << (handedness == LEFT_HANDED ? "--lefthanded" : "--righthanded");
      return stream.str();
    }
    
    embree::AffineSpace3fa camera2world () 
    {
      embree::AffineSpace3fa local2world =
        embree::AffineSpace3fa::lookat(from, to, up);
      if (handedness == RIGHT_HANDED) 
        local2world.l.vx = -local2world.l.vx;
      return local2world;
    }
    embree::AffineSpace3fa world2camera () { return rcp(camera2world()); }
    embree::Vec3fa world2camera(const embree::Vec3fa& p) 
    { 
      return xfmPoint(world2camera(),p); 
    }
    embree::Vec3fa camera2world(const embree::Vec3fa& p) 
    { 
      return xfmPoint(camera2world(),p); 
    }

    ISPCCamera getISPCCamera (size_t width, size_t height, bool flip_y = false)
    {
      const float fovScale = 1.0f/tanf(embree::deg2rad(0.5f*fov));
      const embree::AffineSpace3fa local2world = camera2world();
      embree::Vec3fa vx = local2world.l.vx;
      embree::Vec3fa vy = -local2world.l.vy;
      embree::Vec3fa vz = -0.5f*width*local2world.l.vx + 
        0.5f*height*local2world.l.vy + 
        0.5f*height*fovScale*local2world.l.vz;
      embree::Vec3fa p =  local2world.p;
      if (flip_y) {
        vz = vz+float(height)*vy;
        vy = -vy;
      }
      return ISPCCamera(embree::AffineSpace3fa(vx,vy,vz,p));
    }

    void move (float dx, float dy, float dz)
    {
      embree::AffineSpace3fa xfm = camera2world();
      embree::Vec3fa ds = embree::xfmVector(xfm,embree::Vec3fa(dx,dy,dz));
      from += ds;
      to   += ds;
    }

    void rotate (float dtheta, float dphi)
    {
      if (handedness == RIGHT_HANDED) dtheta *= -1.0f;
      const embree::Vec3fa up1 = embree::normalize(up);
      embree::Vec3fa view1 = embree::normalize(to-from);
      view1 = 
        embree::xfmVector(embree::AffineSpace3fa::rotate(up1, dtheta), view1);
      const float phi = acosf(dot(view1, up1));
      const float dphi2 = 
        phi - embree::clamp(phi-dphi, 0.001f*float(embree::pi), 
          0.999f*float(embree::pi));
      view1 = xfmVector(
        embree::AffineSpace3fa::rotate(
          embree::cross(view1, up1), dphi2), view1);
      to = from + length(to-from) * view1;
    }

    void rotateOrbit (float dtheta, float dphi)
    {
      if (handedness == RIGHT_HANDED) dtheta *= -1.0f;
      const embree::Vec3fa up1 = embree::normalize(up);
      embree::Vec3fa view1 = embree::normalize(to-from);
      view1 = 
        embree::xfmVector(embree::AffineSpace3fa::rotate(up1, dtheta), view1);
      const float phi = acosf(embree::dot(view1, up1));
      const float dphi2 = 
        phi - embree::clamp(phi-dphi, 0.001f*float(embree::pi), 
          0.999f*float(embree::pi));
      view1 = embree::xfmVector(
        embree::AffineSpace3fa::rotate(
          embree::cross(view1, up1), dphi2), view1);
      from = to - embree::length(to-from) * view1;
    }

    void dolly (float ds)
    {
      float dollySpeed = 0.01f;
      float k = powf((1.0f-dollySpeed), ds);
      from += embree::length(to-from) * (1-k) * embree::normalize(to-from);
    }

  public:
    embree::Vec3fa from;   //!< position of camera
    embree::Vec3fa to;     //!< look at point
    embree::Vec3fa up;     //!< up vector
    float fov;     //!< field of view
    Handedness handedness;
  };

  typedef Camera::ISPCCamera ISPCCamera;

} // namespace AMN
