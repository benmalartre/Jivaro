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
#include <pxr/base/gf/matrix4f.h>

#define AMN_EXPORT extern "C" 

#define EDGE_LEVEL 256.0f
#define ENABLE_SMOOTH_NORMALS 1
