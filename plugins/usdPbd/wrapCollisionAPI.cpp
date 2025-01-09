//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/collisionAPI.h"
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
_CreateCollisionEnabledAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCollisionEnabledAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateMarginAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMarginAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateDampAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateDampAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateCollisionStiffnessAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCollisionStiffnessAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateFrictionAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateFrictionAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateRestitutionAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateRestitutionAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateMaxSeparationVelocityAttr(UsdPbdCollisionAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaxSeparationVelocityAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static std::string
_Repr(const UsdPbdCollisionAPI &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "UsdPbd.CollisionAPI(%s)",
        primRepr.c_str());
}

struct UsdPbdCollisionAPI_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    UsdPbdCollisionAPI_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static UsdPbdCollisionAPI_CanApplyResult
_WrapCanApply(const UsdPrim& prim)
{
    std::string whyNot;
    bool result = UsdPbdCollisionAPI::CanApply(prim, &whyNot);
    return UsdPbdCollisionAPI_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapUsdPbdCollisionAPI()
{
    typedef UsdPbdCollisionAPI This;

    UsdPbdCollisionAPI_CanApplyResult::Wrap<UsdPbdCollisionAPI_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("CollisionAPI");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("CanApply", &_WrapCanApply, (arg("prim")))
        .staticmethod("CanApply")

        .def("Apply", &This::Apply, (arg("prim")))
        .staticmethod("Apply")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetCollisionEnabledAttr",
             &This::GetCollisionEnabledAttr)
        .def("CreateCollisionEnabledAttr",
             &_CreateCollisionEnabledAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMarginAttr",
             &This::GetMarginAttr)
        .def("CreateMarginAttr",
             &_CreateMarginAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetDampAttr",
             &This::GetDampAttr)
        .def("CreateDampAttr",
             &_CreateDampAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetCollisionStiffnessAttr",
             &This::GetCollisionStiffnessAttr)
        .def("CreateCollisionStiffnessAttr",
             &_CreateCollisionStiffnessAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetFrictionAttr",
             &This::GetFrictionAttr)
        .def("CreateFrictionAttr",
             &_CreateFrictionAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetRestitutionAttr",
             &This::GetRestitutionAttr)
        .def("CreateRestitutionAttr",
             &_CreateRestitutionAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaxSeparationVelocityAttr",
             &This::GetMaxSeparationVelocityAttr)
        .def("CreateMaxSeparationVelocityAttr",
             &_CreateMaxSeparationVelocityAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

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
