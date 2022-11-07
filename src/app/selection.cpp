#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/kind/registry.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include "../app/selection.h"
#include "../app/application.h"
#include "../app/notice.h"

JVR_NAMESPACE_OPEN_SCOPE

void Selection::Item::ComputeHash()
{
  hash = 0;
  boost::hash_combine(hash, type);
  boost::hash_combine(hash, path);
  boost::hash_combine(hash, components);
  boost::hash_combine(hash, weights);
}

void Selection::ComputeHash()
{
  _hash = 0;
  for (auto& item: _items) {
    boost::hash_combine(_hash, item.hash);
  }
}

bool Selection::IsSelected(const pxr::UsdPrim& prim)
{
  const pxr::SdfPath primPath = prim.GetPath();
  for (auto& item : _items) {
    if (item.path == primPath) return true;
  }
  return false;
}

bool Selection::IsEmpty()
{
  return _items.size() == 0;
}

bool Selection::IsObject()
{
  for (auto& item : _items) {
    if (item.type == Type::PRIM) return true;
  }
  return false;
}

bool Selection::IsComponent()
{
  for (auto& item : _items) {
    if (item.type == Type::VERTEX ||
        item.type == Type::EDGE ||
        item.type == Type::FACE) return true;
  }
  return false;
}

bool Selection::IsAttribute()
{
  for (auto& item : _items) {
    if (item.type == Type::ATTRIBUTE) return true;
  }
  return false;
}

bool 
Selection::_CheckKind(Mode mode, const pxr::TfToken& kind)
{
  switch (mode) {
  case Mode::COMPONENT:
    return pxr::KindRegistry::GetInstance().IsA(kind, pxr::KindTokens->model);
  case Mode::GROUP:
    return pxr::KindRegistry::GetInstance().IsA(kind, pxr::KindTokens->group);
  case Mode::ASSEMBLY:
    return pxr::KindRegistry::GetInstance().IsA(kind, pxr::KindTokens->assembly);
  case Mode::SUBCOMPONENT:
    return pxr::KindRegistry::GetInstance().IsA(kind, pxr::KindTokens->subcomponent);
  default:
    return pxr::KindRegistry::GetInstance().IsA(kind, pxr::KindTokens->model);
  }
}

bool Selection::IsPickablePath(const pxr::UsdStage& stage,
  const pxr::SdfPath& path) {
  auto prim = stage.GetPrimAtPath(path);
  if (prim.IsPseudoRoot())
    return true;

  pxr::TfToken primKind;
  pxr::UsdModelAPI(prim).GetKind(&primKind);
  return(_CheckKind(_mode, primKind));
}

void Selection::AddItem(const pxr::SdfPath& path)
{
  for (auto& item: _items) {
    if (path == item.path)return;
  }
  _items.push_back({ Type::PRIM, path });
  _items.back().ComputeHash();
  ComputeHash();

}

void Selection::RemoveItem(const pxr::SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) _items.erase(it);
  }
  ComputeHash();
}

void Selection::ToggleItem(const pxr::SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) {
      _items.erase(it);
      ComputeHash();
      return;
    }
  }
  AddItem(path);
  ComputeHash();
}

void Selection::AddComponent(const pxr::SdfPath& object,
  Type type, int index)
{
  ComputeHash();
}

void Selection::RemoveComponent(const pxr::SdfPath& object,
  Type type, int index)
{
  ComputeHash();
}

void Selection::AddComponents(const pxr::SdfPath& object,
  Type type, std::vector<int> indices)
{
  ComputeHash();
}

void Selection::RemoveComponents(const pxr::SdfPath& object,
  Type type, std::vector<int> indices)
{
  ComputeHash();
}

void Selection::Clear()
{
  _items.clear();
  ComputeHash();
}

pxr::SdfPathVector Selection::GetSelectedPrims()
{
  pxr::SdfPathVector selectedPrims;
  for (const auto& item : _items) {
    selectedPrims.push_back(item.path);
  }
  return selectedPrims;
}

bool IsPickablePath(const pxr::UsdStage& stage, const pxr::SdfPath& path) {
  pxr::UsdPrim prim = stage.GetPrimAtPath(path);
  if (prim.IsPseudoRoot())
    return true;
  /*
  if (GetPickMode() == SelectionManipulator::PickMode::Prim)
    return true;

  TfToken primKind;
  pxr::UsdModelAPI(prim).GetKind(&primKind);
  if (GetPickMode() == SelectionManipulator::PickMode::Model && KindRegistry::GetInstance().IsA(primKind, KindTokens->model)) {
    return true;
  }
  if (GetPickMode() == SelectionManipulator::PickMode::Assembly &&
    KindRegistry::GetInstance().IsA(primKind, KindTokens->assembly)) {
    return true;
  }

  // Other possible tokens
  // KindTokens->component
  // KindTokens->group
  // KindTokens->subcomponent

  // We can also test for xformable or other schema API
  */
  return false;
}



JVR_NAMESPACE_CLOSE_SCOPE