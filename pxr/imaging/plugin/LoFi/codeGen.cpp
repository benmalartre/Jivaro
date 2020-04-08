//
// Copyright 2020 benmalartre
//
// unlicensed
//

#include "pxr/base/tf/token.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/plugin/LoFi/tokens.h"
#include "pxr/imaging/plugin/LoFi/shaderCode.h"
#include "pxr/imaging/plugin/LoFi/codeGen.h"
#include "pxr/imaging/hio/glslfx.h"

PXR_NAMESPACE_OPEN_SCOPE

/*
static const char *VERTEX_SHADER_330[1] = {
  "#version 330 core                                        \n" 
  "uniform mat4 model;                                      \n"
  "uniform mat4 view;                                       \n"
  "uniform mat4 projection;                                 \n"
  "                                                         \n"
  "in vec3 position;                                        \n"
  "in vec3 normal;                                          \n"
  "in vec3 color;                                           \n"
  "out vec3 vertex_color;                                   \n"
  "out vec3 vertex_normal;                                  \n"
  "void main(){                                             \n"
  "    vertex_normal = (model * vec4(normal, 0.0)).xyz;     \n"
  "    vertex_color = color;                                \n"
  "    vec3 p = vec3(view * model * vec4(position,1.0));    \n"
  "    gl_Position = projection * vec4(p,1.0);              \n"
  "}"
};

static const char *FRAGMENT_SHADER_120[1] = {
  "#version 120                                             \n"
  "varying vec3 vertex_normal;                              \n"
  "varying vec3 vertex_color;                               \n"
  "void main()                                              \n"
  "{                                                        \n"
  " vec3 color = vertex_normal * 0.5 + vertex_color * 0.5;  \n"
  "	gl_FragColor = vec4(color,1.0);                         \n"
  "}"
};

static const char *FRAGMENT_SHADER_330[1] = {
  "#version 330 core                                        \n"
  "in vec3 vertex_color;                                    \n"
  "in vec3 vertex_normal;                                   \n"
  "out vec4 outColor;                                       \n"
  "void main()                                              \n"
  "{                                                        \n"
  "	outColor = vec4(vertex_normal,1.0);                     \n"
  "}"
};
*/

static TfToken LoFiGetAttributeChannelName(LoFiAttributeChannel channel)
{
   switch(channel)
   {
    case CHANNEL_POSITION:
      return LoFiBufferTokens->position;
    case CHANNEL_NORMAL:
      return LoFiBufferTokens->normal;
    case CHANNEL_TANGENT:
      return LoFiBufferTokens->tangent;
    case CHANNEL_COLOR:
      return LoFiBufferTokens->color;
    case CHANNEL_UV:
      return LoFiBufferTokens->uv;
    case CHANNEL_WIDTH:
      return LoFiBufferTokens->width;
    case CHANNEL_ID:
      return LoFiBufferTokens->id;
    case CHANNEL_SCALE:
      return LoFiBufferTokens->scale;
    case CHANNEL_SHAPE_POSITION:
      return LoFiBufferTokens->shape_position;
    case CHANNEL_SHAPE_NORMAL:
      return LoFiBufferTokens->shape_normal;
    case CHANNEL_SHAPE_UV:
      return LoFiBufferTokens->shape_position;
    case CHANNEL_SHAPE_COLOR:
      return LoFiBufferTokens->shape_position;
    default:
      return TfToken();
   }
}

static TfToken LoFiGetAttributeChannelType(LoFiAttributeChannel channel)
{
   switch(channel)
   {
    case CHANNEL_POSITION:
    case CHANNEL_NORMAL:
    case CHANNEL_TANGENT:
    case CHANNEL_COLOR:
    case CHANNEL_SCALE:
    case CHANNEL_SHAPE_POSITION:
    case CHANNEL_SHAPE_NORMAL:
    case CHANNEL_SHAPE_COLOR:
      return LoFiGLTokens->vec3;
    case CHANNEL_UV:
    case CHANNEL_SHAPE_UV:
      return LoFiGLTokens->vec2;
    case CHANNEL_WIDTH:
      return LoFiGLTokens->_float;
    case CHANNEL_ID:
      return LoFiGLTokens->_int;
    default:
      return TfToken();
   }
}

