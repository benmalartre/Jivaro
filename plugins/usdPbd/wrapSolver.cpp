//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/solver.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
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
_CreateStartFrameAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateStartFrameAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateSleepThresholdAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSleepThresholdAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSubStepsAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSubStepsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateGravityAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateGravityAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Vector3f), writeSparsely);
}
        
static UsdAttribute
_CreateShowPointsAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateShowPointsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateShowConstraintsAttr(UsdPbdSolver &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateShowConstraintsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}

static std::string
_Repr(const UsdPbdSolver &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "UsdPbd.Solver(%s)",
        primRepr.c_str());
}

} // anonymous namespace

void wrapUsdPbdSolver()
{
    typedef UsdPbdSolver This;

    class_<This, bases<UsdGeomXform> >
        cls("Solver");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("Define", &This::Define, (arg("stage"), arg("path")))
        .staticmethod("Define")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetStartFrameAttr",
             &This::GetStartFrameAttr)
        .def("CreateStartFrameAttr",
             &_CreateStartFrameAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSleepThresholdAttr",
             &This::GetSleepThresholdAttr)
        .def("CreateSleepThresholdAttr",
             &_CreateSleepThresholdAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSubStepsAttr",
             &This::GetSubStepsAttr)
        .def("CreateSubStepsAttr",
             &_CreateSubStepsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetGravityAttr",
             &This::GetGravityAttr)
        .def("CreateGravityAttr",
             &_CreateGravityAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetShowPointsAttr",
             &This::GetShowPointsAttr)
        .def("CreateShowPointsAttr",
             &_CreateShowPointsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetShowConstraintsAttr",
             &This::GetShowConstraintsAttr)
        .def("CreateShowConstraintsAttr",
             &_CreateShowConstraintsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        
        .def("GetBodiesRel",
             &This::GetBodiesRel)
        .def("CreateBodiesRel",
             &This::CreateBodiesRel)
        
        .def("GetCollidersRel",
             &This::GetCollidersRel)
        .def("CreateCollidersRel",
             &This::CreateCollidersRel)
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
