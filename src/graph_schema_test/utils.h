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
#ifndef USD_AMNGRAPH_UTILS_H
#define USD_AMNGRAPH_UTILS_H

#include "pxr/pxr.h"
#include "api.h"

#include "pxr/base/tf/token.h"

#include <string>
#include <utility>

PXR_NAMESPACE_OPEN_SCOPE


/// \enum GraphAttributeType
/// 
/// Specifies the type of a shading attribute.
/// 
/// "Parameter" and "InterfaceAttribute" are deprecated shading attribute types.
/// 
enum class GraphAttributeType {
    Invalid,
    Input,
    Output,
    Parameter,
    InterfaceAttribute
};

/// \class GraphUtils
///
/// This class contains a set of utility functions used when authoring and 
/// querying shading networks.
///
class GraphUtils {
public:
    /// Returns the namespace prefix of the USD attribute associated with the given
    /// shading attribute type.
    GRAPH_API
    static std::string GetPrefixForAttributeType(
        GraphAttributeType sourceType);

    /// Given the full name of a shading property, returns it's base name and type.
    GRAPH_API
    static std::pair<TfToken, GraphAttributeType> 
        GetBaseNameAndType(const TfToken &fullName);

    /// Returns the full shading attribute name given the basename and the type.
    /// \p baseName is the name of the input or output on the shading node.
    /// \p type is the \ref GraphAttributeType of the shading attribute.
    GRAPH_API
    static TfToken GetFullName(const TfToken &baseName, 
                               const GraphAttributeType type);
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
