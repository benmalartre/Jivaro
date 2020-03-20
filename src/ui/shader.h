#pragma once

#include "../utils/glsl.h"
#include "../utils/glutils.h"

AMN_NAMESPACE_OPEN_SCOPE

//------------------------------------------------------------------------------
// LINE WITH THICKNESS SHADER
//------------------------------------------------------------------------------
// text drawing shader 
static GLuint TEXT_VERTEX_SHADER;
static GLuint TEXT_GEOMETRY_SHADER;
static GLuint TEXT_FRAGMENT_SHADER;
static GLuint TEXT_PROGRAM_SHADER;

// text vertex shader :
static const GLchar* TEXT_VERTEX_SHADER_CODE =
"#version 330\n"
"in vec4 datas;\n"
"out vertex_datas{\n"
" float thickness;\n"
"	vec4 color;\n"
"} vertex;\n"
"vec4 unpackColor(float f) \n"
"{\n"
" vec4 color;\n"
" color.r = floor(f / 256.0 / 256.0);\n"
" color.g = floor((f - color.r * 256.0 * 256.0) / 256.0);\n"
" color.b = floor(f - color.r * 256.0 * 256.0 - color.g * 256.0);\n"
"	color.a = 255;\n"
" return color / 256.0;\n"
"}\n"
"void main(){\n"
"	vertex.color = unpackColor(0.666);\n"
"	vertex.thickness = datas.z;\n"
"	gl_PointSize = 1;\n"
"	gl_Position = vec4(datas.xy, 0.0,1.0);\n"
"}\n";


// text geometry shader :
static const GLchar* TEXT_GEOMETRY_SHADER_CODE =
"#version 330\n"

"layout(lines_adjacency) in;\n"
"layout(triangle_strip) out;\n"
"layout(max_vertices = 4) out;\n"

"in vertex_datas {\n"
"	  float thickness;\n"
"	  vec4 color;\n"
"} vertices[];\n"

"out vec4 fragColor;\n"

"void main() {\n"
" float thick1 = vertices[1].thickness;\n"
" float thick2 = vertices[2].thickness;\n"
" vec3 a = gl_in[0].gl_Position.xyz;\n"
" vec3 b = gl_in[1].gl_Position.xyz;\n"
" vec3 c = gl_in[2].gl_Position.xyz;\n"
" vec3 d = gl_in[3].gl_Position.xyz;\n"
" vec3 dir = c-b;\n"
" vec3 norm = normalize(cross(dir,vec3(0,0,1)));\n"
" vec3 tan1 = normalize(b-a);\n"
" vec3 tan2 = normalize(d-c);\n"
" vec3 miter1 = vec3(-tan1.y,tan1.x,0);\n"
" vec3 miter2 = vec3(-tan2.y,tan2.x,0);\n"
" float length1 = thick1/dot(norm,miter1);\n"
" float length2 = thick2/dot(norm,miter2);\n"
" fragColor = vertices[1].color;\n"
" gl_Position = vec4(b-norm*thick1,1);\n"
" EmitVertex();\n"
" gl_Position = vec4(b+norm*thick1,1);\n"
" EmitVertex();\n"
" fragColor = vertices[2].color;\n"
" gl_Position = vec4(c-norm*thick2,1);\n"
" EmitVertex();\n"
" gl_Position = vec4(c+norm*thick2,1);\n"
" EmitVertex();\n"
" EndPrimitive();\n"
"}\n";


// text fragment shader :
static const GLchar* TEXT_FRAGMENT_SHADER_CODE =
"#version 330\n"
"out vec4 outColor;\n"
"in vec4 fragColor;\n"
"void main(){\n"
"	  outColor = vec4(fragColor);\n"
"}\n";

//------------------------------------------------------------------------------
// FILL SIMPLE SHADER
//------------------------------------------------------------------------------
static GLuint FILL_VERTEX_SHADER;
static GLuint FILL_FRAGMENT_SHADER;
static GLuint FILL_GEOMETRY_SHADER;
static GLuint FILL_PROGRAM_SHADER;

