//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "./tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

GraphTokensType::GraphTokensType() :
    animationCache("animationCache", TfToken::Immortal),
    animCache("animCache", TfToken::Immortal),
    animDatas("animDatas", TfToken::Immortal),
    animRig("animRig", TfToken::Immortal),
    cfxCache("cfxCache", TfToken::Immortal),
    cfxDatas("cfxDatas", TfToken::Immortal),
    cfxRig("cfxRig", TfToken::Immortal),
    connectedSourceFor("connectedSourceFor:", TfToken::Immortal),
    coordSys("coordSys:", TfToken::Immortal),
    deformed("deformed", TfToken::Immortal),
    derivesFrom("derivesFrom", TfToken::Immortal),
    full("full", TfToken::Immortal),
    geometry("geometry", TfToken::Immortal),
    id("id", TfToken::Immortal),
    infoId("info:id", TfToken::Immortal),
    infoImplementationSource("info:implementationSource", TfToken::Immortal),
    inputs("inputs:", TfToken::Immortal),
    interface_("interface:", TfToken::Immortal),
    interfaceOnly("interfaceOnly", TfToken::Immortal),
    interfaceRecipientsOf("interfaceRecipientsOf:", TfToken::Immortal),
    outputs("outputs:", TfToken::Immortal),
    outputsDeformed("outputs:deformed", TfToken::Immortal),
    sdrMetadata("sdrMetadata", TfToken::Immortal),
    sourceAsset("sourceAsset", TfToken::Immortal),
    sourceCode("sourceCode", TfToken::Immortal),
    stateBinding("state:binding", TfToken::Immortal),
    universalSourceType("", TfToken::Immortal),
    allTokens({
        animationCache,
        animCache,
        animDatas,
        animRig,
        cfxCache,
        cfxDatas,
        cfxRig,
        connectedSourceFor,
        coordSys,
        deformed,
        derivesFrom,
        full,
        geometry,
        id,
        infoId,
        infoImplementationSource,
        inputs,
        interface_,
        interfaceOnly,
        interfaceRecipientsOf,
        outputs,
        outputsDeformed,
        sdrMetadata,
        sourceAsset,
        sourceCode,
        stateBinding,
        universalSourceType
    })
{
}

TfStaticData<GraphTokensType> GraphTokens;

PXR_NAMESPACE_CLOSE_SCOPE
