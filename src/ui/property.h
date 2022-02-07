#ifndef JVR_UI_PROPERTY_H
#define JVR_UI_PROPERTY_H

#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


PXR_NAMESPACE_OPEN_SCOPE

class PropertyUI : public BaseUI
{
public:
  PropertyUI(View* parent, const std::string& name);
  ~PropertyUI()         override;

  bool DrawAssetInfo(const pxr::UsdPrim& prim);
  void SetPrim(const pxr::UsdPrim& prim);
  pxr::UsdPrim& GetPrim() { return _prim; };


  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;
  void OnSelectionChangedNotice(const SelectionChangedNotice& n) override;

private:
  bool _DrawXformsCommon(pxr::UsdTimeCode time);
  /*
  static pxr::VtValue _DrawAttributeValue(
    const std::string &label, pxr::UsdAttribute &attribute, const pxr::VtValue &value);
  static void _DrawAttributeTypeInfo(
    const pxr::UsdAttribute &attribute);
  static void _DrawAttributeDisplayName(
    const pxr::UsdAttribute &attribute);
  static void _DrawAttributeValueAtTime(
    pxr::UsdAttribute &attribute, pxr::UsdTimeCode currentTime);
  static void _DrawUsdRelationshipDisplayName(
    const pxr::UsdRelationship &relationship);
  static void _DrawUsdRelationshipList(
    const pxr::UsdRelationship &relationship);
  static bool _DrawVariantSetsCombos(pxr::UsdPrim &prim);
  */

  pxr::GfVec3f            _color;
  pxr::UsdPrim            _prim;
  static ImGuiWindowFlags _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_PROPERTY_H