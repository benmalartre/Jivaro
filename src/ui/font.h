#pragma once

#include "base.h"
#include "shader.h"

AMN_NAMESPACE_OPEN_SCOPE

struct GLUICharacter : public GLUIAtom {
  std::vector<GLUIPoint>      _X;             // points
  std::vector<int>            _I;             // break indices
  float                       _W;             // width
  float                       _H;             // height

  GLUICharacter(){};
  GLUICharacter(const GLUICharacter& other) {
    _P = other._P;
    _X = other._X;
    _I = other._I;
    _W = other._W;
    _H = other._H;
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
static GLUICharacter GLUISpace() {
  GLUICharacter c;

  c._P = pxr::GfVec2f();                           // position
  c._X = std::vector<GLUIPoint>();                 // empty points
  c._I = std::vector<int>();                       // empty indices
  c._W = 0.006f;//60.f;                                     // width
  c._H = 0.01f;//100.f;                                    // height
  
  return c;
}

// !
//------------------------------------------------------------------------------
static GLUICharacter GLUIExclamation() {
  GLUICharacter c;
  c._X.resize(3);
  c._I.resize(2);
  c._X[0] = MakePoint(20, 100);
  c._X[1] = MakePoint(20, 60); 
  c._X[2] = MakePoint(20, 50);
  c._I[0] = 2;
  c._W = 0.004f;//40.f;
  c._H = 0.01f;//100.f;

  return c;
}

// A
//------------------------------------------------------------------------------
static GLUICharacter GLUICapitalA() {
  GLUICharacter c;
  c._X.resize(5);
  c._I.resize(2);
  c._X[0] = MakePoint(-0.3, 0.0); 
  c._X[1] = MakePoint(0, 1); 
  c._X[2] = MakePoint(0.3, 0.0);
  c._X[3] = MakePoint(-0.15, 0.5); 
  c._X[4] = MakePoint(0.15, 0.5);

  c._I[0] = 3;
  c._I[1] = 5;
  c._W = 1.f;
  c._H = 1.f;

  return c;
}

// build alphabet
//------------------------------------------------------------------------------
static void GLUIBuildAlphabet()
{
  ALPHABET.resize(255);
  ALPHABET[32] = GLUISpace();
  ALPHABET[33] = GLUIExclamation();
  ALPHABET[65] = GLUICapitalA();
}

static GLUIString ParseString(const std::string& s)
{
  GLUIString result;
  result._C.resize(s.length());
  result._V = s;
  int idx = 0;
  pxr::GfVec2f p;
  for(auto c : s)
  {
    
    if((int)c == 10 || (int)c == 13) 
    {
      p[1] += ALPHABET[(int)c]._H;
      p[0] = 0;
    }
      
    else
    {
      result._C[idx] = ALPHABET[(int)c];
      result._C[idx++]._P = p;
      p[0] += ALPHABET[(int)c]._W;
    }
    

  }
  return result;
}

static void GLUIDrawString(GLUIString s)
{
  int baseP = 0, baseI = 0, base=0;

  std::vector<GLUIPoint> points;
  std::vector<int> indices;
  for(auto c: s._C)
  {
    points.resize(points.size() + c._X.size());
    for(auto p: c._X)
    {
      points[baseP++] = p;
    }
    indices.resize(indices.size() + c._I.size());
    for(auto i: c._I)
    {
      indices[baseI++] = i + base;
    }
    base += points.size();
  }
  GLUIBuffer buffer = BuildBuffer(points, indices);

  DrawBuffer(buffer, indices);
}

static void GLUITest()
{
  GLUIString s = ParseString("AAAA\nAA\nAA");
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