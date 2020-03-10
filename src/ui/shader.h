#pragma once

#include "../utils/glsl.h"
#include "../utils/glutils.h"

AMN_NAMESPACE_OPEN_SCOPE

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
"	  float thickness;\n"
"	  vec4 color;\n"
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
"	  float thick1 = vertices[1].thickness;\n"
"	  float thick2 = vertices[2].thickness;\n"
"	  vec3 a = gl_in[0].gl_Position.xyz;\n"
"	  vec3 b = gl_in[1].gl_Position.xyz;\n"
"	  vec3 c = gl_in[2].gl_Position.xyz;\n"
"	  vec3 d = gl_in[3].gl_Position.xyz;\n"
"	  vec3 dir = c-b;\n"
"	  vec3 norm = normalize(cross(dir,vec3(0,0,1)));\n"
"	  vec3 tan1 = normalize(b-a);\n"
"	  vec3 tan2 = normalize(d-c);\n"
"	  vec3 miter1 = vec3(-tan1.y,tan1.x,0);\n"
"	  vec3 miter2 = vec3(-tan2.y,tan2.x,0);\n"
"	  float length1 = thick1/dot(norm,miter1);\n"
"	  float length2 = thick2/dot(norm,miter2);\n"
"	  fragColor = vertices[1].color;\n"
"	  gl_Position = vec4(b-norm*thick1,1);\n"
"   EmitVertex();\n"
"	  gl_Position = vec4(b+norm*thick1,1);\n"
"   EmitVertex();\n"
"	  fragColor = vertices[2].color;\n"
"	  gl_Position = vec4(c-norm*thick2,1);\n"
"   EmitVertex();\n"
"	  gl_Position = vec4(c+norm*thick2,1);\n"
"   EmitVertex();\n"
"	  EndPrimitive();\n"
"}\n";


// text fragment shader :
static const GLchar* TEXT_FRAGMENT_SHADER_CODE =
"#version 330\n"
"out vec4 outColor;\n"
"in vec4 fragColor;\n"
"void main(){\n"
"	  outColor = vec4(fragColor);\n"
"}\n";


static GLuint FILL_VERTEX_SHADER;
static GLuint FILL_FRAGMENT_SHADER;
static GLuint FILL_PROGRAM_SHADER;

// fill vertex shader :
static const GLchar* FILL_VERTEX_SHADER_CODE =
"#version 330\n"
"layout(location = 0) in vec4 datas;\n"
"out vec4 color;\n"
"vec4 unpackColor(float f) {\n"
"   vec4 color;\n"
"   color.r = floor(f / 256.0 / 256.0);\n"
"   color.g = floor((f - color.r * 256.0 * 256.0) / 256.0);\n"
"   color.b = floor(f - color.r * 256.0 * 256.0 - color.g * 256.0);\n"
"	  color.a = 255;\n"
"   return color / 256.0;\n"
"}\n"
"void main() {\n"
"    color = unpackColor(datas.a);\n"
"    gl_Position = vec4(datas.xy, 0.0, 1.0);\n"
"}\n";

// fill fragment shader :
static const GLchar* FILL_FRAGMENT_SHADER_CODE =
"#version 330\n"
"in vec4 color;\n"
"out vec4 result;\n"
"void main() {\n"
"    result = color;\n"
"}\n";


static GLuint TTF_VERTEX_SHADER;
static GLuint TTF_FRAGMENT_SHADER;
static GLuint TTF_FRAGMENT_SIMPLE_SHADER;
static GLuint TTF_PROGRAM_SHADER;

const char *TTF_FRAGMENT_SIMPLE_SHADER_CODE = "                 \n\
#version 330                                                    \n\
in vec3 positions;                                              \n\
out fragColor;                                                  \n\
float round(float val)                                          \n\
{                                                               \n\
    return sign(val)*floor(abs(val)+0.5);                       \n\
}                                                               \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = round((tpos.x*tpos.x-tpos.y)*tpos.z+0.5);     \n\
    fragColor = alpha *vec4(1.0,1.0,1.0,1.0);                   \n\
}                                                               \n\
";


const char *TTF_FRAGMENT_SHADER_CODE ="                         \n\
#version 330                                                    \n\
in vec3 tpos;                                                   \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = 1.0;                                          \n\
    if (tpos.z != 0.0)                                          \n\
    {                                                           \n\
        vec2 p = tpos.xy;                                       \n\
        // Gradients                                            \n\
        vec2 px = dFdx(p);                                      \n\
        vec2 py = dFdy(p);                                      \n\
        // Chain rule                                           \n\
        float fx = ((2.0*p.x)*px.x - px.y);                     \n\
        float fy = ((2.0*p.x)*py.x - py.y);                     \n\
        // Signed distance                                      \n\
        float dist = fx*fx + fy*fy;                             \n\
        float sd = (p.x*p.x - p.y)*-tpos.z/sqrt(dist);          \n\
        // Linear alpha                                         \n\
        alpha = clamp(0.5 - sd, 0.0, 1.0);                      \n\
    }                                                           \n\
    gl_FragColor = alpha * vec4(1.0, 1.0, 1.0, 1.0);            \n\
/*                                                              \n\
    if (tpos.z == 1.0)                                          \n\
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);                \n\
    else if (tpos.z == 0.0)                                     \n\
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);                \n\
    else                                                        \n\
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);                \n\
*/                                                              \n\
}                                                               \n\
";

const char *TTF_VERTEX_SHADER_CODE = "                          \n\
attribute float t;                                              \n\
attribute float c;                                              \n\
attribute vec2 pos;                                             \n\
varying vec3 tpos;                                              \n\
void main(void)                                                 \n\
{                                                               \n\
    tpos = vec3(t*0.5, max(t - 1.0, 0.0), c);                   \n\
    gl_Position = gl_ModelViewProjectionMatrix * vec4(0.001*pos, 0.0, 1.0);\n\
}                                                               \n\
";

static void GLUIBuildTextShader()
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
  FILL_PROGRAM_SHADER = 
    glslLinkProgram(FILL_VERTEX_SHADER, FILL_FRAGMENT_SHADER);
    GLCheckError("CREATE PROGRAM SHADER");
}

static GLuint GetTextShaderProgram()
{
  return TEXT_PROGRAM_SHADER;
}

static GLuint GetFillShaderProgram()
{
  return FILL_PROGRAM_SHADER;
}

AMN_NAMESPACE_CLOSE_SCOPE