#include "../pbd/tokens.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDTokensType::PBDTokensType() :
  substeps              ("substeps",              pxr::TfToken::Immortal),
  sleep                 ("sleep",                 pxr::TfToken::Immortal),
  gravity               ("gravity",               pxr::TfToken::Immortal),
  damp                  ("damp",                  pxr::TfToken::Immortal),
  friction              ("friction",              pxr::TfToken::Immortal),
  restitution           ("restitution",           pxr::TfToken::Immortal),
  self                  ("self",                  pxr::TfToken::Immortal),
  stretch               ("stretch",               pxr::TfToken::Immortal),
  shear                 ("shear",                 pxr::TfToken::Immortal),
  bend                  ("bend",                  pxr::TfToken::Immortal),
  volume                ("volume",                pxr::TfToken::Immortal),
  allTokens({
    substeps,
    sleep,
    gravity,
    damp,
    friction,
    restitution,
    self,
    stretch,
    shear,
    bend,
    volume
  })
{
}

pxr::TfStaticData<PBDTokensType> PBDTokens;

JVR_NAMESPACE_CLOSE_SCOPE

