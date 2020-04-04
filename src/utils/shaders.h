#pragma once

#include "../common.h"
#include <stdio.h>
#include <string.h>
#include <pxr/imaging/glf/glew.h>

#include "files.h"

AMN_NAMESPACE_OPEN_SCOPE

// vertex shader :
static const GLchar* SIMPLE_VERTEX_SHADER_CODE_120 =
"#version 120\n"
"attribute in vec3 position;\n"
"attribute in vec3 color;\n"
"uniform float hue;\n"
"uniform mat4 viewProj;\n"
"varying vec3 vertex_color;\n"
"void main() {\n"
"    vertex_color = color*hue;\n"
"    gl_Position = viewProj * vec4(position, 1.0);\n"
"}\n";

// fragment shader :
static const GLchar* SIMPLE_FRAGMENT_SHADER_CODE_120 =
"#version 120\n"
"varying vec3 vertex_color;\n"
"void main() {\n"
"    gl_FragColor = vec4(vertex_color, 1.0);\n"
"//    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
"}\n";

// vertex shader :
static const GLchar* SIMPLE_VERTEX_SHADER_CODE_330 =
"#version 330 core\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec3 color;\n"
"uniform float hue;\n"
"uniform mat4 viewProj;\n"
"out vec3 vertex_color;\n"
"void main() {\n"
"    vertex_color = color*hue;\n"
"    gl_Position = viewProj * vec4(position, 1.0);\n"
"}\n";

// fragment shader :
static const GLchar* SIMPLE_FRAGMENT_SHADER_CODE_330 =
"#version 330 core\n"
"in vec3 vertex_color;\n"
"out vec4 out_color;\n"
"void main() {\n"
"    out_color = vec4(vertex_color, 1.0);\n"
"//    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
"}\n";
    

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
  void Build(const char* name, const char* s_vert="", const char* s_frag="");
  void Build(const char* name, const char* s_vert="", const char* s_geom="", const char* s_frag="");
  void Build(const char* name, GLSLShader* vertex, GLSLShader* fragment);
  void Build(const char* name, GLSLShader* vertex, GLSLShader* geom, GLSLShader* fragment);
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

AMN_NAMESPACE_CLOSE_SCOPE
