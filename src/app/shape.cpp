#include "shape.h"
#include "../utils/shaders.h"
#include <iostream>
#include <map>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>

AMN_NAMESPACE_OPEN_SCOPE

GLSLProgram* SHAPE_PROGRAM = NULL;

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

void Shape::Component::SetBounds(const pxr::GfVec3f& xyz)
{
  bounds = pxr::GfRange3f(pxr::GfVec3f(0.f), xyz);
}

void Shape::Component::ComputeBounds(Shape* shape)
{
  pxr::GfMatrix4f invMatrix = offset.GetInverse();
  bounds.SetEmpty();
  const std::vector<pxr::GfVec3f>& points = shape->GetPoints();
  for(size_t e = start; e < end; ++e) {
    bounds.UnionWith(invMatrix.Transform(points[e]));
  }
}

size_t 
Shape::Component::Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m)
{
  switch(type) {
    case GRID:
    {
      const pxr::GfVec3f& bound = bounds.GetMax();
      pxr::GfRange3d range(
        pxr::GfVec3d(-bound[0] * 0.5f, 0.f, -bound[1] * 0.5f),
        pxr::GfVec3d(bound[0] * 0.5f, 0.f, bound[1] * 0.5f));
      pxr::GfBBox3d bbox(range);
      bbox.Transform(pxr::GfMatrix4d(offset * m));
      if(ray.Intersect(bbox)) {
        hovered = true;
        return index;
      } else {
        hovered = false;
      }
      break;
    }
    case BOX:
    {

    }
    case SPHERE:
    {

    }
    case ICOSPHERE:
    {

    }
    case DISC:
    {

    }
    case CYLINDER:
    {
      const pxr::GfVec3f& bound = bounds.GetMax();
      pxr::GfMatrix4f matrix = offset * m;
      /*
      pxr::GfVec3d origin = matrix.Transform(pxr::GfVec3d(0, 0, 0));
      pxr::GfVec3d axis = matrix.TransformDir(pxr::GfVec3d(0, bound[1], 0));
      if(ray.Intersect(origin, axis, bound[0])) {
        hovered = true;
        return index;
      } else {
        hovered = false;
      }
      */
      pxr::GfRange3d range(
        pxr::GfVec3d(-bound[0], 0.f, -bound[0]),
        pxr::GfVec3d(bound[0], bound[1], bound[0]));
      pxr::GfBBox3d bbox(range);
      bbox.Transform(pxr::GfMatrix4d(offset * m));
      if(ray.Intersect(bbox)) {
        hovered = true;
        return index;
      } else {
        hovered = false;
      }
      break;
    }
    case TUBE:
    {

    }
    case CONE:
    {

    }
    case CAPSULE:
    {

    }
    case TORUS:
    {

    }
  }
  
  return 0;
}

Shape::Shape() : _vao(0), _vbo(0), _eab(0) {};

Shape::~Shape()
{
  if(_vbo) glDeleteBuffers(1,&_vbo);
  if(_eab) glDeleteBuffers(1,&_eab);
  if(_vao) glDeleteVertexArrays(1,&_vao); 
};

void Shape::UpdateComponents(short hovered, short active)
{
  for(Shape::Component& component: _components) {
    component.hovered = (component.index == hovered);
    component.active = (component.index == active);
  }
}

void Shape::AddComponent(const Component& component)
{
  _components.push_back(component);
}

void Shape::AddComponent(short type, short index, size_t start, size_t end, 
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  _components.push_back(Component(type, index, start, end, color, m));
}

void Shape::_MakeCircle(std::vector<pxr::GfVec3f>& points, float radius, 
  size_t numPoints, const pxr::GfMatrix4f& m)
{
  float step = (360 / (float)numPoints) * DEGREES_TO_RADIANS;
  for(size_t k = 0; k < numPoints; ++k) {
    points.push_back(m.TransformAffine(
      pxr::GfVec3f(std::sinf(step*k) * radius, 0.f, std::cosf(step*k) * radius)));
  }
}

void Shape::_TransformPoints(size_t start, size_t end, 
  const pxr::GfMatrix4f& m)
{
  for(size_t k=start; k < end; ++k) {
    _points[k] = m.TransformAffine(_points[k]);
  }
}

size_t
Shape::Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m)
{
  size_t index = 0;
  for(auto& component: _components) {
    size_t intersected = component.Intersect(ray, m);
    if(intersected && !index) index = intersected;
  }
  return index;
}

Shape::Component
Shape::AddGrid(size_t index, float width, float depth, size_t divX, 
  size_t divZ, const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
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
        pxr::GfVec3f(baseX + x * stepX, 0.f, baseZ + z * stepZ));
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
  Shape::Component component(Shape::GRID, index, baseNumIndices, _indices.size(),  
    color, m);
  component.ComputeBounds(this);
  return component;
}

