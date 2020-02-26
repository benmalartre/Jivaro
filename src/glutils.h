//------------------------------------------------------------------------------
// GLUTILS
//------------------------------------------------------------------------------
#pragma once

#include "default.h"
#include "glsl.h"

namespace AMN {

static bool 
GLCheckError(std::string message);

static GLuint SCREENSPACEQUAD_VAO;
static GLuint SCREENSPACEQUAD_VBO;

static GLuint SCREENSPACEQUAD_VERTEX_SHADER;
static GLuint SCREENSPACEQUAD_FRAGMENT_SHADER;
static GLuint SCREENSPACEQUAD_PROGRAM_SHADER;

// vertex shader :
static const GLchar* SCREENSPACEQUAD_VERTEX_SHADER_CODE =
"#version 330\n"
"layout(location = 0) in vec2 position;\n"
"layout(location = 1) in vec2 uvs;\n"
"out vec2 st;\n"
"void main() {\n"
"    st = uvs;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

// fragment shader :
static const GLchar* SCREENSPACEQUAD_FRAGMENT_SHADER_CODE =
"#version 330\n"
"in vec2 st;\n"
"uniform sampler2D tex;\n"
"out vec4 out_color;\n"
"void main() {\n"
"    out_color = vec4(texture(tex,st));//vec4(texture(tex,st).rgb,1.0);\n"
"}\n";

static float SCREENSPACEQUAD_POINTS[12] = {
  -1.f, -1.f,
   1.f, -1.f,
   1.f,  1.f,
   1.f,  1.f,
  -1.f,  1.f,
  -1.f, -1.f
};

static float SCREENSPACEQUAD_UVS[12] = {
  0.f, 1.f,
  1.f, 1.f,
  1.f, 0.f,
  1.f, 0.f,
  0.f, 0.f,
  0.f, 1.f
};

static void SetupScreenSpaceQuadShader()
{
  SCREENSPACEQUAD_VERTEX_SHADER = 
    glslCompileShader(GL_VERTEX_SHADER, SCREENSPACEQUAD_VERTEX_SHADER_CODE);
    GLCheckError("CREATE VERTEX SHADER");
  SCREENSPACEQUAD_FRAGMENT_SHADER = 
    glslCompileShader(GL_FRAGMENT_SHADER, SCREENSPACEQUAD_FRAGMENT_SHADER_CODE);
    GLCheckError("CREATE FRAGMENT SHADER");
  SCREENSPACEQUAD_PROGRAM_SHADER = 
    glslLinkProgram(SCREENSPACEQUAD_VERTEX_SHADER, 
      SCREENSPACEQUAD_FRAGMENT_SHADER);
    GLCheckError("CREATE PROGRAM SHADER");
}

static void SetupScreenSpaceQuad()
{
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
}

static void 
DrawScreenSpaceQuad()
{
  glBindVertexArray(SCREENSPACEQUAD_VAO);
  glDrawArrays(GL_TRIANGLES,0,6);
  glBindVertexArray(0);
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

} // namespace AMN
