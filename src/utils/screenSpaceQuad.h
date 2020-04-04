//------------------------------------------------------------------------------
// SCREEN SPACE QUAD
//------------------------------------------------------------------------------
#pragma once

#include "../common.h"
#include "shaders.h"
#include "glutils.h"

AMN_NAMESPACE_OPEN_SCOPE

static const float SSQ_POINTS[12] = {
  -1.f, -1.f,
  1.f, -1.f,
  1.f,  1.f,
  1.f,  1.f,
  -1.f,  1.f,
  -1.f, -1.f
};

static const float SSQ_UVS[12] = {
  0.f, 1.f,
  1.f, 1.f,
  1.f, 0.f,
  1.f, 0.f,
  0.f, 0.f,
  0.f, 1.f
};

// vertex shader 120:
static const GLchar* SSQ_VERTEX_SHADER_CODE_120 =
  "#version 120\n"
  "attribute vec2 position;\n"
  "attribute vec2 uvs;\n"
  "varying vec2 st;\n"
  "void main() {\n"
  "    st = uvs;\n"
  "    gl_Position = vec4(position, 0.0, 1.0);\n"
  "}\n";

// fragment shader 120:
static const GLchar* SSQ_FRAGMENT_SHADER_CODE_120 =
  "#version 120\n"
  "varying vec2 st;\n"
  "uniform sampler2D tex;\n"
  "void main() {\n"
  "    gl_FragColor = vec4(texture2D(tex,st));//vec4(texture(tex,st).rgb,1.0);\n"
  "}\n";

// vertex shader :
static const GLchar* SSQ_VERTEX_SHADER_CODE_330 =
  "#version 330\n"
  "layout(location = 0) in vec2 position;\n"
  "layout(location = 1) in vec2 uvs;\n"
  "out vec2 st;\n"
  "void main() {\n"
  "    st = uvs;\n"
  "    gl_Position = vec4(position, 0.0, 1.0);\n"
  "}\n";

// fragment shader :
static const GLchar* SSQ_FRAGMENT_SHADER_CODE_330 =
  "#version 330\n"
  "in vec2 st;\n"
  "uniform sampler2D tex;\n"
  "out vec4 out_color;\n"
  "void main() {\n"
  "    out_color = vec4(texture(tex,st));//vec4(texture(tex,st).rgb,1.0);\n"
  "}\n";


// screen-space quad 
//----------------------------------------------------------------------------
class ScreenSpaceQuad {

public:
  ScreenSpaceQuad():_vert(NULL),_frag(NULL),_pgm(NULL),_vbo(0),_vao(0){};
  void Setup();
  void Draw();

  void SetupShaderProgram();
  GLuint GetShaderProgram();

private:
  GLuint          _vao;
  GLuint          _vbo;
  GLSLShader*     _vert;
  GLSLShader*     _frag;
  GLSLProgram*    _pgm;
};

AMN_NAMESPACE_CLOSE_SCOPE