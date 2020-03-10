#pragma once

#include "../default.h"
#include "../utils/utils.h"
#include "../utils/glutils.h"
#include "shader.h"

AMN_NAMESPACE_OPEN_SCOPE

enum GLUIAtomType {
  POINT,
  LINE,
  TRIANGLE,
  SQUARE,
  CIRCLE
};

struct GLUIAtom {
  pxr::GfVec2f          _P;       // position     
};

struct GLUIPoint : public GLUIAtom {
  pxr::GfVec2f          _UV;      // uv
  pxr::GfVec3f          _C;       // color
  int                   _D;       // depth
  float                 _T;       // thickness
};

struct GLUILine : public GLUIAtom {
  std::vector<GLUIPoint>  _X;     // points
};

struct GLUITriangle : public GLUIAtom {
  GLUIPoint               _A;     // point A
  GLUIPoint               _B;     // point B
  GLUIPoint               _C;     // point C
};

struct GLUISquare : public GLUIAtom {
  GLUIPoint               _A;     // point A
  GLUIPoint               _B;     // point B
  GLUIPoint               _C;     // point C
  GLUIPoint               _D;     // point C
};

struct GLUIRoundSquare : public GLUIAtom {
  std::vector<GLUIPoint>  _X;     // points
  std::vector<int>        _I;     // indices
};

struct GLUICircle : public GLUIAtom {
  std::vector<GLUIPoint>  _X;     // points
  std::vector<int>        _I;     // indices
};

class GLUIItem : public GLUIAtom {
public:
  virtual void Init();
  virtual void Term();
  virtual void Draw();

private:

  std::vector<GLUIItem*>          _childrens;
  std::vector<pxr::GfVec2f>       _points;

};

struct GLUIBuffer {
  GLuint                  _vao;
  GLuint                  _vbo;
  GLuint                  _eab;
  int                     _N;
  bool                    _indexed;
};

// make point
//------------------------------------------------------------------------------
static GLUIPoint MakePoint(float x, float y)
{
  GLUIPoint P;
  P._P = pxr::GfVec2f(x, y);           // position
  P._UV = pxr::GfVec2f();              // uv
  P._C = pxr::GfVec3f(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1
  );               // color
  P._D = 0;                            // depth     
  return P;
}

// start a new line
//------------------------------------------------------------------------------
static GLUILine MakeLine(float x, float y)
{
  GLUIPoint P = MakePoint(x, y);
  GLUILine L;
  L._X.push_back(P);
  return L;
}

// make a two points line
//------------------------------------------------------------------------------
static GLUILine MakeLine(float x1, float y1, float x2, float y2)
{
  GLUIPoint A = MakePoint(x1, y1);
  GLUIPoint B = MakePoint(x2, y2);
  GLUILine L;
  L._X.push_back(A);
  L._X.push_back(B);
  return L;
}

// add point to line
//------------------------------------------------------------------------------
static void AddPoint(GLUILine* line, float x, float y)
{
  GLUIPoint P = MakePoint(x, y);
  line->_X.push_back(P);
}

// add arc to line
//------------------------------------------------------------------------------
static void AddArc(GLUILine* line, float radius, pxr::GfVec2f center, 
  float startAngle, float endAngle, int segments)
{
  int base = line->_X.size();
  int last = base + segments;
  line->_X.resize(base+segments);

  float step = (endAngle - startAngle)/(float)segments;
  for(int i = base; i < last; ++i)
  {
    line->_X[i]._P = 
      pxr::GfVec2f(sin(startAngle + i * step), cos(startAngle + i * step));
    line->_X[i]._P *= radius;
    line->_X[i]._P += center;
  }
}

// add arc to line
//------------------------------------------------------------------------------
static GLUIRoundSquare MakeRoundSquare(float x, float y, float width, float height, 
  float radius, int segments)
{
  return GLUIRoundSquare();
}

