#include "../pbd/tokens.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDTokensType::PBDTokensType() :
  substeps              ("substeps",              pxr::TfToken::Immortal),
  sleepThreshold        ("sleepThreshold",        pxr::TfToken::Immortal),
  gravity               ("gravity",               pxr::TfToken::Immortal),
  damp                  ("damp",                  pxr::TfToken::Immortal),
  friction              ("friction",              pxr::TfToken::Immortal),
  restitution           ("restitution",           pxr::TfToken::Immortal),
  selfCollide           ("selfCollide",           pxr::TfToken::Immortal),
  stretchStiffness      ("stretchStiffness",      pxr::TfToken::Immortal),
  bendStiffness         ("bendStiffness",         pxr::TfToken::Immortal),
  allTokens({
    substeps,
    sleepThreshold,
    gravity,
    damp,
    friction,
    restitution,
    selfCollide,
    stretchStiffness,
    bendStiffness
  })
{
}

pxr::TfStaticData<PBDTokensType> PBDTokens;

JVR_NAMESPACE_CLOSE_SCOPE

