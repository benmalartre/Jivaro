#include "shaders.h"
#include <iostream>
#include "../utils/glutils.h"

JVR_NAMESPACE_OPEN_SCOPE

void GLSLShader::OutputInfoLog()
{
  char buffer[512];
  glGetShaderInfoLog(_shader, 512, NULL, &buffer[0]);
  std::cout << "GLSL compilation error  : " << (std::string)buffer << std::endl;
}

void GLSLShader::Load(const char* filename)
{
  File f(filename);
  if(f.Open(FILE_READ))
  {
    unsigned long len = f.GetFileLength();
    if(len==0)
      std::cout << "[GLSL] File Empty : " << filename << std::endl;
    else
    {
      std::string code = f.ReadAll();
      _code = (GLchar*) new char[len+1];
      _code[len] = 0;
      memcpy(_code, code.c_str(), len);
    }
    f.Close();
  }
  else
    std::cout << "[GLSL] Fail Open File : " << filename << std::endl;
}

void GLSLShader::Set(const char* code)
{
  size_t len = strlen(code);
  _code = (GLchar*) new char[len+1];
  _code[len] = 0;
  memcpy(_code, code, len);
}

void GLSLShader::Compile(const char* code, GLenum type)
{
  _shader = glCreateShader(type);

  glShaderSource(_shader,1,(GLchar**)code,NULL);
  glCompileShader(_shader);
  
  GLint status;
  glGetShaderiv(_shader,GL_COMPILE_STATUS,&status);
  if(status)
    std::cout << "[GLSLCreateShader] Success Compiling Shader !"<<std::endl;
  else
  {
    std::cout << "[GLSLCreateShader] Fail Compiling Shader !" <<std::endl;
    OutputInfoLog();
  }
}

void GLSLShader::Compile(GLenum type)
{
  _shader = glCreateShader(type);
  
  glShaderSource(_shader,1,(GLchar**)&_code,NULL);
  glCompileShader(_shader);
  
  GLint status;
  glGetShaderiv(_shader,GL_COMPILE_STATUS,&status);
  if(status)
    std::cout << "[GLSLCreateShader] Success Compiling Shader !"<<std::endl;
  else
  {
    std::cout << "[GLSLCreateShader] Fail Compiling Shader !"<<std::endl;
    OutputInfoLog();
  }
}

void GLSLProgram::_Build()
{  
  _pgm = glCreateProgram();
  GLCheckError("Create Program : ");
  
  if(_vert)
  {
    glAttachShader(_pgm,_vert->Get());
    GLCheckError("Attach Vertex Shader ");
  }

  if(_geom && _geom->Get())
  {
    glAttachShader(_pgm,_geom->Get());
    GLCheckError("Attach Geometry Shader ");
  }
  
  if(_frag)
  {
    glAttachShader(_pgm,_frag->Get());
    GLCheckError("Attach Fragment Shader ");
  }
  
  glBindAttribLocation(_pgm,0,"position");
  
  glLinkProgram(_pgm);
  GLCheckError("Link Program : ");
  
  glUseProgram(_pgm);
  GLCheckError("Use Program : ");

}

void GLSLProgram::BuildFromString(const char* name, const char* vertex, 
  const char* fragment)
{
  _name = name;
  _vert = new GLSLShader();
  _vert->Set(vertex);
  _vert->Compile(GL_VERTEX_SHADER);
  _ownVertexShader = true;
  GLCheckError("Compile Vertex Shader : ");

  _frag = new GLSLShader();
  _frag->Set(fragment);
  _frag->Compile(GL_FRAGMENT_SHADER);
  _ownFragmentShader = true;
  GLCheckError("Compile Fragment Shader : ");

  _Build();
}

void GLSLProgram::BuildFromString(const char* name, const char* vertex, 
  const char* geom, const char* fragment)
{
  _name = name;
  _vert = new GLSLShader();
  _vert->Set(vertex);
  _vert->Compile(GL_VERTEX_SHADER);
  _ownVertexShader = true;
  GLCheckError("Compile Vertex Shader : ");

  _geom = new GLSLShader();
  _geom->Set(geom);
  _geom->Compile(GL_GEOMETRY_SHADER);
  _ownGeometryShader = true;
  GLCheckError("Compile Geometry Shader : ");

  _frag = new GLSLShader();
  _frag->Set(fragment);
  _frag->Compile(GL_FRAGMENT_SHADER);
  _ownFragmentShader = true;
  GLCheckError("Compile Fragment Shader : ");

  _Build();
}

void GLSLProgram::Build(const char* name, GLSLShader* vertex, GLSLShader* fragment)
{
  _name = name;
  _vert = vertex;
  _frag = fragment;

  _Build();
}

void GLSLProgram::Build(const char* name, GLSLShader* vertex, GLSLShader* geom, GLSLShader* fragment)
{
  _name = name;
  _vert = vertex;
  _geom = geom;
  _frag = fragment;

  _Build();
}

void GLSLProgram::Build(const char* name)
{
  _name = (std::string)name;
  std::string filename;
  filename = "glsl/"+_name+"_vertex.glsl";
  _vert = new GLSLShader();
  _vert->Load(filename.c_str());
  _vert->Compile(GL_VERTEX_SHADER);
  _ownVertexShader = true;
  GLCheckError("Compile Vertex Shader : ");

  filename = "glsl/"+_name+"_fragment.glsl";
  _frag = new GLSLShader();
  _frag->Load(filename.c_str());
  _frag->Compile(GL_FRAGMENT_SHADER);
  _ownFragmentShader = true;
  GLCheckError("Compile Fragment Shader : ");
  
  _Build();
}

JVR_NAMESPACE_CLOSE_SCOPE
