//------------------------------------------------------------------------------
// GLUTILS
//------------------------------------------------------------------------------
#ifndef JVR_UTILS_GL_H
#define JVR_UTILS_GL_H

#include "shaders.h"

JVR_NAMESPACE_OPEN_SCOPE

// check opengl error
//----------------------------------------------------------------------------
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
          std::cout << "[OpenGL Error] " << message 
            << " INVALID_OPERATION" << std::endl;
          break;
        case GL_INVALID_ENUM:
          std::cout << "[OpenGL Error] " << message 
            << " INVALID_ENUM" << std::endl;
          break;
        case GL_INVALID_VALUE:
          std::cout << "[OpenGL Error] " << message 
            << " INVALID_VALUE" << std::endl;
          break;
        case GL_OUT_OF_MEMORY:
          std::cout << "[OpenGL Error] " << message 
            << " OUT_OF_MEMORY" << std::endl;
          break;
        case GL_STACK_UNDERFLOW:
          std::cout << "[OpenGL Error] " << message 
            << "STACK_UNDERFLOW" <<std::endl;
          break;
        case GL_STACK_OVERFLOW:
          std::cout << "[OpenGL Error] " << message 
            << "STACK_OVERFLOW" << std::endl;
          break;
        default:
          std::cout << "[OpenGL Error] " << message 
            << " UNKNOWN_ERROR" << std::endl;
          break;
      }
      err = glGetError();
    }
    return true;
  }
  return false;
}

// draw pick image
//----------------------------------------------------------------------------
static void 
CreateOpenGLTexture(int width, int height, 
  int* pixels, GLuint& tex, int ID)
{
  //glDeleteTextures(1 &_pickImage);
  if(!tex)
  {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                    GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST);
  }

  glActiveTexture(GL_TEXTURE0 + ID);
  glBindTexture(GL_TEXTURE_2D,tex);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(	GL_TEXTURE_2D,
                  0,
                  GL_RGBA,
                  width,
                  height,
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  pixels);
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_GL_H