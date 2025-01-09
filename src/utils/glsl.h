//------------------------------------------------------------------------------
// GLSL SHADER UTILS
//------------------------------------------------------------------------------
#ifndef JVR_UTILS_GLSL_H
#define JVR_UTILS_GLSL_H

#include <pxr/imaging/garch/glApi.h>
#include "../common.h"
#include <iostream>
#include <sstream>


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

#endif // JVR_UTILS_GLSL_H