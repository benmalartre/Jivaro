#ifndef JVR_UI_PROPERTYEDITOR_H
#define JVR_UI_PROPERTYEDITOR_H

#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

class PropertyEditorUI : public BaseUI
{
public:
  PropertyEditorUI(View* parent);
  ~PropertyEditorUI()         override;

  void SetPrim(const UsdPrim& prim);
  UsdPrim& GetPrim() { return _prim; };


  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw()      override;
  void OnSelectionChangedNotice(const SelectionChangedNotice& n) override;

private:
  void _DrawAttributeTypeInfo(const UsdAttribute& attribute);
  VtValue _DrawAttributeValue(const UsdAttribute& attribute, const UsdTimeCode& timeCode);
  void _DrawAttributeValueAtTime(const  UsdAttribute& attribute, const UsdTimeCode& currentTime);
  bool _DrawAssetInfo(const UsdPrim& prim);
  bool _DrawXformsCommon(UsdTimeCode time);
  bool _DrawVariantSetsCombos(UsdPrim& prim);
  /*
  static VtValue _DrawAttributeValue(
    const std::string &label, UsdAttribute &attribute, const VtValue &value);
  static void _DrawAttributeTypeInfo(
    const UsdAttribute &attribute);
  static void _DrawAttributeDisplayName(
    const UsdAttribute &attribute);
  static void _DrawAttributeValueAtTime(
    UsdAttribute &attribute, UsdTimeCode currentTime);
  static void _DrawUsdRelationshipDisplayName(
    const UsdRelationship &relationship);
  static void _DrawUsdRelationshipList(
    const UsdRelationship &relationship);
  static bool _DrawVariantSetsCombos(UsdPrim &prim);
  */

  GfVec3f            _color;
  UsdPrim            _prim;
  static ImGuiWindowFlags _flags;
  ImGuiID                 _focused;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_PROPERTYEDITOR_H
