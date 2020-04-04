//------------------------------------------------------------------------------
// SCREEN SPACE QUAD
//------------------------------------------------------------------------------
#include "screenSpaceQuad.h"

AMN_NAMESPACE_OPEN_SCOPE

//#ifdef __APPLE__
//  #undef glGenVertexArrays
//  #define glGenVertexArrays glGenVertexArraysAPPLE
//  #undef glBindVertexArray 
//  #define glBindVertexArray glBindVertexArrayAPPLE
//  #undef glDeleteVertexArray
//  #define glDeleteVertexArray glDeleteVertexArrayAPPLE
//#endif


// screen-space quad 
//----------------------------------------------------------------------------
void ScreenSpaceQuad::Setup()
{
  SetupShaderProgram();
  size_t sz = 12 * sizeof(float);
  
  // generate vertex array object
  float version;
  sscanf((const char*)glGetString(GL_VERSION), "%f", &version);
  int glVersion = (int)(version * 100);
#if defined(__APPLE__)
  if(glVersion <= 210) glGenVertexArraysAPPLE(1, &_vao);
  else glGenVertexArrays(1, &_vao);
  if(glVersion <= 210)glBindVertexArrayAPPLE(_vao);
  else glBindVertexArray(_vao);
#else
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
#endif
  // generate vertex buffer object
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,2 * sz, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, &SSQ_POINTS[0]);
  glBufferSubData(GL_ARRAY_BUFFER, sz, sz, &SSQ_UVS[0]);

  // attibute position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  // attibute uvs
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void*)sz);

  // bind shader program
  glBindAttribLocation(_pgm->Get(), 0, "position");
  glBindAttribLocation(_pgm->Get(), 1, "uvs");
  glLinkProgram(_pgm->Get());

  // unbind vertex array object
#if defined(__APPLE__)
  if(glVersion <= 210)glBindVertexArrayAPPLE(0);
  else glBindVertexArray(0);
#else
  glBindVertexArray(0);
#endif
}

void ScreenSpaceQuad::Draw()
{
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES,0,6);
  glBindVertexArray(0);
}

void ScreenSpaceQuad::SetupShaderProgram()
{
  GLCheckError("Setup SCreen Space QUad SHade");
  _vert = new GLSLShader();
  _vert->Compile(SSQ_VERTEX_SHADER_CODE_120, GL_VERTEX_SHADER);
  
  _frag = new GLSLShader();
  _frag->Compile(SSQ_FRAGMENT_SHADER_CODE_120, GL_FRAGMENT_SHADER);

  _pgm = new GLSLProgram();
  _pgm->Build("ScreenSpaceQuadProgram", _vert, _frag);
  GLCheckError("Linking Screen Space Program Shader");

}

 GLuint ScreenSpaceQuad::GetShaderProgram()
{
  return _pgm->Get();
}


AMN_NAMESPACE_CLOSE_SCOPE