Shape::Component
Shape::AddBox(size_t index, float width, float height, float depth, 
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  _points.push_back(pxr::GfVec3f(-width * 0.5f, -height * 0.5f, -depth * 0.5f));
  _points.push_back(pxr::GfVec3f( width * 0.5f, -height * 0.5f, -depth * 0.5f));
  _points.push_back(pxr::GfVec3f(-width * 0.5f, -height * 0.5f,  depth * 0.5f));
  _points.push_back(pxr::GfVec3f( width * 0.5f, -height * 0.5f,  depth * 0.5f));
  _points.push_back(pxr::GfVec3f(-width * 0.5f,  height * 0.5f, -depth * 0.5f));
  _points.push_back(pxr::GfVec3f( width * 0.5f,  height * 0.5f, -depth * 0.5f));
  _points.push_back(pxr::GfVec3f(-width * 0.5f,  height * 0.5f,  depth * 0.5f));
  _points.push_back(pxr::GfVec3f( width * 0.5f,  height * 0.5f,  depth * 0.5f));

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
  Shape::Component component(Shape::BOX, index, baseNumIndices, _indices.size(),  
    color, m);
  component.ComputeBounds(this);
  return component;
}

Shape::Component 
Shape::AddSphere(size_t index, float radius, size_t lats, size_t longs, 
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  // initialize data
  lats = lats < 3 ? 3 : lats;
  longs = longs < 3 ? 3 : longs;

  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  for(size_t i=0; i < longs; ++i) {
    if(i == 0) {
      _points.push_back(pxr::GfVec3f(0.f, -radius, 0.f));
    } else if(i < longs - 1) {
      float lng = M_PI *(-0.5 + (float)i/(float)(longs-1));
      float y = radius * std::sinf(lng);
      float yr = radius * std::cosf(lng);
      for(size_t j=0; j < lats; ++j) {
        float lat = 2 * M_PI * ((float)(j-1)*(1.f/(float)lats));
        float x = std::cosf(lat);
        float z = std::sinf(lat);
        _points.push_back(pxr::GfVec3f(x * yr, y, z * yr));
      }
    } else {
      _points.push_back(pxr::GfVec3f(0.f, radius, 0.f));
    }
  }

  // transform points
  size_t numPoints = _points.size();
  _TransformPoints(baseNumPoints, numPoints, m);

  // make indices
  for(size_t i = 0; i < longs - 1; ++i) {
    for(size_t j = 0; j < lats; ++j) {
      if(i == 0) {
        _indices.push_back((j + 1) % lats + 1);
        _indices.push_back(j + 1);
        _indices.push_back(0);
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

        _indices.push_back(i1);
        _indices.push_back(i2);
        _indices.push_back(i3);
        _indices.push_back(i1);
        _indices.push_back(i3);
        _indices.push_back(i4);
      }
    }
  }

  // make component
  Shape::Component component(Shape::SPHERE, index, baseNumIndices, 
    _indices.size(), color, m);
  component.SetBounds(pxr::GfVec3f(radius, 0.f, 0.f));
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

static void IcoSubdivide(std::vector<pxr::GfVec3f>& points, 
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
Shape::AddIcoSphere(size_t index, float radius, size_t subdiv, 
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();

  // make points
  for(size_t i = 0; i < 12; ++i) {
    _points.push_back(pxr::GfVec3f(ICO_SPHERE_POINTS[i]) * radius);
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
  Shape::Component component(Shape::CYLINDER, index, baseNumIndices, _indices.size(),  
    color, m);
  component.SetBounds(pxr::GfVec3f(radius, 0.f, 0.f));
  return component;

}

Shape::Component 
Shape::AddCylinder(size_t index, float radius, float height, size_t lats, 
  size_t longs, const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  longs = (longs < 2) ? 2 : longs;
  lats = (lats < 3) ? 3 : lats;

  // make points
  float stepY = height / (float) (longs - 1);
  float baseY = -height * 0.5f;
  for(size_t i=0; i< longs; ++i) {
    _MakeCircle(_points, radius, lats, 
    {1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, baseY + (i * stepY), 0.f, 1.f});
  }

  _points.push_back(pxr::GfVec3f(0.f, -height * 0.5f, 0.f));
  _points.push_back(pxr::GfVec3f(0.f, height * 0.5f, 0.f));

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
    _indices.push_back(baseNumPoints + baseRow + i);
    _indices.push_back(baseNumPoints + baseRow + (i + 1) % lats);
  }

  // make component
  Shape::Component component(Shape::CYLINDER, index, baseNumIndices, 
    _indices.size(), color, m);
  component.SetBounds(pxr::GfVec3f(radius * 2.f, height, 0.f));
  return component;
}

Shape::Component 
Shape::AddTube(size_t index, float outRadius, float inRadius, float height,
  size_t lats, size_t longs, const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
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
    pxr::GfMatrix4f m = {
      1.f, 0.f, 0.f, 0.f,
      0.f, 1.f, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, i * stepY, 0.f, 1.f
    };
    _MakeCircle(_points, outRadius, lats, m);
    _MakeCircle(_points, inRadius, lats, m);
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(size_t x = 0; x < lats; ++x) {
    _indices.push_back(x);
    _indices.push_back((x + 1) % lats);
    _indices.push_back((x + 1) % lats + lats);
    _indices.push_back(x);
    _indices.push_back((x + 1) % lats + lats);
    _indices.push_back(x + lats);
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
      _indices.push_back(x);
      _indices.push_back((x + 1) % lats);
      _indices.push_back((x + 1) % lats + 2 * lats);
      _indices.push_back(x);
      _indices.push_back((x + 1) % lats + 2 * lats);
      _indices.push_back(x + 2 * lats);

      _indices.push_back(x + 3 * lats);
      _indices.push_back((x + 1) % lats + 3 * lats);
      _indices.push_back(x + lats);
      _indices.push_back((x + 1) % lats + 3 * lats);
      _indices.push_back((x + 1) % lats + lats);
      _indices.push_back(x + lats);
    }
  }

  // make component
  Shape::Component  component(Shape::TUBE, index, baseNumIndices, _indices.size(),  
    color, m);
  component.SetBounds(pxr::GfVec3f(inRadius, outRadius, height));
  return component;
}

Shape::Component 
Shape::AddCone(size_t index, float radius, float height, size_t lats, 
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
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
  _points.push_back(pxr::GfVec3f(0.f, height* 0.5f, 0.f));

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
  Shape::Component  component(Shape::CONE, index, baseNumIndices, 
    _indices.size(), color, m);
  component.SetBounds(pxr::GfVec3f(radius, height, 0.f));
  return component;
}

Shape::Component 
Shape::AddTorus(size_t index, float radius, float section, size_t lats, 
  size_t longs, const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
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
  float cradius, zval;

  for(int i = 0; i < longs; ++i){
    cradius = radius + (section * std::cosf(i * sectionAngleInc * DEGREES_TO_RADIANS));
    zval = section * std::sinf(i * sectionAngleInc * DEGREES_TO_RADIANS);
    for(int j = 0; j < lats; ++j) {
      _points.push_back(
        pxr::GfVec3f(
          cradius * std::cosf(j * angleInc * DEGREES_TO_RADIANS),
          cradius * std::sinf(j * angleInc * DEGREES_TO_RADIANS),
          zval));
    }
  }

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
	for(int i = 0; i < longs; ++i) {
		for(int j = 0; j < lats; ++j) {
			size_t i1, i2, i3, i4;
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
  Shape::Component component(Shape::TORUS, index, baseNumIndices, 
    _indices.size(), color, m);
  component.SetBounds(pxr::GfVec3f(radius, section, 0.f));
  return component;
}

Shape::Component 
Shape::AddDisc(size_t index, float radius, float start, float end, size_t lats,
  const pxr::GfVec4f& color, const pxr::GfMatrix4f& m)
{
  size_t baseNumPoints = _points.size();
  size_t baseNumIndices = _indices.size();
  if(lats < 3) lats = 3;
  float totalAngle = end - start; 
  bool closed = (totalAngle >= 360.f);
  if(!closed) lats++;

  // make points
  _points.push_back(pxr::GfVec3f(0.f));
  float step = (360.f / (float)lats);
  for(size_t i=0; i < lats; ++i) {
    float currentAngle = (start + i * step) * DEGREES_TO_RADIANS;
    _points.push_back(
      pxr::GfVec3f(std::sinf(currentAngle), 0.f, std::cosf(currentAngle)));
  }

  // make indices
  if(closed) {
    for(size_t i = 0; i < lats; ++i) {
      _indices.push_back(baseNumPoints + i);
      _indices.push_back(baseNumPoints + (i + 1) % lats);
      _indices.push_back(baseNumPoints);
    }
  } else {
    for(size_t i = 0; i < lats - 1; ++i) {
      _indices.push_back(baseNumPoints + i);
      _indices.push_back(baseNumPoints + (i + 1));
      _indices.push_back(baseNumPoints);
    }
  }
  // make component
  Shape::Component component(Shape::DISC, index, baseNumIndices, 
    _indices.size(), color, m);
  component.SetBounds(pxr::GfVec3f(radius, 0.f, 0.f));
  return component;
}

void Shape::Clear()
{
  _points.clear();
  _indices.clear();
  _components.clear();
}

void Shape::Setup()
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

  // create or reuse element array object
  if(!_eab) glGenBuffers(1,&_eab);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eab);

  // fill buffer data
  size_t length = _points.size() * 3 * sizeof(float);
  if(length) {
    glBufferData(GL_ARRAY_BUFFER,length,NULL,GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, length, &_points[0][0]);
    //glBufferSubData(GL_ARRAY_BUFFER, plength, clength, CArray::GetPtr(*item\colors,0))
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
  
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1,4,#GL_FLOAT,#GL_FALSE,0,plength);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(int), 
      &_indices[0], GL_STATIC_DRAW);
  }
  glBindVertexArray(vao);
}

