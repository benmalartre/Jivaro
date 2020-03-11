#pragma once

#include "base.h"
#include "shader.h"
#include "ttf.h"

AMN_NAMESPACE_OPEN_SCOPE

using namespace Utility::TTF;

struct GLUITextPoint : public GLUIAtom {
  short                       _t;             // texture coordinates
  short                       _c;             // coefficient
};

struct GLUICharacter : public GLUIAtom {
  std::vector<GLUITextPoint>  _X;             // points
  float                       _W;             // width
  float                       _H;             // height

  void operator *=(float s) {
    for(auto x : _X)x._P *= s;
  };
  void operator +=(const pxr::GfVec2f& p) {
    for(auto x : _X)x._P += p;
  };
};

struct GLUIString : public GLUIAtom {
  std::vector<GLUICharacter>      _C;         // characters  
  std::string                     _V;         // string
  float                           _S;         // size
};

static  std::vector<GLUICharacter> ALPHABET;  // alphabet

// Space
//------------------------------------------------------------------------------
static GLUICharacter BuildGLUICharacter(const Utility::TTF::Mesh& mesh, const pxr::GfVec2f& o) {
  GLUICharacter c;
  
  c._P = pxr::GfVec2f(mesh.verts[0].pos.x, mesh.verts[0].pos.y);     // position
  c._X.resize(mesh.verts.size());                                    // points
  
  pxr::GfVec2f bbmin = pxr::GfVec2f(1000000.f, 1000000.f);
  pxr::GfVec2f bbmax = pxr::GfVec2f(-1000000.f, -1000000.f);
  for (int j = 0; j < mesh.verts.size(); j++)
  {
    const MeshVertex &mv = mesh.verts[j];
    c._X[j]._P =  pxr::GfVec2f(mv.pos.x/2500.f, mv.pos.y/2500.f) + o;// point position
    c._X[j]._t = mv.texCoord;                                        // texture coordinate
    c._X[j]._c = mv.coef;                                            // coefficient
    if(c._X[j]._P[0]<bbmin[0])bbmin[0] = c._X[j]._P[0];              // bbox min x
    else if(c._X[j]._P[0]>bbmax[0])bbmax[0] = c._X[j]._P[0];         // bbox max x
    if(c._X[j]._P[1]<bbmin[1])bbmin[1] = c._X[j]._P[1];              // bbox min y
    else if(c._X[j]._P[1]>bbmax[1])bbmax[1] = c._X[j]._P[1];         // bbox max y
  }
  c._W = bbmax[0]-bbmin[0];                                          // width
  c._H = bbmax[1]-bbmin[1];                                          // height

  return c;
}



// build alphabet
//------------------------------------------------------------------------------
static void GLUIBuildAlphabet()
{
  ALPHABET.resize(255);

  Font f("/Users/benmalartre/Documents/RnD/amnesie/fonts/roboto/Roboto-Regular.ttf");
  f.PreCacheBasicLatin();
  for(int i=0; i< 256; ++i)
  {
    CodePoint cp(i);
    const Mesh &m = f.GetTriangulation(cp);
    if(m.verts.size())ALPHABET[i] = BuildGLUICharacter(m, pxr::GfVec2f((i-65)* 0.5f-1.f, 0));
  }
  
}

// parse string
//------------------------------------------------------------------------------
static GLUIString ParseString(const std::string& s)
{
  GLUIString result;
  result._S = 0.01f;
  result._C.resize(s.length());
  result._V = s;
  int idx = 0;
  float scale = 0.01f;
  pxr::GfVec2f p;
  for(auto c : s)
  {
    
    if((int)c == 10 || (int)c == 13) 
    {
      p[1] += ALPHABET[(int)c]._H * scale;
      p[0] = 0;
    }
      
    else
    {
      result._C[idx] = ALPHABET[(int)c];
      result._C[idx] *= scale;
      result._C[idx++] += p;
      p[0] += ALPHABET[(int)c]._W * scale;
    }
  }

  return result;
}

