#ifndef JVR_UI_PROPERTY_H
#define JVR_UI_PROPERTY_H

#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/head.h"
#include "../ui/utils.h"


PXR_NAMESPACE_OPEN_SCOPE

class PropertyUI : public HeadedUI
{
public:
  PropertyUI(View* parent, const std::string& name);
  ~PropertyUI()         override;

  void SetPrim(const pxr::UsdPrim& prim);
  pxr::UsdPrim& GetPrim() { return _prim; };


  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;
  void OnSelectionChangedNotice(const SelectionChangedNotice& n) override;

private:
  void _DrawAttributeTypeInfo(const pxr::UsdAttribute& attribute);
  VtValue _DrawAttributeValue(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  void _DrawAttributeValueAtTime(const pxr:: UsdAttribute& attribute, const pxr::UsdTimeCode& currentTime);
  bool _DrawAssetInfo(const pxr::UsdPrim& prim);
  bool _DrawXformsCommon(pxr::UsdTimeCode time);
  bool _DrawVariantSetsCombos(pxr::UsdPrim& prim);
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
  ImGuiID                 _focused;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_PROPERTY_H
