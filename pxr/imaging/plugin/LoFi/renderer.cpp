//
// Copyright 2020 benmalartre
//
// Unlicensed 
//
#include "pxr/pxr.h"
#include "pxr/imaging/glf/glew.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/plugin/LoFi/renderer.h"
#include "pxr/imaging/plugin/LoFi/scene.h"
#include "pxr/imaging/plugin/LoFi/mesh.h"

#include "pxr/imaging/hd/perfLog.h"

#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/work/loops.h"

#include <chrono>
#include <thread>
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE


const char *VERTEX_SHADER_330[1] = {
  "#version 330                                             \n" 
  "uniform mat4 model;                                      \n"
  "uniform mat4 view;                                       \n"
  "uniform mat4 projection;                                 \n"
  "                                                         \n"
  "in vec3 position;                                        \n"
  "void main(){                                             \n"
  "    vec3 p = vec3(view * model * vec4(position,1.0));    \n"
  "    gl_Position = projection * vec4(p,1.0);              \n"
  "}"
};

const char *FRAGMENT_SHADER_330[1] = {
  "#version 330                                             \n"
  "uniform vec3 color;                                      \n"
  "out vec4 outColor;                                       \n"
  "void main()                                              \n"
  "{                                                        \n"
  "	outColor = vec4(1.0,1.0,1.0,1.0);                       \n"
  "}"
};

static float POINTS[12] = {
  -100.f, -100.f, 0.f,
  -100.f,  100.f, 0.f,
   100.f,  100.f, 0.f,
   100.f, -100.f, 0.f,
};

static int INDICES[6] = {
  0, 1, 2,
  2, 3, 0
};

LoFiRenderer::LoFiRenderer()
    : _width(0)
    , _height(0)
    , _viewMatrix(1.0f) // == identity
    , _projMatrix(1.0f) // == identity
    , _inverseViewMatrix(1.0f) // == identity
    , _inverseProjMatrix(1.0f) // == identity
    , _scene(nullptr)
    , _enableSceneColors(false)
{

  GlfContextCaps const& caps = GlfContextCaps::GetInstance();
  if(caps.glslVersion >= 330)
  {
    // load shader from files
    GLSLShader vertexShader;
    vertexShader.Load(
      "/Users/benmalartre/Documents/RnD/USD/pxr/imaging/plugin/LoFi/simple_vertex.glsl", 
      GL_VERTEX_SHADER
    );

    GLSLShader fragmentShader;
    fragmentShader.Load(
      "/Users/benmalartre/Documents/RnD/USD/pxr/imaging/plugin/LoFi/simple_fragment.glsl",
      GL_FRAGMENT_SHADER
    );
    _program.Build("Simple", &vertexShader, &fragmentShader);
  }
  else
  {
    // load shader from files
    GLSLShader vertexShader;
    vertexShader.Load(
      "/Users/benmalartre/Documents/RnD/USD/pxr/imaging/plugin/LoFi/simple120_vertex.glsl", 
      GL_VERTEX_SHADER
    );

    GLSLShader fragmentShader;
    fragmentShader.Load(
      "/Users/benmalartre/Documents/RnD/USD/pxr/imaging/plugin/LoFi/simple120_fragment.glsl",
      GL_FRAGMENT_SHADER
    );
    _program.Build("Simple", &vertexShader, &fragmentShader);
  
  }
  
  // build shader from string
  //_program.Build("Simple", VERTEX_SHADER_330, FRAGMENT_SHADER_330);

  size_t szp = 12 * sizeof(float);
  size_t szi = 6 * sizeof(int);

  glGenVertexArraysAPPLE(1, &_vao);
  glBindVertexArrayAPPLE(_vao);

  // generate vertex buffer object
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);

  // push buffer to GPU
  glBufferData(GL_ARRAY_BUFFER, szp, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, szp, &POINTS[0]);

  // generate element buffer object
  glGenBuffers(1, &_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, szi, NULL,GL_STATIC_DRAW);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, szi, &INDICES[0]);

  // attibute position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

  // bind shader program
  glUseProgram(_program.Get());
  glBindAttribLocation(_program.Get(), 0, "position");
  glLinkProgram(_program.Get());

  // unbind vertex array object
  glBindVertexArrayAPPLE(0);
  glUseProgram(0);
}

LoFiRenderer::~LoFiRenderer()
{
}

void
LoFiRenderer::SetScene(LoFiScene* scene)
{
  _scene = scene;
}

