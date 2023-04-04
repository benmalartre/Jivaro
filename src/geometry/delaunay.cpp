#include "../geometry/delaunay.h"

JVR_NAMESPACE_OPEN_SCOPE

Delaunay::Delaunay(const std::vector<pxr::GfVec2d>& inCoords)
  : _coords(inCoords)
  , _triangles()
  , _halfEdges()
  , _hullPrev()
  , _hullNext()
  , _hullTri()
  , _hullStart()
  , _hash()
  , _center()
  , _hashSize()
  , _edgeStack() 
{
  size_t n = _coords.size() >> 1;

  pxr::GfVec2d maxCoord(std::numeric_limits<double>::min());
  pxr::GfVec2d minCoord(std::numeric_limits<double>::max());

  std::vector<size_t> ids;
  ids.reserve(n);

  for (size_t i = 0; i < n; i++) {
    const pxr::GfVec2d& coord = _coords[i];
    if (coord[0] < minCoord[0]) minCoord[0] = coord[0];
    if (coord[1] < minCoord[1]) minCoord[1] = coord[1];
    if (coord[0] > maxCoord[0]) maxCoord[0] = coord[0];
    if (coord[1] > maxCoord[1]) maxCoord[1] = coord[1];
    ids.push_back(i);
  }
  const pxr::GfVec2d center((minCoord + maxCoord) * 0.5);

  double minDist = std::numeric_limits<double>::max();

  size_t i0 = GEOM_INVALID_INDEX;
  size_t i1 = GEOM_INVALID_INDEX;
  size_t i2 = GEOM_INVALID_INDEX;

  // pick a seed point close to the centroid
  for (size_t i = 0; i < n; ++i) {
    const double d = ComputeDistanceSquared(center, _coords[i]);
    if (d < minDist) {
      i0 = i;
      minDist = d;
    }
  }

  const pxr::GfVec2d& coord0 = _coords[i0];

  minDist = std::numeric_limits<double>::max();

  // find the point closest to the seed
  for (size_t i = 0; i < n; ++i) {
    if (i == i0) continue;
    const double d = ComputeDistanceSquared(coord0, _coords[i]);
    if (d < minDist && d > 0.0) {
      i1 = i;
      minDist = d;
    }
  }

  pxr::GfVec2d coord1 = _coords[i1];

  double minRadius = std::numeric_limits<double>::max();

  // find the third point which forms the smallest circumcircle with the first two
  for(size_t i = 0; i < n; ++i) {
    if (i == i0 || i == i1) continue;

    const double r = _CircumRadius(
      coord0, coord1, _coords[i]);

    if (r < minRadius) {
      i2 = i;
      minRadius = r;
    }
  }

  if (!(minRadius < std::numeric_limits<double>::max())) {
    throw std::runtime_error("not triangulation");
  }

  pxr::GfVec2d coord2 = _coords[i2];

  if(_Orient(coord0, coord1, coord2)) {
    std::swap(i1, i2);
    std::swap(coord1, coord2);
  }
 
  _center = _CircumCenter(coord0, coord1, coord2);

  // sort the points by distance from the seed triangle circumcenter
  std::sort(ids.begin(), ids.end(), Compare{ _coords, _center });

  // initialize a hash table for storing edges of the advancing convex hull
  _hashSize = static_cast<size_t>(std::llround(std::ceil(std::sqrt(n))));
  _hash.resize(_hashSize);
  std::fill(_hash.begin(), _hash.end(), GEOM_INVALID_INDEX);

  // initialize arrays for tracking the edges of the advancing convex hull
  _hullPrev.resize(n);
  _hullNext.resize(n);
  _hullTri.resize(n);

  _hullStart = i0;

  size_t hullSize = 3;

  _hullNext[i0] = _hullPrev[i2] = i1;
  _hullNext[i1] = _hullPrev[i0] = i2;
  _hullNext[i2] = _hullPrev[i1] = i0;

  _hullTri[i0] = 0;
  _hullTri[i1] = 1;
  _hullTri[i2] = 2;

  _hash[_ComputeHash(coord0)] = i0;
  _hash[_ComputeHash(coord1)] = i1;
  _hash[_ComputeHash(coord2)] = i2;

  size_t maxTriangles = n < 3 ? 1 : 2 * n - 5;
  _triangles.reserve(maxTriangles * 3);
  _halfEdges.reserve(maxTriangles * 3);
  _AddTriangle(i0, i1, i2, GEOM_INVALID_INDEX, GEOM_INVALID_INDEX, GEOM_INVALID_INDEX);

  pxr::GfVec2d coord(std::numeric_limits<double>::quiet_NaN());

  for (size_t k = 0; k < n; ++k) {
    const size_t i = ids[k];
    const pxr::GfVec2d current = _coords[i];

    // skip near-duplicate points
    if (k > 0 && _CheckPointEquality(coord, current)) continue;
    coord = current;

    // skip seed triangle points
    if (
      _CheckPointEquality(coord, coord0) ||
      _CheckPointEquality(coord, coord1) ||
      _CheckPointEquality(coord, coord2)) continue;

    // find a visible edge on the convex hull using edge hash
    size_t start = 0;

    size_t key = _ComputeHash(coord);
    for (size_t j = 0; j < _hashSize; j++) {
      start = _hash[_FastMod(key + j, _hashSize)];
      if (start != GEOM_INVALID_INDEX && start != _hullNext[start]) break;
    }

    start = _hullPrev[start];
    size_t e = start;
    size_t q;

    while (q = _hullNext[e], !_Orient(coord, _coords[e], _coords[q])) {
      e = q;
      if (e == start) {
        e = GEOM_INVALID_INDEX;
        break;
      }
    }

    if (e == GEOM_INVALID_INDEX) continue; // likely a near-duplicate point; skip it

    // add the first triangle from the point
    size_t t = _AddTriangle(
      e,
      i,
      _hullNext[e],
      GEOM_INVALID_INDEX,
      GEOM_INVALID_INDEX,
      _hullTri[e]);

    _hullTri[i] = _Legalize(t + 2);
    _hullTri[e] = t;
    hullSize++;

    // walk forward through the hull, adding more triangles and flipping recursively
    size_t next = _hullNext[e];
    while (
      q = _hullNext[next],
      _Orient(coord, _coords[next], _coords[q])) {
      t = _AddTriangle(next, i, q, _hullTri[i], GEOM_INVALID_INDEX, _hullTri[next]);
      _hullTri[i] = _Legalize(t + 2);
      _hullNext[next] = next; // mark as removed
      hullSize--;
      next = q;
    }

    // walk backward from the other side, adding more triangles and flipping
    if (e == start) {
      while (
        q = _hullPrev[e],
        _Orient(coord, _coords[q], _coords[e])) {
        t = _AddTriangle(q, i, e, GEOM_INVALID_INDEX, _hullTri[e], _hullTri[q]);
        _Legalize(t + 2);
        _hullTri[q] = t;
        _hullNext[e] = e; // mark as removed
        hullSize--;
        e = q;
      }
    }

    // update the hull indices
    _hullPrev[i] = e;
    _hullStart = e;
    _hullPrev[next] = i;
    _hullNext[e] = i;
    _hullNext[i] = next;

    _hash[_ComputeHash(coord)] = i;
    _hash[_ComputeHash(_coords[e])] = e;
  }
}