// build buffer
//------------------------------------------------------------------------------
static GLUIBuffer GLUIBuildBuffer(const std::vector<GLUITextPoint>& points)
{
  GLUIBuffer buffer;
  GLuint program = GetTTFShaderSimpleProgram();
  size_t numPoints = points.size();
  size_t sz = numPoints * sizeof(GLUITextPoint);

  GLCheckError("PRE BUILD BUFFER");
  // generate vertex array object
  glGenVertexArrays(1, &buffer._vao);
  glBindVertexArray(buffer._vao);
  GLCheckError("GEN VAO");

  // generate vertex buffer object
  glGenBuffers(1, &buffer._vbo);
  glBindBuffer(GL_ARRAY_BUFFER, buffer._vbo);
  GLCheckError("GEN VBO");

  buffer._indexed = false;

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,sz, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, (void*)&points[0]._P[0]);
  GLCheckError("SET DATA");
  //delete [] datas;

  GLint loc = glGetAttribLocation(program, "t");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    1, 
    GL_BYTE, 
    GL_FALSE, 
    sizeof(GLUITextPoint), 
    (void*)sizeof(pxr::GfVec2f)
  );
  GLCheckError("VERTEX ATTRIBUTE POINTER : t");

  loc = glGetAttribLocation(program, "c");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    1, 
    GL_BYTE, 
    GL_FALSE, 
    sizeof(GLUITextPoint), 
    (void*)(sizeof(pxr::GfVec2f) + sizeof(short))
  );
  GLCheckError("VERTEX ATTRIBUTE POINTER : c");

  loc = glGetAttribLocation(program, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    2, 
    GL_FLOAT, 
    GL_FALSE, 
    sizeof(GLUITextPoint), 
    0
  );
  GLCheckError("VERTEX ATTRIBUTE POINTER : pos");

  glLinkProgram(GetTTFShaderSimpleProgram());
  GLCheckError("LINK PROGRAM");  
  
  // unbind vertex array object
  glBindVertexArray(0);

  return buffer;
}

// draw buffer
//------------------------------------------------------------------------------
static void GLUIDrawBuffer(GLUIBuffer* buffer, const int N)
{
  glUseProgram(GetTTFShaderSimpleProgram());
  glBindVertexArray(buffer->_vao);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES,0, N);

  /*
  glEnable(GL_POINT_SIZE);
  glPointSize(4);
  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  glDrawArrays(GL_TRIANGLES,0, N);
  glDisable(GL_POINT_SIZE);
  */

  glBindVertexArray(0);
}

// draw string
//------------------------------------------------------------------------------
static void GLUIDrawString(GLUIString s)
{

  int baseP = 0, N = 0;

  std::vector<GLUITextPoint> points;
  for(auto c: s._C)
  {
    points.resize(points.size() + c._X.size());
    N += c._X.size();
    for(auto p: c._X)
    {
      points[baseP++] = p;
    }
  }
  GLUIBuffer buffer = GLUIBuildBuffer(points);
  GLUIDrawBuffer(&buffer, N);
}



static void GLUITest()
{
  GLUIString s = ParseString("ABC");
  GLUIDrawString( s);
/*
  GLUICharacter a = ALPHABET[65];
  //GLUICircle c = MakeCircle(20,20,32, 32);

  size_t sz = 12 * sizeof(float);
  // generate vertex array object
  glGenVertexArrays(1, &SCREENSPACEQUAD_VAO);
  glBindVertexArray(SCREENSPACEQUAD_VAO);

  // generate vertex buffer object
  glGenBuffers(1, &SCREENSPACEQUAD_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, SCREENSPACEQUAD_VBO);

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,2 * sz, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, &SCREENSPACEQUAD_POINTS[0]);
  glBufferSubData(GL_ARRAY_BUFFER, sz, sz, &SCREENSPACEQUAD_UVS[0]);

  // attibute position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
  
  // attibute uvs
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void*)sz);

  // bind shader program
  glBindAttribLocation(SCREENSPACEQUAD_PROGRAM_SHADER, 0, "position");
  glBindAttribLocation(SCREENSPACEQUAD_PROGRAM_SHADER, 1, "uvs");
  
  glLinkProgram(SCREENSPACEQUAD_PROGRAM_SHADER);

  // unbind vertex array object
  glBindVertexArray(0);
  */
}

static void GLUIInit()
{
  GLUIBuildTextShader();
  GLUIBuildAlphabet();
}


AMN_NAMESPACE_CLOSE_SCOPE