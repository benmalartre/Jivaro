

#include "../utils/usd.h"

JVR_NAMESPACE_OPEN_SCOPE

// Return all the value types.
// In the next version 22.03 we should be able to use:
// SdfSchema::GetInstance().GetAllTypes();
const std::array<SdfValueTypeName, 106> & GetAllValueTypeNames() {
  static std::array<SdfValueTypeName, 106> allValueTypeNames = {
    SdfValueTypeNames->Bool,
    SdfValueTypeNames->UChar,
    SdfValueTypeNames->Int,
    SdfValueTypeNames->UInt,
    SdfValueTypeNames->Int64,
    SdfValueTypeNames->UInt64,
    SdfValueTypeNames->Half,
    SdfValueTypeNames->Float,
    SdfValueTypeNames->Double,
    SdfValueTypeNames->TimeCode,
    SdfValueTypeNames->String,
    SdfValueTypeNames->Token,
    SdfValueTypeNames->Asset,
    SdfValueTypeNames->Int2,
    SdfValueTypeNames->Int3,
    SdfValueTypeNames->Int4,
    SdfValueTypeNames->Half2,
    SdfValueTypeNames->Half3,
    SdfValueTypeNames->Half4,
    SdfValueTypeNames->Float2,
    SdfValueTypeNames->Float3,
    SdfValueTypeNames->Float4,
    SdfValueTypeNames->Double2,
    SdfValueTypeNames->Double3,
    SdfValueTypeNames->Double4,
    SdfValueTypeNames->Point3h,
    SdfValueTypeNames->Point3f,
    SdfValueTypeNames->Point3d,
    SdfValueTypeNames->Vector3h,
    SdfValueTypeNames->Vector3f,
    SdfValueTypeNames->Vector3d,
    SdfValueTypeNames->Normal3h,
    SdfValueTypeNames->Normal3f,
    SdfValueTypeNames->Normal3d,
    SdfValueTypeNames->Color3h,
    SdfValueTypeNames->Color3f,
    SdfValueTypeNames->Color3d,
    SdfValueTypeNames->Color4h,
    SdfValueTypeNames->Color4f,
    SdfValueTypeNames->Color4d,
    SdfValueTypeNames->Quath,
    SdfValueTypeNames->Quatf,
    SdfValueTypeNames->Quatd,
    SdfValueTypeNames->Matrix2d,
    SdfValueTypeNames->Matrix3d,
    SdfValueTypeNames->Matrix4d,
    SdfValueTypeNames->Frame4d,
    SdfValueTypeNames->TexCoord2h,
    SdfValueTypeNames->TexCoord2f,
    SdfValueTypeNames->TexCoord2d,
    SdfValueTypeNames->TexCoord3h,
    SdfValueTypeNames->TexCoord3f,
    SdfValueTypeNames->TexCoord3d,
    SdfValueTypeNames->BoolArray,
    SdfValueTypeNames->UCharArray,
    SdfValueTypeNames->IntArray,
    SdfValueTypeNames->UIntArray,
    SdfValueTypeNames->Int64Array,
    SdfValueTypeNames->UInt64Array,
    SdfValueTypeNames->HalfArray,
    SdfValueTypeNames->FloatArray,
    SdfValueTypeNames->DoubleArray,
    SdfValueTypeNames->TimeCodeArray,
    SdfValueTypeNames->StringArray,
    SdfValueTypeNames->TokenArray,
    SdfValueTypeNames->AssetArray,
    SdfValueTypeNames->Int2Array,
    SdfValueTypeNames->Int3Array,
    SdfValueTypeNames->Int4Array,
    SdfValueTypeNames->Half2Array,
    SdfValueTypeNames->Half3Array,
    SdfValueTypeNames->Half4Array,
    SdfValueTypeNames->Float2Array,
    SdfValueTypeNames->Float3Array,
    SdfValueTypeNames->Float4Array,
    SdfValueTypeNames->Double2Array,
    SdfValueTypeNames->Double3Array,
    SdfValueTypeNames->Double4Array,
    SdfValueTypeNames->Point3hArray,
    SdfValueTypeNames->Point3fArray,
    SdfValueTypeNames->Point3dArray,
    SdfValueTypeNames->Vector3hArray,
    SdfValueTypeNames->Vector3fArray,
    SdfValueTypeNames->Vector3dArray,
    SdfValueTypeNames->Normal3hArray,
    SdfValueTypeNames->Normal3fArray,
    SdfValueTypeNames->Normal3dArray,
    SdfValueTypeNames->Color3hArray,
    SdfValueTypeNames->Color3fArray,
    SdfValueTypeNames->Color3dArray,
    SdfValueTypeNames->Color4hArray,
    SdfValueTypeNames->Color4fArray,
    SdfValueTypeNames->Color4dArray,
    SdfValueTypeNames->QuathArray,
    SdfValueTypeNames->QuatfArray,
    SdfValueTypeNames->QuatdArray,
    SdfValueTypeNames->Matrix2dArray,
    SdfValueTypeNames->Matrix3dArray,
    SdfValueTypeNames->Matrix4dArray,
    SdfValueTypeNames->Frame4dArray,
    SdfValueTypeNames->TexCoord2hArray,
    SdfValueTypeNames->TexCoord2fArray,
    SdfValueTypeNames->TexCoord2dArray,
    SdfValueTypeNames->TexCoord3hArray,
    SdfValueTypeNames->TexCoord3fArray,
    SdfValueTypeNames->TexCoord3dArray,
  };
  return allValueTypeNames;
}


// Return all the prim types from the registry.
// There is no function in USD to simply retrieve the list, this is explained in the forum:
// https://groups.google.com/g/usd-interest/c/q8asqMYuyeg/m/sRhFTIEfCAAJ
const std::vector<std::string> &GetAllSpecTypeNames() {
  static std::vector<std::string> allSpecTypeNames;
  static std::once_flag called_once;
  std::call_once(called_once, [&]() {
    allSpecTypeNames.push_back("Empty"); // empty type
    const TfType baseType = TfType::Find<UsdTyped>();
    std::set<TfType> schemaTypes;
    PlugRegistry::GetAllDerivedTypes(baseType, &schemaTypes);
    TF_FOR_ALL(type, schemaTypes) { allSpecTypeNames.push_back(UsdSchemaRegistry::GetInstance().GetSchemaTypeName(*type)); }
    std::sort(allSpecTypeNames.begin(), allSpecTypeNames.end());
  });
  return allSpecTypeNames;
}

JVR_NAMESPACE_CLOSE_SCOPE

