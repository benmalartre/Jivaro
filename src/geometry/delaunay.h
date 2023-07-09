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

#include <pxr/base/gf/vec2d.h>

#include "../geometry/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

class Delaunay {
  static inline size_t _FastMod(const size_t i, const size_t c) {
    return i >= c ? i % c : i;
  }

  // Kahan and Babuska summation, Neumaier variant; accumulates less FP error
  static inline double _Sum(const std::vector<double>& x) {
    double sum = x[0];
    double err = 0.0;

    for (size_t i = 1; i < x.size(); i++) {
      const double k = x[i];
      const double m = sum + k;
      err += std::fabs(sum) >= std::fabs(k) ? sum - m + k : k - m + sum;
      sum = m;
    }
    return sum + err;
  }

  static inline double _Dist(
    const pxr::GfVec2d& a,
    const pxr::GfVec2d& b) {
    return (b - a).GetLengthSq();
  }

  static inline double _CircumRadius(
    const pxr::GfVec2d& a, const pxr::GfVec2d& b, const pxr::GfVec2d& c) {

    const pxr::GfVec2d d(b - a);
    const pxr::GfVec2d e(c - a);

    const double bl = d.GetLengthSq();
    const double cl = e.GetLengthSq();
    const double f = d[0] * e[1] - d[1] * e[0];

    const double x = (e[1] * bl - d[1] * cl) * 0.5 / f;
    const double y = (d[0] * cl - e[0] * bl) * 0.5 / f;

    if ((bl > 0.0 || bl < 0.0) && (cl > 0.0 || cl < 0.0) && (f > 0.0 || f < 0.0)) {
      return x * x + y * y;
    }
    else {
      return std::numeric_limits<double>::max();
    }
  }

  static inline bool _Orient(const pxr::GfVec2d& p,const  pxr::GfVec2d& q, const pxr::GfVec2d& r) {
    return (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1]) < 0.0;
  }

  static inline pxr::GfVec2d _CircumCenter(
    const pxr::GfVec2d& a, const pxr::GfVec2d& b, const pxr::GfVec2d& c) {
    const pxr::GfVec2d d(b - a);
    const pxr::GfVec2d e(c - a);

    const double bl = d.GetLengthSq();
    const double cl = e.GetLengthSq();
    const double f = d[0] * e[1] - d[1] * e[0];

    return a + pxr::GfVec2d(
      (e[1] * bl - d[1] * cl) * 0.5 / f,
      (d[0] * cl - e[0] * bl) * 0.5 / f
    );
  }

  static inline bool _InCircle(
    const pxr::GfVec2d& a, const pxr::GfVec2d& b, const pxr::GfVec2d& c,
    const pxr::GfVec2d& p) {
    const pxr::GfVec2d d(a - p);
    const pxr::GfVec2d e(b - p);
    const pxr::GfVec2d f(c - p);

    const double ap = d.GetLengthSq();
    const double bp = e.GetLengthSq();
    const double cp = f.GetLengthSq();

    return (d[0] * (e[1] * cp - bp * f[1]) -
      d[1] * (e[0] * cp - bp * f[0]) +
      ap * (e[0] * f[1] - e[1] * f[0])) < 0.0;
  }

  static inline bool _CheckPointEquality(const pxr::GfVec2d& a, const pxr::GfVec2d& b) {
    return pxr::GfIsClose(a, b, GEOM_EPSILON);
  }

  // monotonically increases with real angle, but doesn't need expensive trigonometry
  static inline double _PseudoAngle(const pxr::GfVec2d& v) {
    const double p = v[0] / (std::abs(v[0]) + std::abs(v[1]));
    return (v[1] > 0.0 ? 3.0 - p : 1.0 + p) / 4.0; // [0..1)
  }

protected:
  struct Compare {

    std::vector<pxr::GfVec2d> const& coords;
    pxr::GfVec2d coord;

    bool operator()(std::size_t i, std::size_t j) {
      const double d1 = _Dist(coords[i], coord);
      const double d2 = _Dist(coords[j], coord);
      const double diff1 = d1 - d2;
      const double diff2 = coords[i][0] - coords[j][0];
      const double diff3 = coords[i][1] - coords[j][1];

      if (diff1 > 0.0 || diff1 < 0.0) {
        return diff1 < 0;
      }
      else if (diff2 > 0.0 || diff2 < 0.0) {
        return diff2 < 0;
      }
      else {
        return diff3 < 0;
      }
    }
  };

  size_t _Legalize(size_t a);
  size_t _ComputeHash(const pxr::GfVec2d& coord) const;
  size_t _AddTriangle(size_t i0, size_t i1, size_t i2,
    size_t a, size_t b, size_t c);
  void _Link(size_t a, size_t b);

public:
  std::vector<pxr::GfVec2d>         _coords;
  std::vector<size_t>               _triangles;
  std::vector<size_t>               _halfEdges;
  std::vector<size_t>               _hullPrev;
  std::vector<size_t>               _hullNext;
  std::vector<size_t>               _hullTri;
  size_t                            _hullStart;

  Delaunay(const std::vector<pxr::GfVec2d>& inCoords);

  double GetHullArea();

private:
  std::vector<size_t>               _hash;
  pxr::GfVec2d                      _center;
  size_t                            _hashSize;
  std::vector<size_t>               _edgeStack;
};

 JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_DELAUNAY_H
