#pragma once

#include "base.h"
#include "shader.h"
#include "ttf.h"

AMN_NAMESPACE_OPEN_SCOPE

static std::string FONT_NAME = "/Users/benmalartre/Documents/RnD/amnesie/fonts/roboto/Roboto-Regular.ttf";

struct GLUIStringBuffer {
  GLuint                  _vao;
  GLuint                  _vbo;
  int                     _N;
};

struct GLUIStringPoint : public GLUIAtom {
  short                       _t;             // texture coordinates
  short                       _c;             // coefficient
};

struct GLUICharacter : public GLUIAtom {
  std::vector<GLUIStringPoint>  _X;             // points
};

struct GLUIString : public GLUIAtom {
  std::vector<GLUICharacter>      _C;         // characters  
  std::string                     _V;         // string
  float                           _S;         // size
  GLUIStringBuffer                _buffer;    // opengl buffer
};

static  std::vector<GLUICharacter> ALPHABET;  // alphabet

// point size to pixel size
//------------------------------------------------------------------------------
static Utility::TTFCore::vec2f PointToPixels(
  const Utility::TTFCore::vec2f& pointSize,
  const Utility::TTFCore::vec2f& resolution
)
{
  Utility::TTFCore::vec2f pixelSize;
  pixelSize.x = pointSize.x * (resolution.x / 72);
  pixelSize.y = pointSize.y * (resolution.y / 72);
  return pixelSize;
}


// build one character
//------------------------------------------------------------------------------
static GLUICharacter GLUIBuildCharacter(const Utility::TTF::Mesh& mesh) {
  GLUICharacter c;
  
  c._P = pxr::GfVec2f(mesh.verts[0].pos.x, mesh.verts[0].pos.y);     // position
  c._X.resize(mesh.verts.size());                                    // points
  
  for (int j = 0; j < mesh.verts.size(); j++)
  {
    const Utility::TTFCore::MeshVertex &mv = mesh.verts[j];
    c._X[j]._P =  pxr::GfVec2f(mv.pos.x, mv.pos.y);                  // point position
    c._X[j]._t = mv.texCoord;                                        // texture coordinate
    c._X[j]._c = mv.coef;                                            // coefficient
  }

  return c;
}

// build alphabet
//------------------------------------------------------------------------------
static void GLUIBuildAlphabet()
{
  ALPHABET.resize(256);
  Utility::TTFCore::Font f(FONT_NAME);
  f.PreCacheBasicLatin();
  for(int i=0; i< 256; ++i)
  {
    Utility::TTFCore::CodePoint cp(i);
    const Utility::TTFCore::Mesh &m = f.GetTriangulation(cp);
    if(m.verts.size())ALPHABET[i] = GLUIBuildCharacter(m);
  }
}

// delete alphabet
//------------------------------------------------------------------------------
static void GLUIDeleteAlphabet()
{
  ALPHABET.clear();
}

// parse string
//------------------------------------------------------------------------------
static void GLUIParseString(
  const Utility::TTFCore::Font& f, 
  GLUIString* s, 
  int h,
  const Utility::TTFCore::vec2f& offset
)
{
  Utility::TTFCore::FontMetrics fm = f.GetFontMetrics();
  float height = fm.ascent - fm.descent;
  s->_S = h/height;
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
      p.y -= (fm.ascent - fm.descent + fm.lineGap) * s->_S;
      p.x = offset.x;
    }
      
    else
    {
      if((int)l != 10 && (int)l != 13)
      {
        p.x += kerning.x * s->_S;
        p.y += kerning.y * s->_S;
      }

      s->_C[idx] = ALPHABET[(int)c];
      for(auto& x: s->_C[idx]._X)
      {
        x._P *= s->_S; 
        x._P += pxr::GfVec2f(p.x, p.y);
      };
    }
    idx++;
    l = c;
  }
}

// build buffer
//------------------------------------------------------------------------------
static 
void GLUIBuildStringBuffer(
  GLUIStringBuffer* buffer,
  const std::vector<GLUIStringPoint>& points
)
{
  
  GLuint program = GetTTFShaderProgram();
  size_t numPoints = points.size();
  size_t sz = numPoints * sizeof(GLUIStringPoint);

  // generate vertex array object
  glGenVertexArrays(1, &buffer->_vao);
  glBindVertexArray(buffer->_vao);

  // generate vertex buffer object
  glGenBuffers(1, &buffer->_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, buffer->_vbo);

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,sz, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, (void*)&points[0]._P[0]);

  GLint loc = glGetAttribLocation(program, "t");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    1, 
    GL_BYTE, 
    GL_FALSE, 
    sizeof(GLUIStringPoint), 
    (void*)sizeof(pxr::GfVec2f)
  );

  loc = glGetAttribLocation(program, "c");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    1, 
    GL_BYTE, 
    GL_FALSE, 
    sizeof(GLUIStringPoint), 
    (void*)(sizeof(pxr::GfVec2f) + sizeof(short))
  );

  loc = glGetAttribLocation(program, "pos");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(
    loc, 
    2, 
    GL_FLOAT, 
    GL_FALSE, 
    sizeof(GLUIStringPoint), 
    0
  );

  glLinkProgram(GetTTFShaderProgram());
  glBindVertexArray(0);
}

// draw buffer
//------------------------------------------------------------------------------
static void GLUIDrawString(GLUIString* string)
{
  GLuint program = GetTTFShaderSimpleProgram();
  glUseProgram(program);

  glBindVertexArray(string->_buffer._vao);
  glUniform3f(
    glGetUniformLocation(program, "color"), 
    RANDOM_0_1, 
    RANDOM_0_1, 
    RANDOM_0_1
  );
  glDrawArrays(GL_TRIANGLES,0, string->_buffer._N);

  glBindVertexArray(0);
}

// draw string
//------------------------------------------------------------------------------
static void GLUIConcatenateString(GLUIString* s)
{
  int base = 0, N = 0;

  std::vector<GLUIStringPoint> points;
  for(auto c: s->_C)
  {
    points.resize(points.size() + c._X.size());
    N += c._X.size();
    for(auto p: c._X)
    {
      points[base++] = p;
    }
  }
  GLUIBuildStringBuffer(&s->_buffer, points);
  s->_buffer._N = N;
}

// test
//------------------------------------------------------------------------------
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
  Utility::TTFCore::FontMetrics fm = f.GetFontMetrics();

  strings[0]= new GLUIString();
  strings[0]->_V = "AaBbCc\nDdEeFf\nGgHhIi\nJjKkLl";
  Utility::TTFCore::vec2f offset(-0.5, 0.5);
  GLUIParseString(f, strings[0], 12, offset);
  GLUIConcatenateString(strings[0]);

  offset.y -= 0.5;
  strings[1]= new GLUIString();
  strings[1]->_V = "MmNnOoPpQqRr\nSsTtUuVvWwXxYyZz";
  GLUIParseString(f, strings[1], 12, offset);
  GLUIConcatenateString(strings[1]);
}

static void GLUIDrawTest(const std::vector<GLUIString*>& strings)
{
  for(auto s: strings) GLUIDrawString(s);
}

// initialize
//------------------------------------------------------------------------------
static void GLUIInit()
{
  GLUIBuildStringShader();
  GLUIBuildAlphabet();
}

// terminate
//------------------------------------------------------------------------------
static void GLUITerm()
{
  GLUIDeleteAlphabet();
  GLUIDeleteStringShader();
}


AMN_NAMESPACE_CLOSE_SCOPE