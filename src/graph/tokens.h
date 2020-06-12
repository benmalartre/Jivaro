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
#ifndef GRAPH_TOKENS_H
#define GRAPH_TOKENS_H

/// \file Graph/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class GraphTokensType
///
/// \link GraphTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// GraphTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use GraphTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(GraphTokens->allRule);
/// \endcode
struct GraphTokensType {
    GRAPH_API GraphTokensType();
    /// \brief "AllRule"
    /// 
    /// Possible value for GraphStage::GetLoadPrimsStatesAttr()
    const TfToken allRule;
    /// \brief "animationCache"
    /// 
    /// When in 'animationCache state, the asset will exhibit the animation cache only. 
    const TfToken animationCache;
    /// \brief "animCache"
    /// 
    /// The animation cache variation described on a Deformable.
    const TfToken animCache;
    /// \brief "animDatas"
    /// 
    /// The animation datas variation described on a Deformable.
    const TfToken animDatas;
    /// \brief "animRig"
    /// 
    /// When in 'animRig state, the asset will exhibit it's animation controls and authored animation curves. , The animation rig variation described on a Deformable.
    const TfToken animRig;
    /// \brief "cfxCache"
    /// 
    /// When in 'cfxCache state, the asset will exhibit the animation cache with the cfx cache override on top. , The characterFX cache variation described on a Deformable.
    const TfToken cfxCache;
    /// \brief "cfxDatas"
    /// 
    /// The characterFX datas variation described on a Deformable.
    const TfToken cfxDatas;
    /// \brief "cfxRig"
    /// 
    /// When in 'cfxRig state, the asset will exhibit the animation cache + the cfx rig to be simulated/sculpted on top. , The character fx rig variation described on a Deformable.
    const TfToken cfxRig;
    /// \brief "connectedSourceFor:"
    /// 
    /// The prefix on Asset relationships associated with a Parameter.  This prefixed relationship has a suffix matching the associated attribute name, and denotes a logical connection between AssetNodes. 
    const TfToken connectedSourceFor;
    /// \brief "coordSys:"
    /// 
    /// Namespace prefix for relationships that bind coordinate systems.
    const TfToken coordSys;
    /// \brief "deformed"
    /// 
    /// Describes the <i>deformed geometry</i> output terminal  on a Deformable. It is used to output the resulting geometry to upstream the graph.
    const TfToken deformed;
    /// \brief "derivesFrom"
    /// 
    /// A legacy relationship name specifying a specializes  composition on a Deformable.
    const TfToken derivesFrom;
    /// \brief "fileName"
    /// 
    /// GraphLayer, GraphStage
    const TfToken fileName;
    /// \brief "full"
    /// 
    /// Possible value for 'connectability' metadata on a InputPort. When connectability of an input/output is set to "full", it implies that it can be connected to any other port. 
    const TfToken full;
    /// \brief "geometry"
    /// 
    /// The modeling variation described on a Deformable.
    const TfToken geometry;
    /// \brief "in-memory"
    /// 
    /// Possible value for GraphStage::GetLifetimeManagementAttr()
    const TfToken inMemory;
    /// \brief "inputs:"
    /// 
    /// The prefix on input ports. 
    const TfToken inputs;
    /// \brief "inputs:geometry"
    /// 
    /// GraphDeformable
    const TfToken inputsGeometry;
    /// \brief "inputs:layers"
    /// 
    /// GraphStage
    const TfToken inputsLayers;
    /// \brief "interfaceOnly"
    /// 
    /// Possible value for 'connectability' metadata on  a InputPort. It implies that the input can only connect to  a Graph Input (which represents an interface override, not  a computation-time dataflow connection), or another Input whose  connectability is also 'interfaceOnly'. 
    const TfToken interfaceOnly;
    /// \brief "lifetimeManagement"
    /// 
    /// GraphStage
    const TfToken lifetimeManagement;
    /// \brief "loadPrimsPath"
    /// 
    /// GraphStage
    const TfToken loadPrimsPath;
    /// \brief "loadPrimsStates"
    /// 
    /// GraphStage
    const TfToken loadPrimsStates;
    /// \brief "NoneRule"
    /// 
    /// Possible value for GraphStage::GetLoadPrimsStatesAttr()
    const TfToken noneRule;
    /// \brief "on-disk"
    /// 
    /// Possible value for GraphStage::GetLifetimeManagementAttr()
    const TfToken onDisk;
    /// \brief "OnlyRule"
    /// 
    /// Possible value for GraphStage::GetLoadPrimsStatesAttr()
    const TfToken onlyRule;
    /// \brief "outputs:"
    /// 
    /// The prefix on output ports. 
    const TfToken outputs;
    /// \brief "outputs:deformed"
    /// 
    /// GraphDeformable
    const TfToken outputsDeformed;
    /// \brief "outputs:result"
    /// 
    /// GraphLayer
    const TfToken outputsResult;
    /// \brief "populationMask"
    /// 
    /// GraphStage
    const TfToken populationMask;
    /// \brief "state:binding"
    /// 
    ///  The current state of the asset. 
    const TfToken stateBinding;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var GraphTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa GraphTokensType
extern GRAPH_API TfStaticData<GraphTokensType> GraphTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