void Shape::UpdateCamera(const pxr::GfMatrix4f& view, 
  const pxr::GfMatrix4f& proj)
{
  GLuint pgm = SHAPE_PROGRAM->Get();
  glUseProgram(pgm);
  GLuint uView = glGetUniformLocation(pgm, "view");
  GLuint uProj = glGetUniformLocation(pgm, "proj");

  glUniformMatrix4fv(uView,1,GL_FALSE,&view[0][0]);
  glUniformMatrix4fv(uProj,1,GL_FALSE,&proj[0][0]);
}

void Shape::Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color)
{
  GLuint pgm = SHAPE_PROGRAM->Get();
  GLuint uModel = glGetUniformLocation(pgm, "model");
  GLuint uColor = glGetUniformLocation(pgm, "color");
  
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

  glBindVertexArray(_vao);
  glUniformMatrix4fv(uModel,1,GL_FALSE,&model[0][0]);
  glUniform4f(uColor, color[0], color[1], color[2], color[3]);
  
  //glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);

  glBindVertexArray(vao);
}

void Shape::Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color,
  size_t start, size_t end)
{
  GLuint pgm = SHAPE_PROGRAM->Get();
  GLuint uModel = glGetUniformLocation(pgm, "model");
  GLuint uColor = glGetUniformLocation(pgm, "color");
  
  GLint vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);

  glBindVertexArray(_vao);
  glUniformMatrix4fv(uModel,1,GL_FALSE,&model[0][0]);
  glUniform4f(uColor, color[0], color[1], color[2], color[3]);
  
  //glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawRangeElements(GL_TRIANGLES, start, end, 
    end, GL_UNSIGNED_INT, 0);

  glBindVertexArray(vao);
}

