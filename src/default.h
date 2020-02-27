#pragma once

#include <GL/gl3w.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

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