void
LoFiRenderer::SetEnableSceneColors(bool enableSceneColors)
{
  _enableSceneColors = enableSceneColors;
}

void
LoFiRenderer::SetViewport(unsigned int width, unsigned int height)
{
  _width = width;
  _height = height;
}

void
LoFiRenderer::SetCamera(const GfMatrix4d& viewMatrix,
                            const GfMatrix4d& projMatrix)
{
  _viewMatrix = viewMatrix;
  _projMatrix = projMatrix;
  _inverseViewMatrix = viewMatrix.GetInverse();
  _inverseProjMatrix = projMatrix.GetInverse();
}

void
LoFiRenderer::Render(void)
{
  int totalNumPoints = 0;
  int totalNumTriangles = 0;
  for(auto& item: _scene->GetMeshes()) 
  {
    LoFiMeshDesc* mesh = &item.second;
    mesh->_basePointIndex = totalNumPoints;
    mesh->_baseTriangleIndex = totalNumTriangles;
    
    totalNumPoints += mesh->_numPoints;
    totalNumTriangles += mesh->_numTriangles;
  }
  
  
  // repopulate buffer
  glBindVertexArrayAPPLE(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

  glBufferData(
    GL_ARRAY_BUFFER, 
    totalNumPoints * 3 * sizeof(float), 
    NULL, 
    GL_DYNAMIC_DRAW
  );

  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, 
    totalNumTriangles * 3 * sizeof(int), 
    NULL, 
    GL_DYNAMIC_DRAW
  );

  for(auto& item: _scene->GetMeshes()) 
  {
    LoFiMeshDesc* mesh = &item.second;
    
    glBufferSubData(
      GL_ARRAY_BUFFER, 
      mesh->_basePointIndex * 3 * sizeof(float), 
      mesh->_numPoints * 3 * sizeof(float), 
      &mesh->_positions[0][0]
    );
    
    // offset triangles indices;
    VtArray<int> offsetIndices(mesh->_numTriangles * 3);
    int offset = mesh->_baseTriangleIndex * 3;
    int j = 0;
    for(int i=0;i<mesh->_numTriangles;++i)
    {
      offsetIndices[i*3  ] = mesh->_indices[i][0] + offset;
      offsetIndices[i*3+1] = mesh->_indices[i][1] + offset;
      offsetIndices[i*3+2] = mesh->_indices[i][2] + offset;
    }

    glBufferSubData(
      GL_ELEMENT_ARRAY_BUFFER, 
      mesh->_baseTriangleIndex * 3 * sizeof(int), 
      mesh->_numTriangles * 3 * sizeof(int), 
      &offsetIndices[0]
    );
  }

  _clearColor = GfVec4f(0.f,0.f,0.f,1.f);

  glClearColor(_clearColor[0],_clearColor[1],_clearColor[2],_clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(_program.Get());
      
  GfMatrix4f identity(1);
  // model matrix
  glUniformMatrix4fv(
    glGetUniformLocation(_program.Get(), "model"),
    1,
    GL_FALSE,
    &identity[0][0]
  );
  
  // view matrix
  glUniformMatrix4fv(
    glGetUniformLocation(_program.Get(), "view"),
    1,
    GL_FALSE,
    &GfMatrix4f(_viewMatrix)[0][0]
  );

  // projection matrix
  glUniformMatrix4fv(
    glGetUniformLocation(_program.Get(), "projection"),
    1,
    GL_FALSE,
    &GfMatrix4f(_projMatrix)[0][0]
  );

  
  static GfVec3f red(1.f, 0.f, 0.f);
  static GfVec3f green(0.f, 1.f, 0.f);

  // draw red points
  glEnable(GL_POINT_SMOOTH);
  glPointSize(2);
  glBindVertexArrayAPPLE(_vao);
  
  glUniform3fv(
    glGetUniformLocation(_program.Get(),"color"),
    1,
    &green[0]
  );

  glDrawElements(
    GL_TRIANGLES, 
    totalNumTriangles * 3, 
    GL_UNSIGNED_INT, 
    (const void*)0
  );

  glUniform3fv(
    glGetUniformLocation(_program.Get(),"color"),
    1,
    &red[0]
  );
  glDrawArrays(GL_POINTS, 0, totalNumPoints);

  glBindVertexArrayAPPLE(0);
  glUseProgram(0);
  glDisable(GL_POINT_SMOOTH);

}


PXR_NAMESPACE_CLOSE_SCOPE