#ifndef JVR_GEOMETRY_DELAUNAY_H
#define JVR_GEOMETRY_DELAUNAY_H

// https://github.com/delfrrr/delaunator-cpp/blob/master/include/delaunator.hpp

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "../geometry/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

class Delaunay {

  struct Point {
    size_t i;
    double x;
    double y;
    size_t t;
    size_t prev;
    size_t next;
    bool   removed;
  };

  struct Compare {
    std::vector<pxr::GfVec3d> const& coords;
    pxr::GfVec3d coord;

    bool operator()(size_t i, size_t j) {
        const double d1 = ComputeDistanceSquared(coords[i], coord);
        const double d2 = ComputeDistanceSquared(coords[j], coord);
        return d1 < d2;
    }
  };

  public:
    std::vector<pxr::GfVec3d> const& _coords;
    std::vector<size_t> _triangles;
    std::vector<size_t> _halfEdges;
    std::vector<size_t> _hullPrev;
    std::vector<size_t> _hullNext;
    std::vector<size_t> _hullTri;
    size_t              _hullStart;

    Delaunay(std::vector<pxr::GfVec3d> const& inCoords);

    double GetHullArea();

  private:
    std::vector<size_t> _hash;
    pxr::GfVec3d        _center;
    size_t              _hashSize;
    std::vector<size_t> _edgeStack;