static std::string _GetSwizzleString(TfToken const& type, 
                                     std::string const& swizzle=std::string())
{
    if (!swizzle.empty()) {
        return "." + swizzle;
    } 
    if (type == LoFiGLTokens->vec4 || type == LoFiGLTokens->ivec4) {
        return "";
    }
    if (type == LoFiGLTokens->vec3 || type == LoFiGLTokens->ivec3) {
        return ".xyz";
    }
    if (type == LoFiGLTokens->vec2 || type == LoFiGLTokens->ivec2) {
        return ".xy";
    }
    if (type == LoFiGLTokens->_float || type == LoFiGLTokens->_int) {
        return ".x";
    }

    return "";
}

static int _GetNumComponents(TfToken const& type)
{
    int numComponents = 1;
    if (type == LoFiGLTokens->vec2 || type == LoFiGLTokens->ivec2) {
        numComponents = 2;
    } else if (type == LoFiGLTokens->vec3 || type == LoFiGLTokens->ivec3) {
        numComponents = 3;
    } else if (type == LoFiGLTokens->vec4 || type == LoFiGLTokens->ivec4) {
        numComponents = 4;
    } else if (type == LoFiGLTokens->mat3) {
        numComponents = 9;
    } else if (type == LoFiGLTokens->mat4) {
        numComponents = 16;
    }

    return numComponents;
}

/// Constructor.
LoFiCodeGen::LoFiCodeGen(LoFiGeometricProgramType type, 
  const LoFiShaderCodeSharedPtrList& shaders)
  : _type(type)
  , _shaders(shaders)
{
  const GlfContextCaps& caps = GlfContextCaps::GetInstance();
  _glslVersion = caps.glslVersion;
}

LoFiCodeGen::LoFiCodeGen(LoFiGeometricProgramType type, 
            const LoFiBindingList& uniformBindings,
            const LoFiBindingList& vertexBufferBindings,
            const LoFiShaderCodeSharedPtrList& shaders)
  : _type(type)
  , _uniformBindings(uniformBindings)
  , _attributeBindings(vertexBufferBindings)
  , _shaders(shaders)
{
  const GlfContextCaps& caps = GlfContextCaps::GetInstance();
  _glslVersion = caps.glslVersion;
}

void LoFiCodeGen::_EmitDeclaration(std::stringstream &ss,
                                    TfToken const &name,
                                    TfToken const &type,
                                    LoFiBinding const &binding,
                                    size_t arraySize)
{
  LoFiBindingType bindingType = binding.type;

  if (!TF_VERIFY(!name.IsEmpty())) return;
  if (!TF_VERIFY(!type.IsEmpty(),
                    "Unknown dataType for %s",
                    name.GetText())) return;

  if (arraySize > 0) 
  {
    if (!TF_VERIFY(bindingType == LoFiBindingType::UNIFORM_ARRAY))
        return;
  }

  switch (bindingType) 
  {
  case LoFiBindingType::VERTEX:
    if(_glslVersion >= 330)
    {
      //ss << "layout (location = " << binding.location << ") in ";

      ss <<"in "<< type << " " << name << ";\n";
      break;
    }
    else
    {
      ss << "attribute " << type << " " << name << ";\n";
      break;
    }
  case LoFiBindingType::UNIFORM:
      ss << "uniform " << type << " " << name << ";\n";
      break;
  case LoFiBindingType::UNIFORM_ARRAY:
      ss << "uniform " << type << " " << name
          << "[" << arraySize << "];\n";
      break;
  default:
      TF_CODING_ERROR("Unknown binding type %d, for %s\n",
                      binding.type, name.GetText());
      break;
  }
}

void LoFiCodeGen::_EmitAccessor(std::stringstream &ss,
                                TfToken const &name,
                                TfToken const &type,
                                LoFiBinding const &binding,
                                const char *index)
{
  if (index) 
  {
    ss << type << " LOFI_GET_" << name << "(int localIndex) {\n"
        << "  int index = " << index << ";\n";
    if (binding.type == LoFiBindingType::TBO)
    {
      ss << "  return "
          << type
          << "(texelFetch(" << name << ", index)"
          << _GetSwizzleString(type) << ");\n}\n";
    } 
    else 
    {
      ss << "  return " << name << "[index];\n}\n";
    }
  } 
  else 
  {
    /*
    if (binding.type == LoFiBindingType::UNIFORM || 
        binding.type == LoFiBindingType::VERTEX) 
    {
      ss << type
          << " LOFI_GET_" << name << "(int localIndex) { ";
      ss << "return " << name << ";}\n";
    }
    */
  }

  ss << type << " LOFI_GET_" << name << "()"
      << " { return " << name << "; }\n";
}

