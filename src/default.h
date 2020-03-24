#pragma once

// opengl
#include <GL/gl3w.h>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

// embree
#include <embree3/rtcore.h>
#include <sys/platform.h>
#include <sys/sysinfo.h>
#include <sys/ref.h>
#include <sys/vector.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/bbox.h>
#include <math/lbbox.h>
#include <math/affinespace.h>
#include <sys/filename.h>
#include <sys/string.h>
#include <lexers/tokenstream.h>
#include <lexers/streamfilters.h>
#include <lexers/parsestream.h>

// system
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <set>
#include <deque>
#include <chrono>
#include <ctime>
#include <functional>

// usd
#include <pxr/base/tf/hash.h>
#include <pxr/base/tf/hashmap.h>
#include <pxr/base/tf/stopwatch.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/quaternion.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/ray.h>


#define AMN_EXPORT extern "C" 

#define EDGE_LEVEL 256.0f
#define ENABLE_SMOOTH_NORMALS 1

#define AMN_MAJOR_VERSION 0
#define AMN_MINOR_VERSION 0
#define AMN_PATCH_VERSION 0

#define AMN_VERSION 000

#define AMN_USE_NAMESPACES 1

#if AMN_USE_NAMESPACES

  #define AMN_NS AMN
  #define AMN_INTERNAL_NS amnInternal_v0_00__reserved__
  #define AMN_NS_GLOBAL ::AMN_NS

  namespace AMN_INTERNAL_NS { }

  // The root level namespace for all source in the USD distribution.
  namespace AMN_NS {
      using namespace AMN_INTERNAL_NS;
  }

  #define AMN_NAMESPACE_OPEN_SCOPE   namespace AMN_INTERNAL_NS {
  #define AMN_NAMESPACE_CLOSE_SCOPE  }  
  #define AMN_NAMESPACE_USING_DIRECTIVE using namespace AMN_NS;

#else

  #define AMN_NS 
  #define AMN_NS_GLOBAL 
  #define AMN_NAMESPACE_OPEN_SCOPE   
  #define AMN_NAMESPACE_CLOSE_SCOPE 
  #define AMN_NAMESPACE_USING_DIRECTIVE

#endif // AMN_USE_NAMESPACES

#define DEGREES_TO_RADIANS 0.0174532925f
#define RADIANS_TO_DEGREES 57.2957795f

// conversion pxr/embree - embree/pxr  
inline embree::Vec3fa 
pxr2embree(const pxr::GfVec3f& v){return embree::Vec3fa(v[0], v[1], v[2]);};
inline embree::Vec3fa 
pxr2embree(const pxr::GfVec3d& v){return embree::Vec3fa((float)v[0], (float)v[1], (float)v[2]);};
inline embree::Vec2fa
pxr2embree(const pxr::GfVec2f& v){return embree::Vec2fa(v[0], v[1]);};
inline embree::Vec2fa 
pxr2embree(const pxr::GfVec2d& v){return embree::Vec2fa((float)v[0], (float)v[1]);};

inline pxr::GfVec3f
embree2pxr(const embree::Vec3fa & v){return pxr::GfVec3f(v.x, v.y, v.z);};
inline pxr::GfVec2f
embree2pxr(const embree::Vec2fa & v){return pxr::GfVec2f(v.x, v.y);};

