//------------------------------------------------------------------------------
// GLSL SHADER UTILS
//------------------------------------------------------------------------------

#pragma once

#include "../default.h"
#include <iostream>

// vertex shader :
static const GLchar* SIMPLE_VERTEX_SHADER_CODE =
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
static const GLchar* SIMPLE_FRAGMENT_SHADER_CODE =
"#version 330 core\n"
"in vec3 vertex_color;\n"
"out vec4 out_color;\n"
"void main() {\n"
"    out_color = vec4(vertex_color, 1.0);\n"
"//    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
"}\n";


static GLuint 
glslCompileShader(GLenum type, const GLchar *source)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  GLint param;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &param);
  if (!param) 
  {
    GLchar log[4096];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    std::cerr << "error compiling shader: " << (char *) log << std::endl;
    return 0;
  }
  return shader;
}

static GLuint 
glslLinkProgram(GLuint vert, GLuint frag, GLuint geom = 0)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  if(geom != 0)
    glAttachShader(program, geom);

  glLinkProgram(program);
  GLint param;
  glGetProgramiv(program, GL_LINK_STATUS, &param);
  if (!param) 
  {
    GLchar log[4096];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    std::cerr << "error linking program shader: " << (char *) log << std::endl;
    return 0;
  }
  return program;
}

#define checkGLError() _checkGLError(__FILE__, __LINE__)

static bool 
_checkGLError(const char *file, int line)
{
  std::stringstream ss;
  // may accumulate errors, loop on types
  GLenum e(glGetError());
  while(e != GL_NO_ERROR)
  {
    ss << "GLERROR : ";
    switch(e) 
    {
      case GL_INVALID_OPERATION:
        ss << "GL_INVALID_OPERATION";
        break;
      case GL_INVALID_ENUM:
        ss << "GL_INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        ss << "GL_INVALID_VALUE";
        break;
      case GL_OUT_OF_MEMORY:
        ss << "GL_OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        ss << "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
      case GL_STACK_UNDERFLOW:
        ss << "GL_STACK_UNDERFLOW";
        break;
      case GL_STACK_OVERFLOW:
        ss << "GL_STACK_OVERFLOW";
        break;
      default:
        ss << "UnknowError";
        break;
    }
    ss << std::endl;
    e = glGetError();
  }
  // Finally print all in one
  if(ss.str().size())
  {
    std::cerr << file << ":" << line << ":" 
      << std::endl << ss.str().c_str() << std::endl;
    return true;
  }
  return false;
}
