#ifndef JVR_GEOMETRY_SUBDIV_H
#define JVR_GEOMETRY_SUBDIV_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mesh;
//------------------------------------------------------------------------------
// Vertex container implementation.
//
struct _Vertex {

  // Minimal required interface ----------------------
  _Vertex() { }

  _Vertex(_Vertex const & src) {
      _position[0] = src._position[0];
      _position[1] = src._position[1];
      _position[2] = src._position[2];
  }

  void Clear( void * =0 ) {
      _position[0]=_position[1]=_position[2]=0.0f;
  }

  void AddWithWeight(_Vertex const & src, float weight) {
      _position[0]+=weight*src._position[0];
      _position[1]+=weight*src._position[1];
      _position[2]+=weight*src._position[2];
  }

  // Public interface ------------------------------------
  void SetPosition(float x, float y, float z) {
      _position[0]=x;
      _position[1]=y;
      _position[2]=z;
  }

  const float * GetPosition() const {
      return _position;
  }

private:
    float _position[3];
};

void _SubdivideMesh(Mesh* mesh, int refineLevel);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_SUBDIV_H
