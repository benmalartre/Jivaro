//--------------------------------------------------------------------------------
// SHADERS UTILS
//--------------------------------------------------------------------------------
#ifndef JVR_UTILS_SHADER_H
#define JVR_UTILS_SHADER_H

#include "../common.h"
#include <stdio.h>
#include <string.h>
#include <pxr/imaging/garch/glApi.h>

#include "files.h"

JVR_NAMESPACE_OPEN_SCOPE

// vertex shader :
static const char* SIMPLE_VERTEX_SHADER_CODE_120 = R"""(
#version 120
attribute in vec3 position;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;
void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
})""";


// fragment shader :
static const char* SIMPLE_FRAGMENT_SHADER_CODE_120 = R"""(
#version 120
uniform vec4 color;
void main() {
    gl_FragColor = color;
//    out_color = vec4(0.0, 1.0, 0.0, 1.0);
})""";

// vertex shader :
static const char* SIMPLE_VERTEX_SHADER_CODE_330 = R"""(
#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;
void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
})""";

// fragment shader :
static const char* SIMPLE_FRAGMENT_SHADER_CODE_330 = R"""(
#version 330 core
uniform vec4 color;
out vec4 out_color;
void main() {
    out_color = color;
//    out_color = vec4(0.0, 1.0, 0.0, 1.0);
})""";

// vertex shader :
static const char* COLORED_VERTEX_SHADER_CODE_120 = R"""(
#version 120
attribute in vec3 position;
attribute in vec3 color;
uniform float hue;
uniform mat4 viewProj;
varying vec3 vertex_color;
void main() {
    vertex_color = color*hue;
    gl_Position = viewProj * vec4(position, 1.0);
})""";

// fragment shader :
static const char* COLORED_FRAGMENT_SHADER_CODE_120 = R"""(
#version 120
varying vec3 vertex_color;
void main() {
    gl_FragColor = vec4(vertex_color, 1.0);
//    out_color = vec4(0.0, 1.0, 0.0, 1.0);
})""";

// vertex shader :
static const char* COLORED_VERTEX_SHADER_CODE_330 = R"""(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
uniform float hue;
uniform mat4 viewProj;
out vec3 vertex_color;
void main() {
    vertex_color = color*hue;
    gl_Position = viewProj * vec4(position, 1.0);
})""";

// fragment shader :
static const char* COLORED_FRAGMENT_SHADER_CODE_330 = R"""(
#version 330 core
in vec3 vertex_color;
out vec4 out_color;
void main() {
    out_color = vec4(vertex_color, 1.0);
//    out_color = vec4(0.0, 1.0, 0.0, 1.0);
})""";
    

enum GLSLShaderType
{
  SHADER_VERTEX,
  SHADER_GEOMETRY,
  SHADER_FRAGMENT
};
    
class GLSLShader{
public:
  GLSLShader():_shader(0),_code(NULL){};
  GLSLShader(const GLSLShader& ) = delete;
  GLSLShader(GLSLShader&&) = delete;

  ~GLSLShader()
  {
    if(_shader)glDeleteShader(_shader);
    if(_code)delete [] _code;
  }
  void Load(const char* filename);
  void Compile(const char* c, GLenum t);
  void Compile(GLenum t);
  void OutputInfoLog();
  GLuint Get(){return _shader;};
  void Set(const char* code);
private:
  std::string _path;
  GLchar* _code;
  GLenum _type;
  GLuint _shader;
};
    
class GLSLProgram
{
friend GLSLShader;
public:
  GLSLProgram():_vert(),_geom(),_frag(),_pgm(0),_ownVertexShader(false),
    _ownGeometryShader(false),_ownFragmentShader(false){};
  GLSLProgram(const GLSLProgram&) = delete;
  GLSLProgram(GLSLProgram&&) = delete;

  ~GLSLProgram()
  {
    if(_ownFragmentShader && _frag)delete _frag;
    if(_ownGeometryShader && _geom)delete _geom;
    if(_ownVertexShader && _vert)delete _vert;
  }
  void _Build();
  void Build(const char* name);
  void BuildFromString(const char* name, const char* s_vert, const char* s_frag);
  void BuildFromString(const char* name, const char* s_vert, const char* s_geom, 
    const char* s_frag);
  void Build(const char* name, GLSLShader* vertex, GLSLShader* fragment);
  void Build(const char* name, GLSLShader* vertex, GLSLShader* geom, 
    GLSLShader* fragment);
  void OutputInfoLog();
  GLuint Get(){return _pgm;};
private:
  GLSLShader* _vert;
  bool _ownVertexShader;
  GLSLShader* _geom;
  bool _ownGeometryShader;
  GLSLShader* _frag;
  bool _ownFragmentShader;
  GLuint _pgm;
  std::string _name; 
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_SHADER_H