void InitStaticShapes()
{
  // setup glsl program
  SHAPE_PROGRAM = new GLSLProgram();
  pxr::GlfContextCaps const & caps = pxr::GlfContextCaps::GetInstance();
  if (caps.glslVersion < 330) {
    SHAPE_PROGRAM->BuildFromString(
      "SimpleShapePGM", 
      SIMPLE_VERTEX_SHADER_CODE_120, 
      SIMPLE_FRAGMENT_SHADER_CODE_120);
  } else {
    SHAPE_PROGRAM->BuildFromString(
      "SimpleShapePGM", 
      SIMPLE_VERTEX_SHADER_CODE_330, 
      SIMPLE_FRAGMENT_SHADER_CODE_330);
  }
  /*
  CYLINDER_SHAPE = new Shape();
  CYLINDER_SHAPE->AddCylinder(0.05f, 5.f, 8, 1);
  CYLINDER_SHAPE->Setup();

  GRID_SHAPE = new Shape();
  GRID_SHAPE->AddGrid(1.f, 1.f, 1, 1);
  GRID_SHAPE->Setup();

  BOX_SHAPE = new Shape();
  BOX_SHAPE->AddBox();
  BOX_SHAPE->Setup();

  CONE_SHAPE = new Shape();
  CONE_SHAPE->AddCone(0.5f, 1.f, 8);
  CONE_SHAPE->Setup();

  SPHERE_SHAPE = new Shape();
  SPHERE_SHAPE->AddSphere(0.5f, 8, 8);
  SPHERE_SHAPE->Setup();

  ICOSPHERE_SHAPE = new Shape();
  ICOSPHERE_SHAPE->AddIcoSphere(0.5f, 2);
  ICOSPHERE_SHAPE->Setup();

  TUBE_SHAPE = new Shape();
  TUBE_SHAPE->AddTube(1.f, 0.5f, 0.1f, 32);
  TUBE_SHAPE->Setup();

  TORUS_SHAPE = new Shape();
  TORUS_SHAPE->AddTorus(1.f, 0.1f, 32, 8);
  TORUS_SHAPE->Setup();
  */
  SHAPE_INITIALIZED = true;
}

AMN_NAMESPACE_CLOSE_SCOPE