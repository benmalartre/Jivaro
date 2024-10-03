#ifndef JVR_PBD_TOKENS_H
#define JVR_PBD_TOKENS_H

#include <pxr/pxr.h>
#include <pxr/base/tf/staticData.h>
#include <pxr/base/tf/token.h>
#include <vector>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

struct PBDTokensType {
    PBDTokensType();

    const pxr::TfToken substeps;
    const pxr::TfToken sleep;
    const pxr::TfToken mass;
    const pxr::TfToken velocity;
    const pxr::TfToken gravity;
    const pxr::TfToken stiffness;
    const pxr::TfToken damp;
    const pxr::TfToken friction;
    const pxr::TfToken restitution;
    const pxr::TfToken self;
    const pxr::TfToken stretch;
    const pxr::TfToken shear;
    const pxr::TfToken bend;
    const pxr::TfToken dihedral;
    const pxr::TfToken volume;

    const std::vector<pxr::TfToken> allTokens;
};


extern pxr::TfStaticData<PBDTokensType> PBDTokens;


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_TOKENS_H



