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
#include "pxr/pxr.h"
#include "utils.h"
#include "tokens.h"
#include "pxr/usd/sdf/path.h"

#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/stringUtils.h"

#include <string>

AMN_NAMESPACE_OPEN_SCOPE

TF_DEFINE_ENV_SETTING(
    USD_SHADE_READ_OLD_ENCODING, true,
    "Set to false to disable support for reading old-style of encoding with "
    "parameters, interface attributes and terminals.");

TF_DEFINE_ENV_SETTING(
    USD_SHADE_WRITE_NEW_ENCODING, true,
    "Set to true to enable the authoring of shading networks using the new "
    "encoding (with inputs and outputs, in place of parameters, interface "
    "attributes and terminals.");

using std::vector;
using std::string;

/* static */
string 
GraphUtils::GetPrefixForAttributeType(GraphAttributeType sourceType)
{
    switch (sourceType) {
        case GraphAttributeType::Input:
            return AmnGraphTokens->inputs.GetString();
        case GraphAttributeType::Output:
            return AmnGraphTokens->outputs.GetString();
        case GraphAttributeType::Parameter: 
            return string();
        case GraphAttributeType::InterfaceAttribute:
            return AmnGraphTokens->interface_.GetString();
        default:
            return string();
    }
}

/* static */
std::pair<TfToken, GraphAttributeType> 
GraphUtils::GetBaseNameAndType(const TfToken &fullName)
{
    std::pair<std::string, bool> res = 
        SdfPath::StripPrefixNamespace(fullName, AmnGraphTokens->inputs);
    if (res.second) {
        return std::make_pair(TfToken(res.first), GraphAttributeType::Input);
    }

    res = SdfPath::StripPrefixNamespace(fullName, AmnGraphTokens->outputs);
    if (res.second) {
        return std::make_pair(TfToken(res.first),GraphAttributeType::Output);
    }

    res = SdfPath::StripPrefixNamespace(fullName, AmnGraphTokens->interface_);
    if (res.second) {
        return std::make_pair(TfToken(res.first), 
                              GraphAttributeType::InterfaceAttribute);
    }

    return std::make_pair(fullName, GraphAttributeType::Parameter);
}

/* static */
TfToken 
GraphUtils::GetFullName(const TfToken &baseName, 
                           const GraphAttributeType type)
{
    return TfToken(GraphUtils::GetPrefixForAttributeType(type) + 
                   baseName.GetString());
}

/* static */
bool 
GraphUtils::ReadOldEncoding()
{
    static const bool readOldEncoding = 
        TfGetEnvSetting(USD_SHADE_READ_OLD_ENCODING);
    return readOldEncoding;
}

/* static */
bool 
GraphUtils::WriteNewEncoding()
{       
    static const bool writeNewEncoding = 
        TfGetEnvSetting(USD_SHADE_WRITE_NEW_ENCODING);
    return writeNewEncoding;
}

AMN_NAMESPACE_CLOSE_SCOPE