double 
Delaunay::GetHullArea()
{
  std::vector<double> hullArea;
  size_t e = _hullStart;
  do {
    hullArea.push_back((_coords[2 * e] - _coords[2 * _hullPrev[e]]) * (_coords[2 * e + 1] + _coords[2 * _hullPrev[e] + 1]));
    e = _hullNext[e];
  } while (e != _hullStart);
  return _Sum(hullArea);
}

size_t 
Delaunay::_Legalize(size_t a) 
{
  size_t i = 0;
  size_t ar = 0;
  _edgeStack.clear();

  // recursion eliminated with a fixed-size stack
  while (true) {
    const size_t b = _halfEdges[a];

    /* if the pair of triangles doesn't satisfy the Delaunay condition
    * (p1 is inside the circumcircle of [p0, pl, pr]), flip them,
    * then do the same check/flip recursively for the new pair of triangles
    *
    *           pl                    pl
    *          /||\                  /  \
    *       al/ || \bl            al/    \a
    *        /  ||  \              /      \
    *       /  a||b  \    flip    /___ar___\
    *     p0\   ||   /p1   =>   p0\---bl---/p1
    *        \  ||  /              \      /
    *       ar\ || /br             b\    /br
    *          \||/                  \  /
    *           pr                    pr
    */
    const size_t a0 = 3 * (a / 3);
    ar = a0 + (a + 2) % 3;

    if (b == GEOM_INVALID_INDEX) {
      if (i > 0) {
        i--;
        a = _edgeStack[i];
        continue;
      }
      else {
        //i = INVALID_INDEX;
        break;
      }
    }

    const size_t b0 = 3 * (b / 3);
    const size_t al = a0 + (a + 1) % 3;
    const size_t bl = b0 + (b + 2) % 3;

    const size_t p0 = _triangles[ar];
    const size_t pr = _triangles[a];
    const size_t pl = _triangles[al];
    const size_t p1 = _triangles[bl];

    const bool illegal = _InCircle(
      _coords[p0], _coords[pr], _coords[pl], _coords[p1]);

    if (illegal) {
      _triangles[a] = p1;
      _triangles[b] = p0;

      auto hbl = _halfEdges[bl];

      // edge swapped on the other side of the hull (rare); fix the halfedge reference
      if (hbl == GEOM_INVALID_INDEX) {
        size_t e = _hullStart;
        do {
          if (_hullTri[e] == bl) {
            _hullTri[e] = a;
            break;
          }
          e = _hullNext[e];
        } while (e != _hullStart);
      }
      _Link(a, hbl);
      _Link(b, _halfEdges[ar]);
      _Link(ar, bl);
      size_t br = b0 + (b + 1) % 3;

      if (i < _edgeStack.size()) {
        _edgeStack[i] = br;
      } else {
        _edgeStack.push_back(br);
      }
      i++;
    } else {
      if (i > 0) {
        i--;
        a = _edgeStack[i];
        continue;
      } else {
        break;
      }
    }
  }
  return ar;
}

