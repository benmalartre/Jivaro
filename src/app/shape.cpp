#include "shape.h"
#include <iostream>
#include <map>

AMN_NAMESPACE_OPEN_SCOPE

Shape::Shape() : _numPoints(0), _numTriangles(0) {};

Shape::~Shape(){};

void Shape::_MakeCircle(std::vector<pxr::GfVec3f>& points, float radius, 
  size_t numPoints, const pxr::GfMatrix4f& m)
{
  float step = 360 / numPoints;
  for(size_t i = 0; i < numPoints; ++i) {
    points.push_back(m.TransformAffine(
      pxr::GfVec3f(std::sinf(step*i), 0.f, std::cosf(step*i))));
  }
}

void Shape::_TransformPoints(size_t startIdx, size_t endIdx, 
  const pxr::GfMatrix4f& m)
{
  for(size_t i=startIdx; i < endIdx; ++i) {
    _points[i] = m.TransformAffine(_points[i]);
  }
}


void Shape::AddGrid(float width, float depth, size_t divX, size_t divZ, 
  const pxr::GfMatrix4f& m)
{
  // initialize data
  divX = divX < 2 ? 2 : divX;
  divZ = divZ < 2 ? 2 : divZ;

  size_t baseNumPoints = _numPoints;
  _numPoints += divX * divZ;
  _numTriangles += ((divX -1) * (divZ - 1)) * 2;

  // make points
  float baseX = -width * 0.5f;
  float baseZ = -depth * 0.5f;
  float stepX = (width - 1) / divX;
  float stepZ = (depth - 1) / divZ;

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
}

void Shape::AddBox(float width, float height, float depth, 
  const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _numPoints;
  _numPoints += 8;
  _numTriangles += 12;

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
    0, 3, 3, 0, 3, 1, 0, 1, 5, 0, 5, 4,
    0, 4, 6, 0, 6, 2, 1, 3, 7, 1, 7, 5,
    2, 6, 7, 2, 7, 3, 4, 5, 7, 4, 7, 6
  };
  
  for(size_t i=0; i < 36; ++i) {
    _indices.push_back(indices[i]);
  }
}

