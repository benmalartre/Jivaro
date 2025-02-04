//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/usd/usdExec/execNode.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdExecNode,
        TfType::Bases< UsdTyped > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("ExecNode")
    // to find TfType<UsdExecNode>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdExecNode>("ExecNode");
}

/* virtual */
UsdExecNode::~UsdExecNode()
{
}

/* static */
UsdExecNode
UsdExecNode::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdExecNode();
    }
    return UsdExecNode(stage->GetPrimAtPath(path));
}

/* static */
UsdExecNode
UsdExecNode::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("ExecNode");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdExecNode();
    }
    return UsdExecNode(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdExecNode::_GetSchemaKind() const
{
    return UsdExecNode::schemaKind;
}

/* static */
const TfType &
UsdExecNode::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdExecNode>();
    return tfType;
}

/* static */
bool 
UsdExecNode::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdExecNode::_GetTfType() const
{
    return _GetStaticTfType();
}

/*static*/
const TfTokenVector&
UsdExecNode::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames;
    static TfTokenVector allNames =
        UsdTyped::GetSchemaAttributeNames(true);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

#include "pxr/usd/usdExec/execConnectableAPI.h"
#include "pxr/usd/usdExec/execNode.h"
#include "pxr/usd/usdExec/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE


UsdExecNode::UsdExecNode(const UsdExecConnectableAPI &connectable)
    : UsdExecNode(connectable.GetPrim())
{
}

UsdExecConnectableAPI 
UsdExecNode::ConnectableAPI() const
{
    return UsdExecConnectableAPI(GetPrim());
}

UsdExecOutput
UsdExecNode::CreateOutput(const TfToken& name,
                             const SdfValueTypeName& typeName)
{
    return UsdExecConnectableAPI(GetPrim()).CreateOutput(name, typeName);
}

UsdExecOutput
UsdExecNode::GetOutput(const TfToken &name) const
{
    return UsdExecConnectableAPI(GetPrim()).GetOutput(name);
}

std::vector<UsdExecOutput>
UsdExecNode::GetOutputs(bool onlyAuthored) const
{
    return UsdExecConnectableAPI(GetPrim()).GetOutputs(onlyAuthored);
}

UsdExecInput
UsdExecNode::CreateInput(const TfToken& name,
                            const SdfValueTypeName& typeName)
{
    return UsdExecConnectableAPI(GetPrim()).CreateInput(name, typeName);
}

UsdExecInput
UsdExecNode::GetInput(const TfToken &name) const
{
    return UsdExecConnectableAPI(GetPrim()).GetInput(name);
}

std::vector<UsdExecInput>
UsdExecNode::GetInputs(bool onlyAuthored) const
{
    return UsdExecConnectableAPI(GetPrim()).GetInputs(onlyAuthored);
}


NdrTokenMap
UsdExecNode::GetExecMetadata() const
{
    NdrTokenMap result;

    VtDictionary execMetadata;
    if (GetPrim().GetMetadata(UsdExecTokens->execMetadata, &execMetadata)){
        for (const auto &it : execMetadata) {
            result[TfToken(it.first)] = TfStringify(it.second);
        }
    }

    return result;
}

std::string 
UsdExecNode::GetExecMetadataByKey(const TfToken &key) const
{
    VtValue val;
    GetPrim().GetMetadataByDictKey(UsdExecTokens->execMetadata, key, &val);
    return TfStringify(val);
}
    
void 
UsdExecNode::SetExecMetadata(const NdrTokenMap &execMetadata) const
{
    for (auto &i: execMetadata) {
        SetExecMetadataByKey(i.first, i.second);
    }
}

void 
UsdExecNode::SetExecMetadataByKey(
    const TfToken &key, 
    const std::string &value) const
{
    GetPrim().SetMetadataByDictKey(UsdExecTokens->execMetadata, key, value);
}

bool 
UsdExecNode::HasExecMetadata() const
{
    return GetPrim().HasMetadata(UsdExecTokens->execMetadata);
}

bool 
UsdExecNode::HasExecMetadataByKey(const TfToken &key) const
{
    return GetPrim().HasMetadataDictKey(UsdExecTokens->execMetadata, key);
}

void 
UsdExecNode::ClearExecMetadata() const
{
    GetPrim().ClearMetadata(UsdExecTokens->execMetadata);
}

void
UsdExecNode::ClearExecMetadataByKey(const TfToken &key) const
{
    GetPrim().ClearMetadataByDictKey(UsdExecTokens->execMetadata, key);
}

PXR_NAMESPACE_CLOSE_SCOPE