// fill vertex shader :
static const GLchar* FILL_VERTEX_SHADER_CODE =
"#version 330\n"
"layout(location = 0) in vec4 datas;\n"
"out vec4 color;\n"
"vec4 unpackColor(float f) {\n"
"   int packed = floatBitsToInt(f);\n"
"   int r = (packed >> 16) & 255;\n"
"   int g = (packed >> 8) & 255;\n"
"   int b = packed & 255;\n"
"   return vec4(float(r)/255.0, float(g)/255.0, float(b)/255.0, 1.0);"
"}\n"
"void main() {\n"
"    color = unpackColor(datas.a);\n"
"    gl_Position = vec4(datas.xy, 0.0, 1.0);\n"
"}\n";

// fill geometry shader :
static const GLchar* FILL_GEOMETRY_SHADER_CODE =
"#version 150 core\n"
"layout(points) in;\n"
"layout(line_strip, max_vertices = 2) out;\n"
"void main()\n"
"{\n"
"    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);\n"
"    EmitVertex();\n"
"    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);\n"
"    EmitVertex();\n"
"    EndPrimitive();\n"
"}\n";

// fill fragment shader :
static const GLchar* FILL_FRAGMENT_SHADER_CODE =
"#version 330\n"
"in vec4 color;\n"
"out vec4 result;\n"
"void main() {\n"
"    result = color;\n"
"}\n";


//------------------------------------------------------------------------------
// TTF DEDICATED SHADER
//------------------------------------------------------------------------------
static GLuint TTF_VERTEX_SHADER;
static GLuint TTF_FRAGMENT_SHADER;
static GLuint TTF_FRAGMENT_SIMPLE_SHADER;
static GLuint TTF_PROGRAM_SHADER;
static GLuint TTF_PROGRAM_SIMPLE_SHADER;

static const char *TTF_FRAGMENT_SIMPLE_SHADER_CODE = "\
#version 330\n\
in vec3 positions;\n\
uniform vec3 color;\n\
out vec4 fragColor;\n\
float round(float val)\n\
{\n\
  return sign(val)*floor(abs(val)+0.5);\n\
}\n\
void main()\n\
{\n\
  float alpha = round((positions.x*positions.x-positions.y)*positions.z+0.5);\n\
  fragColor = alpha * vec4(color, 1.0);\n\
}";

static const char *TTF_FRAGMENT_SHADER_CODE ="\
#version 330\n\
in vec3 positions;\n\
uniform vec3 color;\n\
out vec4 fragColor;\n\
void main()\n\
{\n\
  float alpha = 1.0;\n\
  if (positions.z != 0.0)\n\
  {\n\
    vec2 p = positions.xy;\n\
    // Gradients\n\
    vec2 px = dFdx(p);\n\
    vec2 py = dFdy(p);\n\
    // Chain rule\n\
    float fx = ((2.0*p.x)*px.x - px.y);\n\
    float fy = ((2.0*p.x)*py.x - py.y);\n\
    // Signed distance\n\
    float dist = fx*fx + fy*fy;\n\
    float sd = (p.x*p.x - p.y)*-positions.z/sqrt(dist);\n\
    // Linear alpha\n\
    alpha = clamp(0.5 - sd, 0.0, 1.0);\n\
  }\n\
  fragColor = alpha * vec4(color, 1.0);\n\
}\n\
";

static const char *TTF_VERTEX_SHADER_CODE = "\n\
#version 330\n\
in float t;\n\
in float c;\n\
in vec2 pos;\n\
out vec3 positions;\n\
void main(void)\n\
{\n\
  positions = vec3(t*0.5, max(t - 1.0, 0.0), c);\n\
  gl_Position = vec4(pos, 0.0, 1.0);\n\
}\n\
";

