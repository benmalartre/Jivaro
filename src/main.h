
#include "default.h"
#include "application.h"
#include "device.h"
#include "mesh.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>

namespace embree {
  
  extern RTCScene g_scene;
  extern Vec3fa* face_colors; 
  extern Vec3fa* vertex_colors;
  extern RTCDevice g_device;
  extern bool g_changed;
  extern float g_debug;
 
  extern RTCScene g_scene;
  extern Vec3fa* face_colors;
  extern Vec3fa* vertex_colors;
  extern RTCDevice g_device ;

  void RecursePrim(const pxr::UsdPrim& prim);
  int TraverseAllRecursive(const pxr::UsdStageRefPtr stage);
  int TraverseAllPrimRange(const pxr::UsdStageRefPtr stage);
  void TraverseStage();

}
