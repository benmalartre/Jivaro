#include <iostream>
#include <map>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include "../geometry/shape.h"
#include "../geometry/intersection.h"
#include "../utils/shaders.h"
#include "../utils/glutils.h"

JVR_NAMESPACE_OPEN_SCOPE

ShapeProgramMap SHAPE_PROGRAM_MAP;

Shape* GRID_SHAPE = NULL;
Shape* BOX_SHAPE = NULL;
Shape* SPHERE_SHAPE = NULL;
Shape* ICOSPHERE_SHAPE = NULL;
Shape* DISC_SHAPE = NULL;
Shape* CYLINDER_SHAPE = NULL;
Shape* TUBE_SHAPE = NULL;
Shape* CONE_SHAPE = NULL;
Shape* CAPSULE_SHAPE = NULL;
Shape* TORUS_SHAPE = NULL;

void 
Shape::Component::SetFlag(short flag, bool value)
{
  if(value) {
    BITMASK_SET(flags, flag);
  } else {
    BITMASK_CLEAR(flags, flag);
  }
}

bool 
Shape::Component::GetFlag(short flag) const
{
  return BITMASK_CHECK(flags, flag);
}

void 
Shape::Component::SetBounds(const GfVec3f& xyz)
{
  bounds = GfRange3f(GfVec3f(0.f), xyz);
}

void
Shape::Component::SetMode(short m)
{
  mode = m;
}

void 
Shape::Component::SetBounds(const GfRange3f& value)
{
  bounds = value;
}

void 
Shape::Component::ComputeBounds(Shape* shape)
{
  bounds.SetEmpty();
  const std::vector<GfVec3f>& points = shape->GetPoints();
  for(size_t e = basePoints; e < (basePoints + numPoints); ++e) {
    bounds.UnionWith(points[e]);
  }
}