void LoFiCodeGen::_EmitStructAccessor(std::stringstream &ss,
                                      TfToken const &structName,
                                      TfToken const &name,
                                      TfToken const &type,
                                      int arraySize,
                                      const char *index)
{ 
  if (index) 
  {
    if (arraySize > 1) 
    {
      ss << type << " LOFI_GET_" << name
          << "(int arrayIndex, int localIndex) {\n"
          << "  int index = " << index << ";\n"
          << "  return "
          << structName << "[index]." << name << "[arrayIndex];\n}\n";
    } 
    else 
    {
      ss << type << " LOFI_GET_" << name
          << "(int localIndex) {\n"
          << "  int index = " << index << ";\n"
          << "  return "
          << structName << "[index]." << name << ";\n}\n";
    }
  }
  else 
  {
    if (arraySize > 1) 
    {
      ss << type << " LOFI_GET_" << name
          << "(int arrayIndex, int localIndex) { return "
          << structName << "." << name << "[arrayIndex];}\n";
    } 
    else 
    {
      ss << type << " LOFI_GET_" << name
          << "(int localIndex) { return "
          << structName << "." << name << ";}\n";
    }
  }

  if (arraySize > 1) 
  {
    ss << type << " LOFI_GET_" << name
        << "(int arrayIndex)"
        << " { return LOFI_GET_" << name << "(arrayIndex, 0); }\n";
  } 
  else 
  {
    ss << type << " LOFI_GET_" << name << "()"
        << " { return LOFI_GET_" << name << "(0); }\n";
  }
}

void 
LoFiCodeGen::_GenerateVersion()
{
  if(_glslVersion >= 330)
  {
    _genCommon << "#version 330 core \n";
    _genCommon << "#define LOFI_GLSL_330 1\n";
  }
  else
  {
    _genCommon << "#version 120 \n";
  }
}

void 
LoFiCodeGen::_GenerateCommon()
{
  _genVS << _genCommon.str();
  _genGS << _genCommon.str();
  _genFS << _genCommon.str();
}

void 
LoFiCodeGen::_GenerateConstants()
{
  LoFiShaderCodeSharedPtr shaderCode = _shaders[0];

  std::string constantCode = shaderCode->GetSource(LoFiShaderTokens->common);
  std::cout << ":D \n" << constantCode << std::endl;

  _genVS << constantCode;
  _genGS << constantCode;
  _genFS << constantCode;
}

void
LoFiCodeGen::_GeneratePrimvars(bool hasGeometryShader)
{
  std::stringstream vertexInputs;
  std::stringstream interstageVertexData;
  std::stringstream accessorsVS, accessorsGS, accessorsFS;

  // vertex varying
  TF_FOR_ALL (it, _attributeBindings) 
  {
    TfToken const &name = it->name;
    TfToken const &dataType = it->dataType;

    _EmitDeclaration(vertexInputs, name, dataType, *it);

    interstageVertexData << "  " << dataType
                          << " " << name << ";\n";

    // primvar accessors
    _EmitAccessor(accessorsVS, name, dataType, *it);

    _EmitStructAccessor(accessorsGS,  LoFiBufferTokens->inPrimvars,
                        name, dataType, 1, "localIndex");

    _EmitStructAccessor(accessorsFS,  LoFiBufferTokens->inPrimvars,
                        name, dataType, 1);

    // interstage plumbing
    _procVS << "  " << LoFiBufferTokens->outPrimvars << "." << name
            << " = " << name << ";\n";

    _procGS  << "  " << LoFiBufferTokens->outPrimvars << "." << name
              << " = " << LoFiBufferTokens->inPrimvars << "[index]." << name << ";\n";
  }

  // face varying
  std::stringstream fvarDeclarations;
  std::stringstream interstageFVarData;

/*
  if (hasGS) {
    // FVar primvars are emitted only by the GS.
    // If the GS isn't active, we can skip processing them.
    TF_FOR_ALL (it, _metaData.fvarData) {
      HdBinding binding = it->first;
      TfToken const &name = it->second.name;
      TfToken const &dataType = it->second.dataType;

      _EmitDeclaration(fvarDeclarations, name, dataType, binding);

      interstageFVarData << "  " << _GetPackedType(dataType, false)
                          << " " << name << ";\n";

      // primvar accessors (only in GS and FS)
      _EmitFVarGSAccessor(accessorsGS, name, dataType, binding,
                          _geometricShader->GetPrimitiveType());
      _EmitStructAccessor(accessorsFS, _tokens->inPrimvars, name, dataType,
                          1, NULL);

      _procGS << "  outPrimvars." << name 
                                  <<" = HdGet_" << name << "(index);\n";
    }
  }
  */
  if (!interstageVertexData.str().empty()) 
  {
    _genVS  << vertexInputs.str()
            << "out Primvars {\n"
            << interstageVertexData.str()
            << "} outPrimvars;\n"
            << accessorsVS.str();

    _genGS  << fvarDeclarations.str()
            << "in Primvars {\n"  
            << interstageVertexData.str()
            << "} inPrimvars[LOFI_NUM_PRIMITIVE_VERTS];\n"
            << "out Primvars {\n"
            << interstageVertexData.str()
            << interstageFVarData.str()
            << "} outPrimvars;\n"
            << accessorsGS.str();

    _genFS  << "in Primvars {\n"
            << interstageVertexData.str()
            << interstageFVarData.str()
            << "} inPrimvars;\n"
            << accessorsFS.str();
  }
}