    size_t Legalize(size_t a);
    size_t ComputeHash(double x, double y) const;
    size_t AddTriangle(size_t i0, size_t i1, size_t i2,
      size_t a, size_t b, size_t c);
    void Link(size_t a, size_t b);
};

  Delaunay::Delaunay(std::vector<pxr::GfVec3d> const& inCoords)
    : _coords(inCoords),
      _triangles(),
      _halfEdges(),
      _hullPrev(),
      _hullNext(),
      _hullTri(),
      _hullStart(),
      _hash(),
      _center(),
      _hashSize(),
      _edgeStack() {
    size_t n = _coords.size() >> 1;

    double maxX = std::numeric_limits<double>::min();
    double maxY = std::numeric_limits<double>::min();
    double maxZ = std::numeric_limits<double>::min();
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double minZ = std::numeric_limits<double>::max();
    std::vector<size_t> ids;
    ids.reserve(n);

    for (size_t i = 0; i < n; i++) {
      const pxr::GfVec3d& coord = _coords[i];
        if (coord[0] < minX) minX = coord[0];
        if (coord[1] < minY) minY = coord[1];
        if (coord[2] < minZ) minZ = coord[2];
        if (coord[0] > maxX) maxX = coord[0];
        if (coord[1] > maxY) maxY = coord[1];
        if (coord[2] > maxZ) maxZ = coord[2];
        ids.push_back(i);
    }
    const double cx = (minX + maxX) / 2;
    const double cy = (minY + maxY) / 2;
    double minDist = std::numeric_limits<double>::max();

    size_t i0 = INVALID_INDEX;
    size_t i1 = INVALID_INDEX;
    size_t i2 = INVALID_INDEX;

    // pick a seed point close to the centroid
    for (size_t i = 0; i < n; ++i) {
        const double d = ComputeDistanceSquared(cx, cy, _coords[2 * i], _coords[2 * i + 1]);
        if (d < min_dist) {
            i0 = i;
            min_dist = d;
        }
    }

    const double i0x = _coords[2 * i0];
    const double i0y = _coords[2 * i0 + 1];

    min_dist = std::numeric_limits<double>::max();

    // find the point closest to the seed
    for (size_t i = 0; i < n; ++i) {
        if (i == i0) continue;
        const double d = ComputeDistanceSquared(i0x, i0y, _coords[2 * i], _coords[2 * i + 1]);
        if (d < min_dist && d > 0.0) {
            i1 = i;
            min_dist = d;
        }
    }

    double i1x = _coords[2 * i1];
    double i1y = _coords[2 * i1 + 1];

    double min_radius = std::numeric_limits<double>::max();

    // find the third point which forms the smallest circumcircle with the first two
    for (size_t i = 0; i < n; i++) {
        if (i == i0 || i == i1) continue;

        const double r = ComputeCircumRadius(
            i0x, i0y, i1x, i1y, _coords[2 * i], _coords[2 * i + 1]);

        if (r < min_radius) {
            i2 = i;
            min_radius = r;
        }
    }

    if (!(min_radius < std::numeric_limits<double>::max())) {
        throw std::runtime_error("not triangulation");
    }

    double i2x = _coords[2 * i2];
    double i2y = _coords[2 * i2 + 1];

    if (_Orient(i0x, i0y, i1x, i1y, i2x, i2y)) {
        std::swap(i1, i2);
        std::swap(i1x, i2x);
        std::swap(i1y, i2y);
    }

    _center = ComputeCircumCenter(i0x, i0y, i1x, i1y, i2x, i2y);

    // sort the points by distance from the seed triangle circumcenter
    std::sort(ids.begin(), ids.end(), Compare{ _coords, _center});

    // initialize a hash table for storing edges of the advancing convex hull
    _hashSize = static_cast<size_t>(std::llround(std::ceil(std::sqrt(n))));
    _hash.resize(_hashSize);
    std::fill(_hash.begin(), _hash.end(), INVALID_INDEX);

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

    _hash[ComputeHash(i0x, i0y)] = i0;
    _hash[ComputeHash(i1x, i1y)] = i1;
    _hash[ComputeHash(i2x, i2y)] = i2;

    size_t maxTriangles = n < 3 ? 1 : 2 * n - 5;
    _triangles.reserve(maxTriangles * 3);
    _halfEdges.reserve(maxTriangles * 3);
    AddTriangle(i0, i1, i2, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);
    pxr::GfVec3d x(std::numeric_limits<double>::quiet_NaN());

    for (size_t k = 0; k < n; k++) {
        const size_t i = ids[k];
        const double x = _coords[2 * i];
        const double y = _coords[2 * i + 1];

        // skip near-duplicate points
        if (k > 0 && CheckPointsEquals(x, xp, yp)) continue;
        xp = x;
        yp = y;

        // skip seed triangle points
        if (
            _CheckPointsEquals(x, y, i0x, i0y) ||
            _CheckPointsEquals(x, y, i1x, i1y) ||
            _CheckPointsEquals(x, y, i2x, i2y)) continue;

        // find a visible edge on the convex hull using edge hash
        size_t start = 0;

        size_t key = ComputeHash(x, y);
        for (size_t j = 0; j < _hashSize; j++) {
            start = _hash[_FastMod(key + j, _hashSize)];
            if (start != INVALID_INDEX && start != _hullNext[start]) break;
        }

        start = _hullPrev[start];
        size_t e = start;
        size_t q;

        while (q = _hullNext[e], !_Orient(x, y, _coords[2 * e], _coords[2 * e + 1], _coords[2 * q], _coords[2 * q + 1])) {
            e = q;
            if (e == start) {
                e = INVALID_INDEX;
                break;
            }
        }

        if (e == INVALID_INDEX) continue; // likely a near-duplicate point; skip it

        // add the first triangle from the point
        size_t t = AddTriangle(
            e,
            i,
            _hullNext[e],
            INVALID_INDEX,
            INVALID_INDEX,
            _hullTri[e]);

        _hullTri[i] = Legalize(t + 2);
        _hullTri[e] = t;
        hullSize++;

        // walk forward through the hull, adding more triangles and flipping recursively
        size_t next = _hullNext[e];
        while (
            q = _hullNext[next],
            _Orient(x, y, _coords[2 * next], _coords[2 * next + 1], _coords[2 * q], _coords[2 * q + 1])) {
            t = AddTriangle(next, i, q, _hullTri[i], INVALID_INDEX, _hullTri[next]);
            _hullTri[i] = Legalize(t + 2);
            _hullNext[next] = next; // mark as removed
            hullSize--;
            next = q;
        }

        // walk backward from the other side, adding more triangles and flipping
        if (e == start) {
            while (
                q = _hullPrev[e],
                _Orient(x, y, _coords[2 * q], _coords[2 * q + 1], _coords[2 * e], _coords[2 * e + 1])) {
                t = AddTriangle(q, i, e, INVALID_INDEX, _hullTri[e], _hullTri[q]);
                Legalize(t + 2);
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

        _hash[ComputeHash(x, y)] = i;
        _hash[ComputeHash(_coords[2 * e], _coords[2 * e + 1])] = e;
    }
}

double DelaunayTriangulation::GetHullArea() {
    std::vector<double> hullArea;
    size_t e = _hullStart;
    do {
        hullArea.push_back((_coords[2 * e] - _coords[2 * _hullPrev[e]]) * (_coords[2 * e + 1] + _coords[2 * _hullPrev[e] + 1]));
        e = _hullNext[e];
    } while (e != _hullStart);
    return _Sum(hullArea);
}

size_t DelaunayTriangulation::Legalize(size_t a) {
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

        if (b == INVALID_INDEX) {
            if (i > 0) {
                i--;
                a = _edgeStack[i];
                continue;
            } else {
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
            _coords[2 * p0],
            _coords[2 * p0 + 1],
            _coords[2 * pr],
            _coords[2 * pr + 1],
            _coords[2 * pl],
            _coords[2 * pl + 1],
            _coords[2 * p1],
            _coords[2 * p1 + 1]);

        if (illegal) {
            _triangles[a] = p1;
            _triangles[b] = p0;

            auto hbl = _halfEdges[bl];

            // edge swapped on the other side of the hull (rare); fix the halfedge reference
            if (hbl == INVALID_INDEX) {
                size_t e = _hullStart;
                do {
                    if (_hullTri[e] == bl) {
                        _hullTri[e] = a;
                        break;
                    }
                    e = _hullNext[e];
                } while (e != _hullStart);
            }
            Link(a, hbl);
            Link(b, _halfEdges[ar]);
            Link(ar, bl);
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

inline size_t DelaunayTriangulation::ComputeHash(const double x, const double y) const 
{
  const double dx = x - _centerX;
  const double dy = y - _centerY;
  return _FastMod(
    static_cast<size_t>(std::llround(std::floor(_GetPseudoAngle(dx, dy) * static_cast<double>(_hashSize)))),
      _hashSize);
}

size_t DelaunayTriangulation::AddTriangle(
  size_t i0, size_t i1, size_t i2, size_t a, size_t b, size_t c) 
{
  size_t t = _triangles.size();
  _triangles.push_back(i0);
  _triangles.push_back(i1);
  _triangles.push_back(i2);
  Link(t, a);
  Link(t + 1, b);
  Link(t + 2, c);
  return t;
}

void DelaunayTriangulation::Link(const size_t a, const size_t b) {
  size_t s = _halfEdges.size();
  if (a == s) {
    _halfEdges.push_back(b);
  } else if (a < s) {
    _halfEdges[a] = b;
  } else {
    throw std::runtime_error("Cannot link edge");
  }
  if (b != INVALID_INDEX) {
    size_t s2 = _halfEdges.size();
    if (b == s2) {
      _halfEdges.push_back(a);
    } else if (b < s2) {
      _halfEdges[b] = a;
    } else {
      throw std::runtime_error("Cannot link edge");
    }
  }
}

JVR_NAMESPACE_OPEN_SCOPE

#endif // JVR_GEOMETRY_DELAUNAY_H