short 
Shape::Component::_IntersectGrid(const GfRay& ray, 
   const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4f matrix = (offsetMatrix * parentMatrix * m).GetInverse();
  GfRay localRay(ray);
  localRay.Transform(GfMatrix4d(matrix));

  const GfVec3f& bound = bounds.GetMax();
  GfRange3d range(
    GfVec3d(-bound[0] * 0.5f, -0.01f * scale, -bound[1] * 0.5f),
    GfVec3d(bound[0] * 0.5f, 0.01f * scale, bound[1] * 0.5f)
  );
  GfBBox3d bbox(range);

  double enterDistance, exitDistance;
  if (localRay.Intersect(bbox, &enterDistance, &exitDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short 
Shape::Component::_IntersectBox(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4f matrix = (offsetMatrix * parentMatrix * m).GetInverse();
  GfRay localRay(ray);
  localRay.Transform(GfMatrix4d(matrix));

  const GfVec3f offset = bounds.GetSize() * (1.0 - scale) * 0.5;
  GfRange3d range(
    GfVec3d(bounds.GetMin() - offset),
    GfVec3d(bounds.GetMax() + offset));
  GfBBox3d bbox(range);
  double enterDistance, exitDistance;
  if (localRay.Intersect(bbox, &enterDistance, &exitDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short 
Shape::Component::_IntersectSphere(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4f matrix = (offsetMatrix * parentMatrix * m).GetInverse();
  GfRay localRay(ray);
  localRay.Transform(GfMatrix4d(matrix));

  double enterDistance, exitDistance;
  if (localRay.Intersect(GfVec3d(0.0), bounds.GetMax()[0] * scale, &enterDistance, &exitDistance)) {
    *distance = enterDistance;
    return index;
  }
  return -1;
}

short 
Shape::Component::_IntersectDisc(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4d matrix((offsetMatrix * parentMatrix * m).GetInverse());
  GfRay localRay(ray);
  localRay.Transform(matrix);

  float radius = bounds.GetMax()[0];
  double enterDistance;
  if(IntersectDisc(localRay, radius * scale, &enterDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short
Shape::Component::_IntersectRing(const GfRay& ray,
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4d matrix((offsetMatrix * parentMatrix * m).GetInverse());
  GfRay localRay(ray);
  localRay.Transform(matrix);

  const GfVec3d& bound = bounds.GetMax();
  double enterDistance;
  if (IntersectRing(localRay, bound[0], bound[1] * scale, &enterDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short 
Shape::Component::_IntersectCylinder(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4d matrix((offsetMatrix * parentMatrix * m).GetInverse());
  GfRay localRay(ray);
  localRay.Transform(matrix);

  const GfVec3d& bound = bounds.GetMax();
  double enterDistance;
  if (IntersectCylinder(localRay, bound[0] * scale, bound[1], &enterDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short
Shape::Component::_IntersectTube(const GfRay& ray,
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4d matrix((offsetMatrix * parentMatrix * m).GetInverse());
  GfRay localRay(ray);
  localRay.Transform(matrix);

  const GfVec3d& bound = bounds.GetMax();
  double enterDistance;
  double innerRadius = bound[0] - (bound[0] * scale - bound[0]);
  if (IntersectTube(localRay, innerRadius, bound[1] * scale, bound[2], &enterDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short 
Shape::Component::_IntersectTorus(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  GfMatrix4f matrix = (offsetMatrix * parentMatrix * m).GetInverse();
  GfRay localRay(ray);
  localRay.Transform(GfMatrix4d(matrix));

  const GfVec3d& bound = bounds.GetMax();
  double enterDistance;
  if (IntersectTorusApprox(localRay, bound[0], bound[1] * scale, &enterDistance)) {
    *distance = enterDistance;
    return index;
  }
  return 0;
}

short 
Shape::Component::Intersect(const GfRay& ray, 
  const GfMatrix4f& m, double* distance, double scale)
{
  return _intersectImplementation ? 
    (this->*_intersectImplementation)(ray, m, distance, scale) : 0;
}

Shape::Shape(short mode, short usage) 
  : _mode(mode)
  , _usage(usage)
  ,_vao(0)
  , _vbo(0)
  , _eab(0)
  , _scale(1.1)
  , _uColor(0)
  , _uModel(0)
{
};

Shape::~Shape()
{
  if(_vbo) glDeleteBuffers(1,&_vbo);
  if(_eab) glDeleteBuffers(1,&_eab);
  if(_vao) glDeleteVertexArrays(1,&_vao); 
};

void
Shape::SetVisibility(int bits)
{
  for (int i = 0; i < _components.size(); ++i) {
    _components[i].SetFlag(Shape::VISIBLE, bits & 1 << i);
  }
}

void 
Shape::UpdateComponents(short hovered, short active, bool hideInactive)
{
  for(Shape::Component& component: _components) {
    if (component.GetFlag(Shape::MASK)) continue;
    if (hideInactive && component.index != active) 
      BITMASK_CLEAR(component.flags, VISIBLE);
    else
      BITMASK_SET(component.flags, VISIBLE);

    if(component.index == hovered)
      BITMASK_SET(component.flags, HOVERED);
    else 
      BITMASK_CLEAR(component.flags, HOVERED);

    if(component.index == active)
      BITMASK_SET(component.flags, SELECTED);
    else 
      BITMASK_CLEAR(component.flags, SELECTED);
  }
}

void
Shape::UpdateVisibility(const GfMatrix4f& m, const GfVec3f& dir)
{
  for(Shape::Component& component: _components) {
    GfVec3f axis = _GetComponentAxis(component, m);
    switch(component.index) {
      case 1:
      case 2:
      case 3:
        if(GfAbs(GfDot(dir, axis)) < 0.9f)
          BITMASK_SET(component.flags, VISIBLE);
        else
          BITMASK_CLEAR(component.flags, VISIBLE);
        break;
      case 4:
      case 5:
      case 6:
        if(GfAbs(GfDot(dir, axis)) > 0.15f)
         BITMASK_SET(component.flags, VISIBLE);
        else
          BITMASK_CLEAR(component.flags, VISIBLE);
        break;
      default:
        BITMASK_SET(component.flags, VISIBLE);
        break;
    }
  }
}

void 
Shape::AddComponent(const Component& component)
{
  _components.push_back(component);
}

void 
Shape::RemoveComponent(size_t idx)
{
  if (!_components.size() || idx >= _components.size())return;
  const Component& comp = _components[idx];
  size_t remainingPoints = _points.size() - (comp.basePoints + comp.numPoints);
  size_t remainingIndices = _indices.size() - (comp.baseIndices + comp.numIndices);
  memmove(
    &_points[comp.basePoints], 
    &_points[comp.basePoints + comp.numPoints], 
    remainingPoints * sizeof(GfVec3f)
  );
  memmove(
    &_indices[comp.baseIndices],
    &_indices[comp.baseIndices + comp.numIndices],
    remainingIndices * sizeof(int)
  );
  _indices.resize(_indices.size() - comp.numIndices);
  _points.resize(_points.size() - comp.numPoints);
  _components.erase(_components.begin() + idx);
}

void 
Shape::RemoveLastComponent()
{
  if (!_components.size()) return;
  const Component& comp = _components.back();
  _indices.resize(_indices.size() - comp.numIndices);
  _points.resize(_points.size() - comp.numPoints);
  _components.pop_back();
}

void 
Shape::_MakeCircle(std::vector<GfVec3f>& points, float radius, 
  size_t numPoints, const GfMatrix4f& m)
{
  float step = (360 / (float)numPoints) * DEGREES_TO_RADIANS;
  for(size_t k = 0; k < numPoints; ++k) {
    points.push_back(m.TransformAffine(
      GfVec3f(std::sinf(step*k) * radius, 0.f, std::cosf(step*k) * radius)));
  }
}

void 
Shape::_TransformPoints(size_t startIdx, size_t endIdx, 
  const GfMatrix4f& m)
{
  for(size_t k=startIdx; k < endIdx; ++k) {
    _points[k] = m.TransformAffine(_points[k]);
  }
}

GfVec3f
Shape::_GetComponentAxis(const Shape::Component& component, 
  const GfMatrix4f& m)
{
  const GfMatrix4f matrix = 
    component.offsetMatrix * component.parentMatrix * m;
  return matrix.TransformDir(GfVec3f(0.f, 1.f, 0.f));
}

short
Shape::Intersect(const GfRay& ray, const GfMatrix4f& model, const GfMatrix4f& view)
{
  double minDistance = DBL_MAX;
  short result = 0;
  short intersected;
  for(auto& component: _components) {
    if (component.GetFlag(Shape::VISIBLE) && component.GetFlag(Shape::PICKABLE)) {
      double distance = DBL_MAX;
      if (component.GetFlag(Shape::FLAT)) {
        intersected = component.Intersect(ray, view, &distance, _scale);
      } else {
        intersected = component.Intersect(ray, model, &distance, _scale);
      }
      if (distance < minDistance) {
        if (component.GetFlag(Shape::MASK)) {
          result = 0;
        } else {
          result = intersected;
        }
        minDistance = distance;
      }
    }
  }
  return result;
}

Shape::Component
Shape::AddGrid(short index, float width, float depth, size_t divX, 
  size_t divZ, const GfVec4f& color, const GfMatrix4f& m)
{
  // initialize data
  divX = divX < 2 ? 2 : divX;
  divZ = divZ < 2 ? 2 : divZ;

  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  float baseX = -width * 0.5f;
  float baseZ = -depth * 0.5f;
  float stepX = width / (divX - 1);
  float stepZ = depth / (divZ - 1);

  for(size_t z=0; z < divZ; ++z) {
    for(size_t x=0; x < divX; ++x) {
      _points.push_back( 
        GfVec3f(baseX + x * stepX, 0.f, baseZ + z * stepZ));
    }
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(size_t z=0; z < (divZ - 1); ++z) {
    for(size_t x=0; x < (divX - 1); ++x) {
      _indices.push_back(baseNumPoints + x + z * divX);
      _indices.push_back(baseNumPoints + x + (z + 1) * divX);
      _indices.push_back(baseNumPoints + (x + 1) + z * divX);

      _indices.push_back(baseNumPoints + (x + 1) + z * divX);
      _indices.push_back(baseNumPoints + x + (z + 1) * divX);
      _indices.push_back(baseNumPoints + (x + 1) + (z + 1) * divX);
    }
  }

  // make component
  Shape::Component component(
    Shape::GRID,
    index, 
    baseNumPoints, 
    (_points.size() - baseNumPoints), 
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.ComputeBounds(this);
  return component;
}

Shape::Component
Shape::AddBox(short index, float width, float height, float depth, 
  const GfVec4f& color, const GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  _points.push_back(GfVec3f(-width * 0.5f, -height * 0.5f, -depth * 0.5f));
  _points.push_back(GfVec3f( width * 0.5f, -height * 0.5f, -depth * 0.5f));
  _points.push_back(GfVec3f(-width * 0.5f, -height * 0.5f,  depth * 0.5f));
  _points.push_back(GfVec3f( width * 0.5f, -height * 0.5f,  depth * 0.5f));
  _points.push_back(GfVec3f(-width * 0.5f,  height * 0.5f, -depth * 0.5f));
  _points.push_back(GfVec3f( width * 0.5f,  height * 0.5f, -depth * 0.5f));
  _points.push_back(GfVec3f(-width * 0.5f,  height * 0.5f,  depth * 0.5f));
  _points.push_back(GfVec3f( width * 0.5f,  height * 0.5f,  depth * 0.5f));

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  static const float indices[36] = {
    0, 2, 3, 0, 3, 1, 0, 1, 5, 0, 5, 4,
    0, 4, 6, 0, 6, 2, 1, 3, 7, 1, 7, 5,
    2, 6, 7, 2, 7, 3, 4, 5, 7, 4, 7, 6
  };

  for(size_t i=0; i < 36; ++i) {
    _indices.push_back(indices[i] + baseNumPoints);
  }

  // make component
  Shape::Component component(
    Shape::BOX, 
    index, 
    baseNumPoints,
    (_points.size() - baseNumPoints), 
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  //component.ComputeBounds(this);
  component.SetBounds(
    GfRange3f(
      GfVec3f(-width * 0.5f, -height * 0.5f, -depth * 0.5f),
      GfVec3f(width * 0.5f, height * 0.5f, depth * 0.5f)));
  return component;
}

Shape::Component 
Shape::AddSphere(short index, float radius, size_t lats, size_t longs, 
  const GfVec4f& color, const GfMatrix4f& m)
{
  // initialize data
  lats = lats < 3 ? 3 : lats;
  longs = longs < 3 ? 3 : longs;

  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  for(size_t i=0; i < longs; ++i) {
    if(i == 0) {
      _points.push_back(GfVec3f(0.f, -radius, 0.f));
    } else if(i < longs - 1) {
      float lng = M_PI *(-0.5 + (float)i/(float)(longs-1));
      float y = radius * std::sinf(lng);
      float yr = radius * std::cosf(lng);
      for(size_t j=0; j < lats; ++j) {
        float lat = 2 * M_PI * ((float)(j-1)*(1.f/(float)lats));
        float x = std::cosf(lat);
        float z = std::sinf(lat);
        _points.push_back(GfVec3f(x * yr, y, z * yr));
      }
    } else {
      _points.push_back(GfVec3f(0.f, radius, 0.f));
    }
  }

  // transform points
  size_t numPoints = _points.size();
  _TransformPoints(baseNumPoints, numPoints, m);

  // make indices
  for(size_t i = 0; i < longs - 1; ++i) {
    for(size_t j = 0; j < lats; ++j) {
      if(i == 0) {
        _indices.push_back((j + 1) % lats + 1 + baseNumPoints);
        _indices.push_back(j + 1 + baseNumPoints);
        _indices.push_back(baseNumPoints);
      } else if( i == longs - 2) {
        _indices.push_back(numPoints - 1);
        _indices.push_back(numPoints - lats + j - 1);
        if(j == lats - 1) {
          _indices.push_back(numPoints - lats - 1);
        } else {
          _indices.push_back(numPoints - lats + j);
        }
      } else {
        int i1, i2, i3, i4;
        i1 = (i - 1) * lats + j + 1;
        i4 = i1 + lats;
          
        if(j == lats - 1) {
          i2 = i1 - lats + 1;
          i3 = i1 + 1;
        } else {
          i2 = i1 + 1;
          i3 = i1 + lats + 1;
        }

        _indices.push_back(i1 + baseNumPoints);
        _indices.push_back(i2 + baseNumPoints);
        _indices.push_back(i3 + baseNumPoints);
        _indices.push_back(i1 + baseNumPoints);
        _indices.push_back(i3 + baseNumPoints);
        _indices.push_back(i4 + baseNumPoints);
      }
    }
  }

  // make component
  Shape::Component component(
    Shape::SPHERE, 
    index, 
    baseNumPoints, 
    (_points.size() - baseNumPoints), 
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(radius, 0.f, 0.f));
  return component;
}

#define ICO_SPHERE_X 0.525731112119133606
#define ICO_SPHERE_Z 0.850650808352039932

static float ICO_SPHERE_POINTS[12][3] = {
  {-ICO_SPHERE_X, 0.f, ICO_SPHERE_Z},
  {ICO_SPHERE_X, 0.f, ICO_SPHERE_Z},
  {-ICO_SPHERE_X, 0.f, -ICO_SPHERE_Z},
  {ICO_SPHERE_X, 0.f, -ICO_SPHERE_Z},

  {0.f, ICO_SPHERE_Z, ICO_SPHERE_X},
  {0.f, ICO_SPHERE_Z, -ICO_SPHERE_X},
  {0.f, -ICO_SPHERE_Z, ICO_SPHERE_X},
  {0.f, -ICO_SPHERE_Z, -ICO_SPHERE_X},

  {ICO_SPHERE_Z, ICO_SPHERE_X, 0.f},
  {-ICO_SPHERE_Z, ICO_SPHERE_X, 0.f},
  {ICO_SPHERE_Z, -ICO_SPHERE_X, 0.f},
  {-ICO_SPHERE_Z, -ICO_SPHERE_X, 0.f},
};

static int ICO_SPHERE_INDICES[20][3] = {
  {0, 4, 1}, {0, 9, 4}, {9, 5, 4}, {4, 5, 8}, {4, 8, 1},
  {8, 10, 1}, {8, 3, 10}, {5, 3, 8}, {5, 2, 3}, {2, 7, 3},
  {7, 10, 3}, {7, 6, 10}, {7,11, 6}, {11, 0, 6}, {0, 1, 6},
  {6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5}, {7, 2, 11}
};

static void IcoSubdivide(std::vector<GfVec3f>& points, 
  std::vector<int>& indices, size_t start, float radius)
{
  typedef unsigned long long Key;
  std::map<Key, int> edgeMap;
  std::vector<int> refinedIndices;

  int end = indices.size();
  for(int i=0; i < end; i += 3) {
    int ids0[3]; // outter vertices
    int ids1[3]; // edge vertices
    for(int j = 0; j < 3; ++j) {
      int k = (j + 1) % 3;
      int e0 = indices[i + j];
      int e1 = indices[i + k];
      ids0[j] = e0;
      if(e1 > e0) std::swap(e0, e1);
      Key edgeKey = Key(e0) | (Key(e1) << 32);
      std::map<Key, int>::iterator it = edgeMap.find(edgeKey);
      if(it == edgeMap.end()) {
        ids1[j] = points.size() - start;
        edgeMap[edgeKey] = ids1[j];
        points.push_back(
          (points[e0 + start] + points[e1 + start]).GetNormalized() 
            * radius);
      } else {
        ids1[j] = it->second;
      }
    }
    refinedIndices.push_back(ids0[0]);
    refinedIndices.push_back(ids1[0]);
    refinedIndices.push_back(ids1[2]);

    refinedIndices.push_back(ids0[1]);
    refinedIndices.push_back(ids1[1]);
    refinedIndices.push_back(ids1[0]);

    refinedIndices.push_back(ids0[2]);
    refinedIndices.push_back(ids1[2]);
    refinedIndices.push_back(ids1[1]);

    refinedIndices.push_back(ids1[0]);
    refinedIndices.push_back(ids1[1]);
    refinedIndices.push_back(ids1[2]);
  }

  indices = refinedIndices;
}

Shape::Component
Shape::AddIcoSphere(short index, float radius, size_t subdiv, 
  const GfVec4f& color, const GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  for(size_t i = 0; i < 12; ++i) {
    _points.push_back(GfVec3f(ICO_SPHERE_POINTS[i]) * radius);
  }

  // make indices
  if(subdiv) {
    std::vector<int> indices;
    for(size_t i = 0; i < 20; ++i) {
      indices.push_back(ICO_SPHERE_INDICES[i][0]);
      indices.push_back(ICO_SPHERE_INDICES[i][1]);
      indices.push_back(ICO_SPHERE_INDICES[i][2]);
    }
    // subdivide
    for(size_t i = 0; i < subdiv; ++i) {
      IcoSubdivide(_points, indices, baseNumPoints, radius);
    }
    // offset
    for(const auto& index: indices) {
      _indices.push_back(index + baseNumPoints);
    }
  } else {
    for(size_t i = 0; i < 20; ++i) {
      _indices.push_back(ICO_SPHERE_INDICES[i][0] + baseNumPoints);
      _indices.push_back(ICO_SPHERE_INDICES[i][1] + baseNumPoints);
      _indices.push_back(ICO_SPHERE_INDICES[i][2] + baseNumPoints);
    }
  }
  
  // tranform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make component
  Shape::Component component(
    Shape::ICOSPHERE, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f),
    m);

  component.SetBounds(GfVec3f(radius, 0.f, 0.f));
  return component;

}

Shape::Component 
Shape::AddCylinder(short index, float radius, float height, size_t lats, 
  size_t longs, const GfVec4f& color, const GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  longs = (longs < 2) ? 2 : longs;
  lats = (lats < 3) ? 3 : lats;

  // make points
  float stepY = height / (float) (longs - 1);
  for(size_t i=0; i< longs; ++i) {
    _MakeCircle(_points, radius, lats, 
    {1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, (float)(i * stepY), 0.f, 1.f});
  }

  _points.push_back(GfVec3f(0.f, 0.f, 0.f));
  _points.push_back(GfVec3f(0.f, height, 0.f));

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);
    
  // make indices
  for(size_t i=0; i < longs - 1; ++i) {
    for(size_t j=0; j < lats; ++j) {
      _indices.push_back((int)(baseNumPoints + i * lats + j));
      _indices.push_back((int)(baseNumPoints + i * lats + ((j + 1) % lats)));
      _indices.push_back((int)(baseNumPoints + (i + 1) * lats + ((j + 1) % lats)));

      _indices.push_back((int)(baseNumPoints + i * lats + j));
      _indices.push_back((int)(baseNumPoints + (i + 1) * lats + ((j + 1) % lats)));
      _indices.push_back((int)(baseNumPoints + (i + 1) * lats + j));
    }
  };
  size_t numPoints = _points.size();
  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(numPoints - 2);
    _indices.push_back(baseNumPoints + (i + 1) % lats);
    _indices.push_back(baseNumPoints + i);
  }
  size_t baseRow = numPoints - (lats + 2);
  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(numPoints - 1);
    _indices.push_back(baseRow + i);
    _indices.push_back(baseRow + (i + 1) % lats);
  }

  // make component
  Shape::Component component(
    Shape::CYLINDER, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(radius + 0.05f, height, 0.f));

  return component;
}

Shape::Component 
Shape::AddTube(short index, float outRadius, float inRadius, float height,
  size_t lats, size_t longs, const GfVec4f& color, 
  const GfMatrix4f& m)
{
  // initialize data
  longs = longs < 2 ? 2 : longs;
  lats = lats < 3 ? 3 : lats;

  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  float baseY = -height * 0.5f;
  float stepY = height / (float)(longs - 1);
  for(size_t i = 0; i < longs; ++i) {
    GfMatrix4f m = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, baseY + i * stepY, 0.f, 1.f
    };
    _MakeCircle(_points, outRadius, lats, m);
    _MakeCircle(_points, inRadius, lats, m);
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(size_t x = 0; x < lats; ++x) {
    _indices.push_back(x + baseNumPoints);
    _indices.push_back((x + 1) % lats + baseNumPoints);
    _indices.push_back((x + 1) % lats + lats + baseNumPoints);
    _indices.push_back(x + baseNumPoints);
    _indices.push_back((x + 1) % lats + lats + baseNumPoints);
    _indices.push_back(x + lats + baseNumPoints);
  }

  size_t baseRowIdx = baseNumPoints + (longs - 1) * lats * 2;
  for(size_t x = 0; x < lats; ++x) {
    _indices.push_back(baseRowIdx + x);
    _indices.push_back(baseRowIdx + (x + 1) % lats);
    _indices.push_back(baseRowIdx + (x + 1) % lats + lats);
    _indices.push_back(baseRowIdx + x);
    _indices.push_back(baseRowIdx + (x + 1) % lats + lats);
    _indices.push_back(baseRowIdx + x + lats);
  }

  for(size_t y = 0; y < longs -1; ++y) {
    for(size_t x = 0; x < lats; ++x) {
      _indices.push_back(x + baseNumPoints);
      _indices.push_back((x + 1) % lats + baseNumPoints);
      _indices.push_back((x + 1) % lats + 2 * lats + baseNumPoints);
      _indices.push_back(x + baseNumPoints);
      _indices.push_back((x + 1) % lats + 2 * lats + baseNumPoints);
      _indices.push_back(x + 2 * lats + baseNumPoints);

      _indices.push_back(x + 3 * lats + baseNumPoints);
      _indices.push_back((x + 1) % lats + 3 * lats + baseNumPoints);
      _indices.push_back(x + lats + baseNumPoints);
      _indices.push_back((x + 1) % lats + 3 * lats + baseNumPoints);
      _indices.push_back((x + 1) % lats + lats + baseNumPoints);
      _indices.push_back(x + lats + baseNumPoints);
    }
  }

  // make component
  Shape::Component  component(
    Shape::TUBE, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(inRadius - 0.05f, outRadius + 0.05f, height));
  return component;
}

Shape::Component 
Shape::AddCone(short index, float radius, float height, size_t lats, 
  const GfVec4f& color, const GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  lats = lats < 3 ? 3 : lats;

  // make points
  _MakeCircle(_points, radius, lats,
    {1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, -height * 0.5f, 0.f, 1.f});
  _points.push_back(GfVec3f(0.f, height* 0.5f, 0.f));

  // tranform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  size_t lastPointIdx = _points.size() - 1;
  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(baseNumPoints + i);
    _indices.push_back(baseNumPoints + ((i + 1) % lats));
    _indices.push_back(lastPointIdx);
  }

  // make component
  Shape::Component  component(
    Shape::CONE, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(radius, height, 0.f));
  return component;
}

Shape::Component 
Shape::AddTorus(short index, float radius, float section, size_t lats, 
  size_t longs, const GfVec4f& color, const GfMatrix4f& m)
{
  // check data
  lats = (lats < 3) ? 3 : lats;
  longs = (longs < 3) ? 3 : longs;
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  size_t torusNumPoints = lats * longs;

  // make points
  float angleInc = 360.f / (float)lats;
  float sectionAngleInc = 360.f / (float)longs;
  float cradius, yval;

  for(int i = 0; i < longs; ++i){
    cradius = radius + (section * std::cosf(i * sectionAngleInc * DEGREES_TO_RADIANS));
    yval = section * std::sinf(i * sectionAngleInc * DEGREES_TO_RADIANS);
    for(int j = 0; j < lats; ++j) {
      _points.push_back(
        GfVec3f(
          cradius * std::cosf(j * angleInc * DEGREES_TO_RADIANS),
          yval,
          cradius * std::sinf(j * angleInc * DEGREES_TO_RADIANS)));
    }
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(int i = 0; i < longs; ++i) {
    for(int j = 0; j < lats; ++j) {
      int i1, i2, i3, i4;
      i1 = baseNumPoints + (j + i * lats);
      if(j < lats -1) {
        i2 = baseNumPoints + ((j + i * lats + 1) % torusNumPoints);
        i3 = baseNumPoints + ((j + (i+1) * lats + 1) % torusNumPoints);
      } else {
        i2 = baseNumPoints + ((j + (i-1) * lats + 1) % torusNumPoints);
        i3 = baseNumPoints + ((j + (i) * lats + 1) % torusNumPoints);
      }
      i4 = baseNumPoints + ((j + (i+1) * lats) % torusNumPoints);
      
      _indices.push_back(i1);
      _indices.push_back(i2);
      _indices.push_back(i3);
      _indices.push_back(i1);
      _indices.push_back(i3);
      _indices.push_back(i4);

    } 
  }

  // make component
  Shape::Component component(
    Shape::TORUS, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(radius, section * 2.f, 0.f));
  return component;
}

Shape::Component
Shape::AddExtrusion(short index, 
  const std::vector<GfMatrix4f>& xfos,
  const std::vector<GfVec3f>& profile, 
  const GfVec4f& color,
  const GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  size_t numPoints = profile.size();
  size_t numXfos = xfos.size();

  bool closeU = false;
  //  GfIsClose(xfos.front(), xfos.back(), 0.001f) ? true : false;
  bool closeV = false;
   // GfIsClose(profile.front(), profile.back(), 0.001f) ? true : false;

  // make points
  size_t numLongs = closeU ? numXfos - 1 : numXfos;
  size_t numLats = closeV ? numPoints - profile.size() : numPoints;
  for(size_t i=0; i < numLongs; ++i) {
    for(size_t j=0; j < numLats; ++j) {
      _points.push_back(xfos[i].Transform(profile[j]));
    }
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(size_t i=0; i < numLongs - 1; ++i) {
    size_t baseIdx = baseNumPoints + i * numPoints;
    for(size_t j=0; j < numLats - 1; ++j) {
      _indices.push_back(baseIdx + j);
      _indices.push_back(baseIdx + ((j + 1) % numLats));
      _indices.push_back(baseIdx + ((j + 1) % numLats ) + numLats);

      _indices.push_back(baseIdx + j);
      _indices.push_back(baseIdx + ((j + 1) % numLats ) + numLats);
      _indices.push_back(baseIdx + j + numLats);
    }
  }

  // make component
  Shape::Component component(
    Shape::EXTRUSION, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.ComputeBounds(this);
  return component;

}

Shape::Component
Shape::AddPoints(
  const std::vector<GfVec3f>& points,
  const GfVec4f& color,
  const GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // add points
  for (auto& point: points) {
    _points.push_back(point);
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // add indices
  GfRange3f range;
  for (size_t i = 0; i < points.size(); ++i) {
    _indices.push_back(baseNumPoints + i);
    range.ExtendBy(points[i]);
  }

  // make component
  Shape::Component component(
    Shape::POINTS,
    -1,
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices,
    _indices.size() - baseNumIndices,
    color,
    GfMatrix4f(1.f),
    m);

  component.SetBounds(range.GetSize());
  component.SetMode(POINT);
  return component;
}

Shape::Component 
Shape::AddDisc(short index, float radius, size_t lats,
  const GfVec4f& color, const GfMatrix4f& m)
{
  return Shape::AddDisc(index, radius, 0.f, 360.f, lats, color, m);
}

Shape::Component 
Shape::AddDisc(short index, float radius, float start, float end, size_t lats,
  const GfVec4f& color, const GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  if(lats < 3) lats = 3;
  float totalAngle = end - start; 
  bool closed = (totalAngle >= 360.f);
  float step = ((end - start) / (float)lats);
  if(!closed) lats++;

  // make points
  for(size_t i=0; i < lats; ++i) {
    float currentAngle = (start + i * step) * DEGREES_TO_RADIANS;
    _points.push_back(
      GfVec3f(
        std::sinf(currentAngle) * radius, 
        0.f, 
        std::cosf(currentAngle) * radius));
  }
  _points.push_back(GfVec3f(0.f));
  size_t lastIdx = _points.size() - 1;

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  if(closed) {
    for(size_t i = 0; i < lats; ++i) {
      _indices.push_back(baseNumPoints + i);
      _indices.push_back(baseNumPoints + ((i + 1) % lats));
      _indices.push_back(lastIdx);
    }
  } else {
    for(size_t i = 0; i < lats - 1; ++i) {
      _indices.push_back(baseNumPoints + i);
      _indices.push_back(baseNumPoints + (i + 1));
      _indices.push_back(lastIdx);
    }
  }
  // make component
  Shape::Component component(
    Shape::DISC, 
    index, 
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices, 
    _indices.size() - baseNumIndices, 
    color, 
    GfMatrix4f(1.f), 
    m);

  component.SetBounds(GfVec3f(radius, 0.f, 0.f));
  return component;
}

Shape::Component
Shape::AddRing(short index, float radius, float section, size_t lats,
  const GfVec4f& color, const GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  if (lats < 3) lats = 3;
  size_t ringNumPoints = lats * 2;
  float step = 360.f / lats;
  float innerRadius = radius - section;
  float outerRadius = radius + section;

  // make points
  for (size_t i = 0; i < lats; ++i) {
    float currentAngle = (i * step) * DEGREES_TO_RADIANS;
    _points.push_back(
      GfVec3f(
        std::sinf(currentAngle) * innerRadius,
        0.f,
        std::cosf(currentAngle) * innerRadius));
    _points.push_back(
      GfVec3f(
        std::sinf(currentAngle) * outerRadius,
        0.f,
        std::cosf(currentAngle) * outerRadius));
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for (size_t i = 0; i < lats * 2; i += 2) {
    _indices.push_back(baseNumPoints + i);
    _indices.push_back(baseNumPoints + i + 1);
    _indices.push_back(baseNumPoints + ((i + 3) % ringNumPoints));
    _indices.push_back(baseNumPoints + i);
    _indices.push_back(baseNumPoints + ((i + 3) % ringNumPoints));
    _indices.push_back(baseNumPoints + ((i + 2) % ringNumPoints));
  }
  // make component
  Shape::Component component(
    Shape::RING,
    index,
    baseNumPoints,
    _points.size() - baseNumPoints,
    baseNumIndices,
    _indices.size() - baseNumIndices,
    color,
    GfMatrix4f(1.f),
    m);

  component.SetBounds(GfVec3f(radius, (float)section, 0.f));
  return component;
}

void Shape::Clear()
{
  _points.clear();
  _indices.clear();
  _components.clear();
}

void Shape::Setup(bool dynamic)
{
  // current vao
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

  // create or reuse vertex array object
  if(!_vao) glGenVertexArrays(1,&_vao);
  glBindVertexArray(_vao);
      
  // create or reuse vertex buffer object
  if(!_vbo) glGenBuffers(1,&_vbo);
  glBindBuffer(GL_ARRAY_BUFFER,_vbo);

  // fill buffer data
  size_t length = _points.size() * 3 * sizeof(float);
  if(length) {
    glBufferData(GL_ARRAY_BUFFER,length,NULL, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW );
    glBufferSubData(GL_ARRAY_BUFFER, 0, length, &_points[0][0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
  
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1,4,#GL_FLOAT,#GL_FALSE,0,plength);
  }

  // create or reuse element array object
  if (!_eab) glGenBuffers(1, &_eab);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eab);

  if (_indices.size()) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(int),
      &_indices[0], dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  }


  glBindVertexArray(vao);
}

void Shape::UpdateCamera(const GfMatrix4f& view, 
  const GfMatrix4f& proj)
{
  GLuint pgm = _pgm->Get();
  glUseProgram(pgm);
  GLuint uView = glGetUniformLocation(pgm, "view");
  GLuint uProj = glGetUniformLocation(pgm, "proj");

  glUniformMatrix4fv(uView,1,GL_FALSE,&view[0][0]);
  glUniformMatrix4fv(uProj,1,GL_FALSE,&proj[0][0]);
}

void Shape::Bind()
{
  GLuint program = _pgm->Get();
  _uModel = glGetUniformLocation(program, "model");
  _uColor = glGetUniformLocation(program, "color");

  glUseProgram(program);
  glBindVertexArray(_vao);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Shape::Unbind()
{
  glBindVertexArray(0);
}

void Shape::DrawComponent(size_t index, const GfMatrix4f& model, 
  const GfVec4f& color)
{
  glUniformMatrix4fv(_uModel,1,GL_FALSE,&model[0][0]);
  glUniform4f(_uColor, color[0], color[1], color[2], color[3]);
  
  const Shape::Component& comp = _components[index];
  switch (comp.mode) {
  case POINT:

    break;
  case LINE:

    break;
  case TRIANGLE:
    glDrawElements(GL_TRIANGLES, comp.numIndices,
      GL_UNSIGNED_INT, ((char*)NULL + (comp.baseIndices * sizeof(unsigned int))));
    break;
  }
  
}

void Shape::Draw(const GfMatrix4f& model, const GfVec4f& color)
{
  GLuint pgm = _pgm->Get();
  GLuint uModel = glGetUniformLocation(pgm, "model");
  GLuint uColor = glGetUniformLocation(pgm, "color");

  glBindVertexArray(_vao);
  glUniformMatrix4fv(uModel,1,GL_FALSE,&model[0][0]);
  glUniform4f(uColor, color[0], color[1], color[2], color[3]);
  switch (_mode) {
  case POINT:
    break;
  case LINE:
    break;
  case TRIANGLE:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    break;
  }
  
}

void Shape::Draw(const GfMatrix4f& model, const GfVec4f& color,
  size_t start, size_t end)
{
  GLuint pgm = _pgm->Get();
  GLuint uModel = glGetUniformLocation(pgm, "model");
  GLuint uColor = glGetUniformLocation(pgm, "color");

  glBindVertexArray(_vao);
  glUniformMatrix4fv(uModel,1,GL_FALSE,&model[0][0]);
  glUniform4f(uColor, color[0], color[1], color[2], color[3]);
  
  switch (_mode) {
  case POINT:
    break;
  case LINE:
    break;
  case TRIANGLE:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (end - start), GL_UNSIGNED_INT,
      ((char*)NULL + (start * sizeof(unsigned int))));
    break;
  }
  
}

GLSLProgram* InitShapeShader(void* ctxt)
{
  if (SHAPE_PROGRAM_MAP.find(ctxt) != SHAPE_PROGRAM_MAP.end()) {
    return SHAPE_PROGRAM_MAP[ctxt];
  }
  GLSLProgram*  pgm = new GLSLProgram();
  GlfContextCaps const & caps = GlfContextCaps::GetInstance();
  if (caps.glVersion < 330) {
    pgm->BuildFromString(
      "SimpleShape", 
      SIMPLE_VERTEX_SHADER_CODE_120, 
      SIMPLE_FRAGMENT_SHADER_CODE_120);
  } else {
    pgm->BuildFromString(
      "SimpleShape", 
      SIMPLE_VERTEX_SHADER_CODE_330, 
      SIMPLE_FRAGMENT_SHADER_CODE_330);
  }
  SHAPE_PROGRAM_MAP[ctxt] = pgm;
  return pgm;
}

JVR_NAMESPACE_CLOSE_SCOPE