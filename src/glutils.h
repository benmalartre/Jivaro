//------------------------------------------------------------------------------
// GLUTILS
//------------------------------------------------------------------------------
#pragma once

#include <GL/gl3w.h>
#include "glsl.h"
#include <string>
#include <iostream>

namespace AMN {

GLint SCREENSPACEQUAD_VAO;
GLint SCREENSPACEQUAD_VBO;
GLint SCREENSPACEQUAD_SHADER;

static float SCREENSPACEQUAD_POINTS[12] = {
  -1.f, -1.f,
   1.f, -1.f,
   1.f,  1.f,
   1.f,  1.f,
  -1.f,  1.f,
  -1.f, -1.f
};

static unsigned SCREENSPACEQUAD_UVS[12] = {
  0.f, 0.f,
  1.f, 0.f,
  1.f, 1.f,
  1.f, 1.f,
  0.f, 1.f,
  0.f, 0.f
};

void SetupScreenSpaceQuad()
{
  unsigned sz = 12 * sizeof(float);
  // generate vertex array object
  glGenVertexArrays(1, &SCREENSPACEQUAD_VAO);
  glBindVertexArray(SCREENSPACEQUAD_VAO);

  // generate vertex buffer object
  glGenBuffers(1, SCREENSPACEQUAD_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, SCREENSPACEQUAD_VBO);

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER,2 * sz, NULL,GL_STATIC_DRAW)
  glBufferSubData(GL_ARRAY_BUFFER, 0, sz, &SCREENSPACEQUAD_POINTS[0]);
  glBufferSubData(GL_ARRAY_BUFFER, sz, sz, &SCREENSPACEQUAD_UVS[0]);

  // attibute position
  glEnableVertexAttribArray(0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0)
  
  // attibute uvs
  glEnableVertexAttribArray(1)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, sz)

  // bind shader program
  glBindAttribLocation(SCREENSPACEQUAD_SHADER, 0, "position")
  glBindAttribLocation(SCREENSPACEQUAD_SHADER, 1, "coords")
  
  glLinkProgram(SCREENSPACEQUAD_SHADER)
}

static bool 
GLCheckError(std::string message)
{
  GLenum err = glGetError();
  if(err)
  {
    while(err != GL_NO_ERROR)
    {
      switch(err)
      {
        case GL_INVALID_OPERATION:
          std::cout << "[OpenGL Error] " << message << " INVALID_OPERATION" << std::endl;
          break;
        case GL_INVALID_ENUM:
          std::cout << "[OpenGL Error] " << message << " INVALID_ENUM" << std::endl;
          break;
        case GL_INVALID_VALUE:
          std::cout << "[OpenGL Error] " << message << " INVALID_VALUE" << std::endl;
          break;
        case GL_OUT_OF_MEMORY:
          std::cout << "[OpenGL Error] " << message << " OUT_OF_MEMORY" << std::endl;
          break;
        case GL_STACK_UNDERFLOW:
          std::cout << "[OpenGL Error] " << message << "STACK_UNDERFLOW" <<std::endl;
          break;
        case GL_STACK_OVERFLOW:
          std::cout << "[OpenGL Error] " << message << "STACK_OVERFLOW" << std::endl;
          break;
        default:
          std::cout << "[OpenGL Error] " << message << " UNKNOWN_ERROR" << std::endl;
          break;
      }
      err = glGetError();
    }
    return true;
  }
  return false;
}

} // namespace AMN
