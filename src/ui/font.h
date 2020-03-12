#pragma once

#include "base.h"
#include "shader.h"
#include "ttf.h"

AMN_NAMESPACE_OPEN_SCOPE

static std::string FONT_NAME="/Users/benmalartre/Documents/RnD/amnesie/fonts/baloo/Baloo-Regular.ttf";

struct GLUITextBuffer {
  GLuint                  _vao;
  GLuint                  _vbo;
  int                     _N;

  GLUITextBuffer():_vao(0),_vbo(0){};
  ~GLUITextBuffer()
  {
    if(_vbo)glDeleteBuffers(1,&_vbo);
    if(_vao)glDeleteVertexArrays(1,&_vao);
  }
};

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
  GLUITextBuffer*                 _buffer;    // opengl buffer

  GLUIString(const std::string& s, ):_buffer(NULL){};
  ~GLUIString(){ if(_buffer)delete _buffer;};
};

static  std::vector<GLUICharacter> ALPHABET;  // alphabet

// Space
//------------------------------------------------------------------------------
static GLUICharacter BuildGLUICharacter(const Utility::TTF::Mesh& mesh) {
  GLUICharacter c;
  
  c._P = pxr::GfVec2f(mesh.verts[0].pos.x, mesh.verts[0].pos.y);     // position
  c._X.resize(mesh.verts.size());                                    // points
  
  pxr::GfVec2f bbmin = pxr::GfVec2f(1000000.f, 1000000.f);
  pxr::GfVec2f bbmax = pxr::GfVec2f(-1000000.f, -1000000.f);
  for (int j = 0; j < mesh.verts.size(); j++)
  {
    const Utility::TTFCore::MeshVertex &mv = mesh.verts[j];
    c._X[j]._P =  pxr::GfVec2f(mv.pos.x, mv.pos.y);// point position
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

  Utility::TTFCore::Font f(FONT_NAME);
  f.PreCacheBasicLatin();
  for(int i=0; i< 256; ++i)
  {
    Utility::TTFCore::CodePoint cp(i);
    const Utility::TTFCore::Mesh &m = f.GetTriangulation(cp);
    if(m.verts.size())ALPHABET[i] = BuildGLUICharacter(m);
  }
  
}

// parse string
//------------------------------------------------------------------------------
static void ParseString(
  const Utility::TTFCore::Font& f, 
  GLUIString* s, 
  float scale,
  const Utility::TTFCore::vec2f& offset
)
{
  Utility::TTFCore::FontMetrics fontMetrics = f.GetFontMetrics();
  Utility::TTFCore::HeadTable headTable;
  s->_S = scale;
  s->_C.resize(s->_V.length());
  int idx = 0;
  Utility::TTFCore::vec2f p = offset;
  Utility::TTFCore::vec2f kerning(0,0);
  char l;
  for(auto c : s->_V)
  {
    if(idx > 0)
      kerning = f.GetKerning(Utility::TTFCore::CodePoint(l), 
                              Utility::TTFCore::CodePoint(c));


    if((int)c == 10 || (int)c == 13) 
    {
      p.y -= fontMetrics.ascent * scale;
      p.x = offset.x;
    }
      
    else
    {
      if((int)l != 10 && (int)l != 13)
      {
        p.x += kerning.x * scale;
        p.y += kerning.y * scale;
      }

      s->_C[idx] = ALPHABET[(int)c];
      for(auto& x: s->_C[idx]._X)
      {
        x._P *= scale; 
        x._P += pxr::GfVec2f(p.x, p.y);
      };
    }
    idx++;
    l = c;
  }
}

// build buffer
//------------------------------------------------------------------------------
static GLUITextBuffer* GLUIBuildTextBuffer(const std::vector<GLUITextPoint>& points)
{
  GLUITextBuffer* buffer = new GLUITextBuffer();
  GLuint program = GetTTFShaderSimpleProgram();
  size_t numPoints = points.size();
  size_t sz = numPoints * sizeof(GLUITextPoint);

  GLCheckError("PRE BUILD BUFFER");
  // generate vertex array object
  glGenVertexArrays(1, &buffer->_vao);
  glBindVertexArray(buffer->_vao);
  GLCheckError("GEN VAO");

  // generate vertex buffer object
  glGenBuffers(1, &buffer->_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, buffer->_vbo);
  GLCheckError("GEN VBO");

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
static void GLUIDrawTextBuffer(GLUITextBuffer* buffer, const int N)
{
  GLuint program = GetTTFShaderSimpleProgram();
  glUseProgram(program);

  glBindVertexArray(buffer->_vao);
  glUniform3f(glGetUniformLocation(program, "color"), RANDOM_0_1, RANDOM_0_1, RANDOM_0_1);
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
static void GLUIConcatenateString(GLUIString* s)
{

  int baseP = 0, N = 0;

  std::vector<GLUITextPoint> points;
  for(auto c: s->_C)
  {
    points.resize(points.size() + c._X.size());
    N += c._X.size();
    for(auto p: c._X)
    {
      points[baseP++] = p;
    }
  }
  s->_buffer = GLUIBuildTextBuffer(points);
  s->_buffer->_N = N;
  //GLUIDrawTextBuffer(s->_buffer, s->_buffer->_N);
}

static void GLUIDrawString(GLUIString* s)
{
  GLUIDrawTextBuffer(s->_buffer, s->_buffer->_N);
}

static void GLUIInitTest(
  std::vector<GLUIString*>& strings,
  int x, 
  int y, 
  int width, 
  int height, 
  int size
)
{
  Utility::TTFCore::Font f(FONT_NAME);
  f.PreCacheBasicLatin();

  strings.resize(2);
  strings[0]= new GLUIString();
  strings[0]->_V = "ABC\nDEF\nGHI\nJKL";
  Utility::TTFCore::vec2f offset(-0.5, 0.5);
  ParseString(f, strings[0], 0.0001f, offset);
  GLUIConcatenateString(strings[0]);
  GLUIDrawString(strings[0]);

  offset.y -= 0.5;
  strings[1]= new GLUIString();
  strings[1]->_V = "MNOPQR\nSTUVWXYZ";
  ParseString(f, strings[1], 0.0001f, offset);
  GLUIConcatenateString(strings[1]);
  GLUIDrawString(strings[1]);
}

static void GLUIDrawTest(const std::vector<GLUIString*>& strings)
{
  
  for(auto s: strings)
  {
    GLUIDrawString(s);
  }

}


static void GLUIInit()
{
  GLUIBuildTextShader();
  GLUIBuildAlphabet();
}


AMN_NAMESPACE_CLOSE_SCOPE