static void GLUIBuildStringShader()
{
  // text shader (contour)
  TEXT_VERTEX_SHADER = 
    glslCompileShader(GL_VERTEX_SHADER, TEXT_VERTEX_SHADER_CODE);
    GLCheckError("CREATE VERTEX SHADER");
  TEXT_GEOMETRY_SHADER = 
    glslCompileShader(GL_GEOMETRY_SHADER, TEXT_GEOMETRY_SHADER_CODE);
    GLCheckError("CREATE GEOMETRY SHADER");
  TEXT_FRAGMENT_SHADER = 
    glslCompileShader(GL_FRAGMENT_SHADER, TEXT_FRAGMENT_SHADER_CODE);
    GLCheckError("CREATE FRAGMENT SHADER");
  TEXT_PROGRAM_SHADER = 
    glslLinkProgram(TEXT_VERTEX_SHADER, 
      TEXT_GEOMETRY_SHADER,
      TEXT_FRAGMENT_SHADER);
    GLCheckError("CREATE PROGRAM SHADER");

  // fill shader (inside)
  FILL_VERTEX_SHADER = 
    glslCompileShader(GL_VERTEX_SHADER, FILL_VERTEX_SHADER_CODE);
    GLCheckError("CREATE VERTEX SHADER");
  FILL_FRAGMENT_SHADER = 
    glslCompileShader(GL_FRAGMENT_SHADER, FILL_FRAGMENT_SHADER_CODE);
    GLCheckError("CREATE FRAGMENT SHADER");
  FILL_GEOMETRY_SHADER = 
    glslCompileShader(GL_GEOMETRY_SHADER, FILL_GEOMETRY_SHADER_CODE);
    GLCheckError("CREATE GEOMETRY SHADER");
  FILL_PROGRAM_SHADER = 
    glslLinkProgram(FILL_VERTEX_SHADER, 
      /*FILL_GEOMETRY_SHADER,*/
      FILL_FRAGMENT_SHADER);
    GLCheckError("CREATE PROGRAM SHADER");

  // ttf shader 
  TTF_VERTEX_SHADER = 
    glslCompileShader(GL_VERTEX_SHADER, TTF_VERTEX_SHADER_CODE);
    GLCheckError("CREATE VERTEX SHADER");
  TTF_FRAGMENT_SHADER = 
    glslCompileShader(GL_FRAGMENT_SHADER, TTF_FRAGMENT_SHADER_CODE);
    GLCheckError("CREATE FRAGMENT SHADER");
  TTF_PROGRAM_SHADER = 
    glslLinkProgram(TTF_VERTEX_SHADER, TTF_FRAGMENT_SHADER);
    GLCheckError("CREATE PROGRAM SHADER");

  // ttf imple shader
  TTF_FRAGMENT_SIMPLE_SHADER = 
    glslCompileShader(GL_FRAGMENT_SHADER, TTF_FRAGMENT_SIMPLE_SHADER_CODE);
    GLCheckError("CREATE FRAGMENT SIMPLE SHADER");
  TTF_PROGRAM_SIMPLE_SHADER = 
    glslLinkProgram(TTF_VERTEX_SHADER, TTF_FRAGMENT_SIMPLE_SHADER);
    GLCheckError("CREATE PROGRAM SIMPLE SHADER");
}

static void GLUIDeleteStringShader()
{
  glUseProgram(0);
  glDeleteProgram(TEXT_PROGRAM_SHADER);
  glDeleteShader(TEXT_VERTEX_SHADER);
  glDeleteShader(TEXT_GEOMETRY_SHADER);
  glDeleteShader(TEXT_FRAGMENT_SHADER);

  glDeleteProgram(FILL_PROGRAM_SHADER);
  glDeleteShader(FILL_VERTEX_SHADER);
  glDeleteShader(FILL_FRAGMENT_SHADER);
  glDeleteShader(FILL_GEOMETRY_SHADER);

  glDeleteProgram(TTF_PROGRAM_SIMPLE_SHADER);
  glDeleteProgram(TTF_PROGRAM_SHADER);
  glDeleteShader(TTF_VERTEX_SHADER);
  glDeleteShader(TTF_FRAGMENT_SHADER);
  glDeleteShader(TTF_FRAGMENT_SIMPLE_SHADER);
}

static GLuint GetTextShaderProgram()
{
  return TEXT_PROGRAM_SHADER;
}

static GLuint GetFillShaderProgram()
{
  return FILL_PROGRAM_SHADER;
}

static GLuint GetTTFShaderProgram()
{
  return TTF_PROGRAM_SHADER;
}

static GLuint GetTTFShaderSimpleProgram()
{
  return TTF_PROGRAM_SIMPLE_SHADER;
}

AMN_NAMESPACE_CLOSE_SCOPE