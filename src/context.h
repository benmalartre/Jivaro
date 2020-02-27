#pragma once

#include "default.h"

namespace embree {

  struct Vertex{float x, y, z;};
  struct Triangle{int v0,v1,v2;};

  // scene data
  

  static Camera camera;
  static std::string rtcore("start_threads=1,set_affinity=1");

  struct UsdEmbreeContext {
    RTCDevice       _device;
    RTCScene        _scene;
    char*           _config;

    Vec3fa* face_colors = nullptr;
    Vec3fa* vertex_colors = nullptr;
    bool g_changed = false;
    float g_debug = 0.0f;

    void Initialize();
    void Term();
    void Clean();

    unsigned AddCube (RTCScene scene_i);
    unsigned AddGroundPlane (RTCScene scene_i);
  };

} // namespace embree
