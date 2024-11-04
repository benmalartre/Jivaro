#include <sstream>
#include <iomanip>

#include <pxr/usd/sdf/fileFormat.h>
#include "pxr/base/plug/registry.h"
#include "pxr/base/tf/staticTokens.h"

#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usd/attributeQuery.h"
#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/schemaBase.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdGeom/imageable.h"


#include "../acceleration/bvh.h"
#include "../app/utils.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace Utils {
std::string FindNextAvailableTokenString(std::string prefix) {
    // Find number in the prefix
    size_t end = prefix.size() - 1;
    while (end > 0 && std::isdigit(prefix[end])) {
        end--;
    }
    size_t padding = prefix.size() - 1 - end;
    const std::string number = prefix.substr(end + 1, padding);
    auto value = number.size() ? std::stoi(number) : 0;
    std::ostringstream newName;
    padding = padding == 0 ? 4 : padding; // 4: default padding
    do {
        value += 1;
        newName.seekp(0, std::ios_base::beg); // rewind
        newName << prefix.substr(0, end + 1) << std::setfill('0') << std::setw(padding) << value;
        // Looking for existing token with the same name.
        // There might be a better solution here
    } while (TfToken::Find(newName.str()) != TfToken());
    return newName.str();
}

const std::vector<std::string> GetUsdValidExtensions() {
    const auto usdExtensions = SdfFileFormat::FindAllFileFormatExtensions();
    std::vector<std::string> validExtensions;
    auto addDot = [](const std::string &str) { return "." + str; };
    std::transform(usdExtensions.cbegin(), usdExtensions.cend(), std::back_inserter(validExtensions), addDot);
    return validExtensions;
}

/*static*/
std::vector<UsdPrim> 
_GetAllPrimsOfType(UsdStagePtr const &stage, 
                                  TfType const& schemaType)
{
    std::vector<UsdPrim> result;
    UsdPrimRange range = stage->Traverse();
    std::copy_if(range.begin(), range.end(), std::back_inserter(result),
                 [schemaType](UsdPrim const &prim) {
                     return prim.IsA(schemaType);
                 });
    return result;
}

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (root)
);

PrimInfo::PrimInfo(const UsdPrim &prim, const UsdTimeCode time)
{
    hasCompositionArcs = (prim.HasAuthoredReferences() ||
                          prim.HasAuthoredPayloads() ||
                          prim.HasAuthoredInherits() ||
                          prim.HasAuthoredSpecializes() ||
                          prim.HasVariantSets());
    isActive = prim.IsActive();
    UsdGeomImageable img(prim);
    isImageable = static_cast<bool>(img);
    isDefined = prim.IsDefined();
    isAbstract = prim.IsAbstract();

    // isInPrototype is meant to guide UI to consider the prim's "source", so
    // even if the prim is a proxy prim, then unlike the core
    // UsdPrim.IsInPrototype(), we want to consider it as coming from a
    // prototype to make it visually distinctive.  If in future we need to
    // decouple the two concepts we can, but we're sensitive here to python
    // marshalling costs.
    isInPrototype = prim.IsInPrototype() || prim.IsInstanceProxy();


    // only show camera guides for now, until more guide generation logic is
    // moved into usdImaging
    supportsGuides = prim.IsA<UsdGeomCamera>();

    supportsDrawMode = isActive && isDefined && 
        !isInPrototype && prim.GetPath() != SdfPath::AbsoluteRootPath() &&
        UsdModelAPI(prim).IsModel();

    isInstance = prim.IsInstance();
    isVisibilityInherited = false;
    if (img){
        UsdAttributeQuery query(img.GetVisibilityAttr());
        TfToken visibility = UsdGeomTokens->inherited;
        query.Get(&visibility, time);
        isVisibilityInherited = (visibility == UsdGeomTokens->inherited);
        visVaries = query.ValueMightBeTimeVarying();
    }
    else {
        visVaries = false;
    }

    if (prim.GetParent())
        name = prim.GetName().GetString();
    else
        name = _tokens->root.GetString();
    typeName = prim.GetTypeName().GetString();

    displayName = prim.GetDisplayName();
}

/*static*/
PrimInfo
GetPrimInfo(const UsdPrim &prim, const UsdTimeCode time)
{
    return PrimInfo(prim, time);
}
}


JVR_NAMESPACE_CLOSE_SCOPE