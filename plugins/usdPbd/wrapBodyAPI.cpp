//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/bodyAPI.h"
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
_CreateSimulationEnabledAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSimulationEnabledAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateRadiusAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateRadiusAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionEnabledAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionEnabledAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionRadiusAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionRadiusAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionDampAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionDampAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionFrictionAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionFrictionAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionRestitutionAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionRestitutionAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSelfCollisionMaxSeparationVelocityAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSelfCollisionMaxSeparationVelocityAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateMassAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMassAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateDampAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateDampAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateVelocityAttr(UsdPbdBodyAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateVelocityAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Vector3f), writeSparsely);
}

static std::string
_Repr(const UsdPbdBodyAPI &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "UsdPbd.BodyAPI(%s)",
        primRepr.c_str());
}

struct UsdPbdBodyAPI_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    UsdPbdBodyAPI_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static UsdPbdBodyAPI_CanApplyResult
_WrapCanApply(const UsdPrim& prim)
{
    std::string whyNot;
    bool result = UsdPbdBodyAPI::CanApply(prim, &whyNot);
    return UsdPbdBodyAPI_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapUsdPbdBodyAPI()
{
    typedef UsdPbdBodyAPI This;

    UsdPbdBodyAPI_CanApplyResult::Wrap<UsdPbdBodyAPI_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("BodyAPI");

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

        
        .def("GetSimulationEnabledAttr",
             &This::GetSimulationEnabledAttr)
        .def("CreateSimulationEnabledAttr",
             &_CreateSimulationEnabledAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetRadiusAttr",
             &This::GetRadiusAttr)
        .def("CreateRadiusAttr",
             &_CreateRadiusAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionEnabledAttr",
             &This::GetSelfCollisionEnabledAttr)
        .def("CreateSelfCollisionEnabledAttr",
             &_CreateSelfCollisionEnabledAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionRadiusAttr",
             &This::GetSelfCollisionRadiusAttr)
        .def("CreateSelfCollisionRadiusAttr",
             &_CreateSelfCollisionRadiusAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionDampAttr",
             &This::GetSelfCollisionDampAttr)
        .def("CreateSelfCollisionDampAttr",
             &_CreateSelfCollisionDampAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionFrictionAttr",
             &This::GetSelfCollisionFrictionAttr)
        .def("CreateSelfCollisionFrictionAttr",
             &_CreateSelfCollisionFrictionAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionRestitutionAttr",
             &This::GetSelfCollisionRestitutionAttr)
        .def("CreateSelfCollisionRestitutionAttr",
             &_CreateSelfCollisionRestitutionAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSelfCollisionMaxSeparationVelocityAttr",
             &This::GetSelfCollisionMaxSeparationVelocityAttr)
        .def("CreateSelfCollisionMaxSeparationVelocityAttr",
             &_CreateSelfCollisionMaxSeparationVelocityAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMassAttr",
             &This::GetMassAttr)
        .def("CreateMassAttr",
             &_CreateMassAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetDampAttr",
             &This::GetDampAttr)
        .def("CreateDampAttr",
             &_CreateDampAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetVelocityAttr",
             &This::GetVelocityAttr)
        .def("CreateVelocityAttr",
             &_CreateVelocityAttr,
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