// make circle
//------------------------------------------------------------------------------
static GLUICircle MakeCircle(float x, float y, float radius, int segments, float start=0)
{
  int numVertices = segments + 1;
  GLUICircle circle;
  circle._P = pxr::GfVec2f(x, y);
  circle._X.resize(numVertices);
  circle._I.resize(numVertices * 3);
  circle._X[0]._P = pxr::GfVec2f();
  float step = 360/(float)(segments);

  for(int i = 0; i < segments; ++i)
  {
    circle._X[i+1]._P = 
      pxr::GfVec2f(sin(start + i * step), cos(start + i * step));
    circle._X[i+1]._P *= radius;
    if(i<segments-1)
    {
      circle._I[i*3] = 0;
      circle._I[i*3+1] = i;
      circle._I[i*3+2] = i+1;
    }
    else
    {
      circle._I[i*3] = 0;
      circle._I[i*3+1] = i;
      circle._I[i*3+2] = 1;
    }
  }
  return circle;
}

// concatenate vector
//------------------------------------------------------------------------------
template<typename T>
static T ConcatenateVector(T dst, T src, bool reindex=false)
{
  size_t numSrc = src.size();
  size_t numDst = dst.size();
  dst.resize(numDst + numSrc);
  size_t idx = numDst;
  if(reindex)
    for(auto x : src) dst[idx++] = x + numDst;
  else
    for(auto x : src) dst[idx++] = x;
}

// build buffer
//------------------------------------------------------------------------------
static GLUIBuffer BuildBuffer(const std::vector<GLUIPoint>& points,
  const std::vector<int>& indices)
{
  GLUIBuffer buffer;
  size_t numPoints = points.size();
  size_t sz = numPoints * sizeof(pxr::GfVec4f);

  GLCheckError("PRE BUILD BUFFER");
  // generate vertex array object
  glGenVertexArrays(1, &buffer._vao);
  glBindVertexArray(buffer._vao);
  GLCheckError("GEN VAO");

  // generate vertex buffer object
  glGenBuffers(1, &buffer._vbo);
  glBindBuffer(GL_ARRAY_BUFFER, buffer._vbo);
  GLCheckError("GEN VBO");
/*
  if(indices.size())
  {
    // generate element array buffer
    glGenBuffers(1, &buffer._eab);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer._eab);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                indices.size() * sizeof(int),
                &indices[0], 
                GL_STATIC_DRAW);
    buffer._indexed = true;
  }
  else */
  buffer._indexed = false;
  
  // pack datas
  float* datas = new float[4 * numPoints];
  int j = 0;
  for(int i=0; i < numPoints; ++i)
  {
    datas[j++] = points[i]._P[0];                 // position X
    datas[j++] = points[i]._P[1];                 // position Y
    datas[j++] = 0.1f;//points[i]._T;            // thickness
    datas[j++] = PackColorAsFloat(points[i]._C);  // color (packed)
  }

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,sz, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, (void*)datas);
  GLCheckError("SET DATA");
  delete [] datas;

  // attibute position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  GLCheckError("ATTRIB ENABLED");

  // bind shader program
  glBindAttribLocation(GetTextShaderProgram(), 0, "datas");
  GLCheckError("BIND ATTRIBUTE LOCATION");
  glLinkProgram(GetTextShaderProgram());
  GLCheckError("LINK PROGRAM");  
  
  glLinkProgram(GetFillShaderProgram());

  // unbind vertex array object
  glBindVertexArray(0);

  return buffer;
}

// draw buffer
//------------------------------------------------------------------------------
static void DrawBuffer(GLUIBuffer buffer, std::vector<int> indices)
{
  glUseProgram(GetTextShaderProgram());
  glBindVertexArray(buffer._vao);

  int base = 0;
  for(auto i: indices)
  {
    std::cout << base << ":" << i-base << std::endl;
    glDrawArrays(GL_LINES_ADJACENCY,base,i-base);
    base = i;
  }

  glBindVertexArray(0);
}

AMN_NAMESPACE_CLOSE_SCOPE