//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/constraintAPI.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyAnnotatedBoolResult.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/wrapTypeHelpers.h"

#include "pxr/external/boost/python.hpp"

#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

namespace {

#define WRAP_CUSTOM                                                     \
    template <class Cls> static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

        
static UsdAttribute
_CreateConstraintEnabledAttr(UsdPbdConstraintAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateConstraintEnabledAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateStiffnessAttr(UsdPbdConstraintAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateStiffnessAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateDampAttr(UsdPbdConstraintAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateDampAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static bool _WrapIsPbdConstraintAPIPath(const SdfPath &path) {
    TfToken collectionName;
    return UsdPbdConstraintAPI::IsPbdConstraintAPIPath(
        path, &collectionName);
}

static std::string
_Repr(const UsdPbdConstraintAPI &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    std::string instanceName = TfPyRepr(self.GetName());
    return TfStringPrintf(
        "UsdPbd.ConstraintAPI(%s, '%s')",
        primRepr.c_str(), instanceName.c_str());
}

struct UsdPbdConstraintAPI_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    UsdPbdConstraintAPI_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static UsdPbdConstraintAPI_CanApplyResult
_WrapCanApply(const UsdPrim& prim, const TfToken& name)
{
    std::string whyNot;
    bool result = UsdPbdConstraintAPI::CanApply(prim, name, &whyNot);
    return UsdPbdConstraintAPI_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapUsdPbdConstraintAPI()
{
    typedef UsdPbdConstraintAPI This;

    UsdPbdConstraintAPI_CanApplyResult::Wrap<UsdPbdConstraintAPI_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("ConstraintAPI");

    cls
        .def(init<UsdPrim, TfToken>((arg("prim"), arg("name"))))
        .def(init<UsdSchemaBase const&, TfToken>((arg("schemaObj"), arg("name"))))
        .def(TfTypePythonClass())

        .def("Get",
            (UsdPbdConstraintAPI(*)(const UsdStagePtr &stage, 
                                       const SdfPath &path))
               &This::Get,
            (arg("stage"), arg("path")))
        .def("Get",
            (UsdPbdConstraintAPI(*)(const UsdPrim &prim,
                                       const TfToken &name))
               &This::Get,
            (arg("prim"), arg("name")))
        .staticmethod("Get")

        .def("GetAll",
            (std::vector<UsdPbdConstraintAPI>(*)(const UsdPrim &prim))
                &This::GetAll,
            arg("prim"),
            return_value_policy<TfPySequenceToList>())
        .staticmethod("GetAll")

        .def("CanApply", &_WrapCanApply, (arg("prim"), arg("name")))
        .staticmethod("CanApply")

        .def("Apply", &This::Apply, (arg("prim"), arg("name")))
        .staticmethod("Apply")

        .def("GetSchemaAttributeNames",
             (const TfTokenVector &(*)(bool))&This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .def("GetSchemaAttributeNames",
             (TfTokenVector(*)(bool, const TfToken &))
                &This::GetSchemaAttributeNames,
             arg("includeInherited"),
             arg("instanceName"),
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetConstraintEnabledAttr",
             &This::GetConstraintEnabledAttr)
        .def("CreateConstraintEnabledAttr",
             &_CreateConstraintEnabledAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetStiffnessAttr",
             &This::GetStiffnessAttr)
        .def("CreateStiffnessAttr",
             &_CreateStiffnessAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetDampAttr",
             &This::GetDampAttr)
        .def("CreateDampAttr",
             &_CreateDampAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        .def("IsPbdConstraintAPIPath", _WrapIsPbdConstraintAPIPath)
            .staticmethod("IsPbdConstraintAPIPath")
        .def("__repr__", ::_Repr)
    ;

    _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by 
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
// 
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace {

WRAP_CUSTOM {
}

}
