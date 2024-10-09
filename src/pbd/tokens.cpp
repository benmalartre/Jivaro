#include "../pbd/tokens.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDTokensType::PBDTokensType() :
  substeps              ("substeps",              pxr::TfToken::Immortal),
  iterations            ("iterations",            pxr::TfToken::Immortal),
  sleep                 ("sleep",                 pxr::TfToken::Immortal),
  mass                  ("mass",                  pxr::TfToken::Immortal),
  velocity              ("velocity",              pxr::TfToken::Immortal),
  gravity               ("gravity",               pxr::TfToken::Immortal),
  stiffness             ("stiffness",             pxr::TfToken::Immortal),
  damp                  ("damp",                  pxr::TfToken::Immortal),
  friction              ("friction",              pxr::TfToken::Immortal),
  restitution           ("restitution",           pxr::TfToken::Immortal),
  self                  ("self",                  pxr::TfToken::Immortal),
  stretch               ("stretch",               pxr::TfToken::Immortal),
  shear                 ("shear",                 pxr::TfToken::Immortal),
  bend                  ("bend",                  pxr::TfToken::Immortal),
  dihedral              ("dihedral",              pxr::TfToken::Immortal),
  volume                ("volume",                pxr::TfToken::Immortal),
  allTokens({
    substeps,
    iterations,
    sleep,
    mass,
    velocity,
    gravity,
    stiffness,
    damp,
    friction,
    restitution,
    self,
    stretch,
    shear,
    bend,
    dihedral,
    volume
  })
{
}

pxr::TfStaticData<PBDTokensType> PBDTokens;

JVR_NAMESPACE_CLOSE_SCOPE