void Shape::AddSphere(float radius, size_t lats, size_t longs, 
   const pxr::GfMatrix4f& m)
{
  // initialize data
  lats = lats < 3 ? 3 : lats;
  longs = longs < 3 ? 3 : longs;

  size_t baseNumPoints = _numPoints;
  size_t numPoints = (longs-2)*lats+2;
  size_t numTriangles = (longs - 2) * 2 * lats;
    
  _numPoints += numPoints;
  _numTriangles += numTriangles;

  // make points
  for(size_t i=0; i < longs; ++i) {
    if(i == 0) {
      _points.push_back(pxr::GfVec3f(0.f, -radius, 0.f));
    } else if(i == longs - 1) {
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
  _TransformPoints(baseNumPoints, _points.size(), m);

  // make indices
  for(size_t i = 0; i < longs - 1; ++i) {
    for(size_t j = 0; j < lats; ++j) {
      if(i == 0) {
        _indices.push_back((j + 1) % lats + 1);
        _indices.push_back(j + 1);
        _indices.push_back(0);
      } else if( j == longs - 2) {
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
  std::vector<int>& indices, size_t startIdx, float radius)
{
  typedef unsigned long long Key;
  std::map<Key, int> edgeMap;
  std::vector<int> refinedIndices;

  int end = indices.size();
  for(int i=startIdx; i < end; i += 3) {
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
        ids1[j] = points.size();
        edgeMap[edgeKey] = ids1[j];
        points.push_back((points[e0] + points[e1]).GetNormalized() * radius);
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

void Shape::AddIcoSphere(float radius, size_t subdiv, const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _numPoints;
  _numPoints += subdiv ? 12 * subdiv : 12;
  _numTriangles += subdiv ? 20 * subdiv : 20;

  // make points
  for(size_t i = 0; i < 12; ++i) {
    _points.push_back(pxr::GfVec3f(ICO_SPHERE_POINTS[i]));
  }

  // make indices
  for(size_t i = 0; i < 20; ++i) {
    _indices.push_back(ICO_SPHERE_INDICES[i][0] + baseNumPoints);
    _indices.push_back(ICO_SPHERE_INDICES[i][1] + baseNumPoints);
    _indices.push_back(ICO_SPHERE_INDICES[i][2] + baseNumPoints);
  }

  // subdivide
  for(size_t i = 0; i < subdiv; ++i) {
    IcoSubdivide(_points, _indices, baseNumPoints, radius);
  }

  // tranform points
  _TransformPoints(baseNumPoints, _points.size(), m);

  // update data
  _numPoints = _points.size();
  _numTriangles = _indices.size() / 3;

}

void Shape::AddCylinder(float radius, float height, size_t lats, size_t longs, 
  const pxr::GfMatrix4f& m)
{
  // initialize data
  size_t baseNumPoints = _numPoints;
  longs = longs < 2 ? 2 : longs;
  lats = lats < 3 ? 3 : lats;

  _numPoints += lats * longs + 2;
  size_t numSideTriangles = lats * (longs -1) * 2;
  size_t numCapTriangles = lats * 2;

  _numTriangles += numSideTriangles + numCapTriangles;

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

  _points[_numPoints - 2] = pxr::GfVec3f(0.f, -height * 0.5f, 0.f);
  _points[_numPoints - 1] = pxr::GfVec3f(0.f, height * 0.5f, 0.f);

  // transform points
  _TransformPoints(baseNumPoints, _points.size(), m);
  
  // make indices
  for(size_t i=0; i < longs - 1; ++i) {
    for(size_t j=0; j < lats; ++j) {
      _indices.push_back(baseNumPoints + i * lats + j);
      _indices.push_back(baseNumPoints + i * lats + ((j + 1) % lats));
      _indices.push_back((baseNumPoints + i + 1) * lats + ((j + 1) % lats));

      _indices.push_back(baseNumPoints + i * lats + j);
      _indices.push_back(baseNumPoints + (i + 1) * lats + ((j + 1) % lats));
      _indices.push_back(baseNumPoints + (i + 1) * lats + j);
    }
  };

  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(_numPoints - 2);
    _indices.push_back(baseNumPoints + i);
    _indices.push_back(baseNumPoints + (i + 1) % lats);
  }

  size_t baseRow = _numPoints - (lats + 2);
  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(_numPoints - 1);
    _indices.push_back(baseNumPoints + baseRow + i);
    _indices.push_back(baseNumPoints + baseRow + (i + 1) % lats);
  }
}

void Shape::AddTube(float outRadius, float inRadius, float height, size_t lats,
  size_t longs, const pxr::GfMatrix4f& m)
{
  // initialize data
  longs = longs < 2 ? 2 : longs;
  lats = lats < 3 ? 3 : lats;

  _numPoints += lats * 2 * longs;
  size_t numSideTriangles = lats * (longs -1) * 4;
  size_t numCapTriangles = lats * 4;

  _numTriangles += numSideTriangles + numCapTriangles;

  std::cout << "------- TUBE -----------------" << std::endl;
  std::cout << "NUM POINTS : " << _numPoints << std::endl;
  std::cout << "NUM TRIANGLES : " << _numTriangles << std::endl;

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
  std::cout << "NUM REAL POINTS : " << _points.size() << std::endl;

  // make indices
  for(size_t y = 0; y < longs; ++y) {
    if(y == 0) {
      for(size_t x = 0; x < lats; ++x) {
        _indices.push_back(x);
        _indices.push_back((x + 1) % lats);
        _indices.push_back((x + 1) % lats + lats);
        _indices.push_back(x);
        _indices.push_back((x + 1) % lats + lats);
        _indices.push_back(x + lats);
      }
    } else if(y == longs - 1) {

    }
  }

}

void Shape::AddCone(float radius, float height, size_t lats, 
  const pxr::GfMatrix4f& m)
{
  size_t baseNumPoints = _numPoints;
  lats = lats < 3 ? 3 : lats;

  _numPoints += lats + 1;
  _numTriangles += lats;

  // make points
  _MakeCircle(_points, radius, lats,
    {1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, -height * 0.5f, 0.f, 1.f});
  _points.push_back(pxr::GfVec3f(0.f, height* 0.5f, 0.f));

  // make indices
  size_t lastPointIdx = _points.size() - 1;
  for(size_t i=0; i < lats; ++i) {
    _indices.push_back(baseNumPoints + i);
    _indices.push_back(baseNumPoints + ((i + 1) % lats));
    _indices.push_back(lastPointIdx);
  }
}

void InitStaticShapes()
{
  CYLINDER_SHAPE.AddCylinder(0.05f, 1.f, 8, 1);

  GRID_SHAPE.AddGrid(1.f, 1.f, 1, 1);

  CONE_SHAPE.AddCone(0.5f, 1.f, 8);

  SPHERE_SHAPE.AddSphere(0.5f, 8, 8);

  ICOSPHERE_SHAPE.AddIcoSphere(0.5f, 2);

  TUBE_SHAPE.AddTube(1.f, 0.95f, 0.1f, 12);

  SHAPE_INITIALIZED = true;
}

AMN_NAMESPACE_CLOSE_SCOPE