size_t 
Delaunay::_ComputeHash(const pxr::GfVec2d& coord) const
{
  const pxr::GfVec2d delta(coord - _center);
  return _FastMod(
    static_cast<size_t>(std::llround(std::floor(_PseudoAngle(delta) * static_cast<double>(_hashSize)))),
    _hashSize);
}

size_t 
Delaunay::_AddTriangle(
  size_t i0, size_t i1, size_t i2, size_t a, size_t b, size_t c)
{
  size_t t = _triangles.size();
  _triangles.push_back(i0);
  _triangles.push_back(i1);
  _triangles.push_back(i2);
  _Link(t, a);
  _Link(t + 1, b);
  _Link(t + 2, c);
  return t;
}

void 
Delaunay::_Link(size_t a, size_t b) 
{
  size_t s = _halfEdges.size();
  if (a == s) {
    _halfEdges.push_back(b);
  }
  else if (a < s) {
    _halfEdges[a] = b;
  }
  else {
    throw std::runtime_error("Cannot link edge");
  }
  if (b != GEOM_INVALID_INDEX) {
    size_t s2 = _halfEdges.size();
    if (b == s2) {
      _halfEdges.push_back(a);
    }
    else if (b < s2) {
      _halfEdges[b] = a;
    }
    else {
      throw std::runtime_error("Cannot link edge");
    }
  }
}

JVR_NAMESPACE_CLOSE_SCOPE