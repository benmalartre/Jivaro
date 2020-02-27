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
#include <set>
#include <deque>
#include <chrono>
#include <ctime>
#include <functional>


#define AMN_EXPORT extern "C" 

struct Vertex{float x, y, z;};
struct Face{int a, b, c;};
struct Triangle{int v0,v1,v2;};