void
LoFiCodeGen::_GenerateUniforms()
{
  std::stringstream uniformInputs;
  std::stringstream accessorsCommon;

  // vertex varying
  TF_FOR_ALL (it, _uniformBindings) {
    TfToken const &name = it->name;
    TfToken const &dataType = it->dataType;

    _EmitDeclaration(uniformInputs, name, dataType, *it);
    // uniform accessors
    _EmitAccessor(accessorsCommon, name, dataType, *it);
  }

  _genVS  << uniformInputs.str()
          << accessorsCommon.str();

  _genGS  << uniformInputs.str()
          << accessorsCommon.str();

  _genFS  << uniformInputs.str()
          << accessorsCommon.str();
}


void LoFiCodeGen::GenerateProgramCode()
{
  LoFiShaderCodeSharedPtr shaderCode = _shaders[1];

  // shader sources which own main()
  std::string vertexCode = shaderCode->GetSource(LoFiShaderTokens->vertex);
  std::cout << ":D \n" << vertexCode << std::endl;
  std::string fragmentCode = shaderCode->GetSource(LoFiShaderTokens->fragment);
  std::cout << ":D \n" << fragmentCode << std::endl;

  // initialize autogen source buckets
  _genCommon.str(""), _genVS.str(""), _genGS.str(""), _genFS.str("");
  _procVS.str(""), _procGS.str("");

  _GenerateVersion();
  
  
  TF_FOR_ALL (it, _attributeBindings) {
    _genCommon << "#define LOFI_HAS_" << it->name << " 1\n";
  }

  TF_FOR_ALL (it, _uniformBindings) {
    _genCommon << "#define LOFI_HAS_" << it->name << " 1\n";
  }

  _GenerateCommon();
  _GenerateUniforms();
  _GenerateConstants();
  _GeneratePrimvars(true);

  _genVS << vertexCode;
  _genFS << fragmentCode;
  
  
  std::cout << "-------------------------------------------" << std::endl;
  std::cout << "COMMON CODE : " << std::endl;
  std::cout << _genCommon.str() << std::endl;
  std::cout << "-------------------------------------------" << std::endl;
  std::cout << "VERTEX SHADER : " << std::endl;
  std::cout << _genVS.str() << std::endl;
  /*
  std::cout << "-------------------------------------------" << std::endl;
  std::cout << "GEOMETRY SHADER : " << std::endl;
  std::cout << _genGS.str() << std::endl;
  */
  std::cout << "-------------------------------------------" << std::endl;
  std::cout << "FRAGMENT SHADER : " << std::endl;
  std::cout << _genFS.str() << std::endl;

  _vertexCode = _genVS.str();
  _geometryCode = _genGS.str();
  _fragmentCode = _genFS.str();
}

/*
  "varying vec3 vertex_color;                               \n"
  "void main(){                                             \n"
  "    vertex_normal = (model * vec4(normal, 0.0)).xyz;     \n"
  "    vertex_color = color;                                \n"
  "    vec3 p = vec3(view * model * vec4(position,1.0));    \n"
  "    gl_Position = projection * vec4(p,1.0);              \n"
  "}"
};
*/

PXR_NAMESPACE_CLOSE_SCOPE