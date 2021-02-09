#pragma once

#include "../common.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>


AMN_NAMESPACE_OPEN_SCOPE

struct Proxy {
  pxr::UsdPrim  prim;
  Geometry*     geometry;

  Proxy(const pxr::UsdPrim& prim);
  ~Proxy();
};

class Scene {
public:
  Scene();
  Scene(pxr::UsdStageRefPtr& stage);
  ~Scene();

private:
  pxr::UsdStageRefPtr _stage;
  std::vector<Proxy>  _proxys;
};


AMN_NAMESPACE_CLOSE_SCOPE