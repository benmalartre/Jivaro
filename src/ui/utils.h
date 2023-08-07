#ifndef JVR_UI_UTILS_H
#define JVR_UI_UTILS_H
#include <cassert>
#include <functional>

#include <pxr/usd/usd/attribute.h>
#include <pxr/base/gf/matrix2f.h>
#include <pxr/base/gf/matrix2d.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec2h.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3h.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/vec4h.h>

#include <pxr/usd/sdf/listEditorProxy.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/listOp.h>

#include "../common.h"
#include "../ui/style.h"
#include "../ui/fonts.h"
#include "../ui/utils.h"
#include "../utils/icons.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_stdlib.h"

PXR_NAMESPACE_OPEN_SCOPE
// Unfortunately Inherit and Specialize are just alias to SdfPath
// To reuse templated code we create new type SdfInherit and SdfSpecialize 
template <int InheritOrSpecialize> struct SdfInheritOrSpecialize : pxr::SdfPath {
  SdfInheritOrSpecialize() : pxr::SdfPath() {}
  SdfInheritOrSpecialize(const pxr::SdfPath &path) : pxr::SdfPath(path) {}
};

using SdfInherit = SdfInheritOrSpecialize<0>;
using SdfSpecialize = SdfInheritOrSpecialize<1>;
PXR_NAMESPACE_CLOSE_SCOPE

JVR_NAMESPACE_OPEN_SCOPE

const pxr::GfVec2f BUTTON_LARGE_SIZE(64.f, 64.f);
const pxr::GfVec2f BUTTON_NORMAL_SIZE(24.f, 24.f);
const pxr::GfVec2f BUTTON_MINI_SIZE(16.f, 20.f);

/// Height of a row in the property editor
constexpr float TABLE_ROW_DEFAULT_HEIGHT = 22.f;

class View;
class UIUtils {
public:

  static void HelpMarker(const char* desc);

  static void AddAttributeDisplayName(const pxr::UsdAttribute& attribute);
  static pxr::VtValue AddTokenWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddColorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  static pxr::VtValue AddAttributeWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename VectorType, int DataType, int N>
  static pxr::VtValue AddVectorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  template <typename MatrixType, int DataType, int Rows, int Cols>
  static pxr::VtValue AddMatrixWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);

  static void IconButton(const char* icon, short state, CALLBACK_FN func);
  static bool AddIconButton(const char* icon, short state, CALLBACK_FN func);
  static bool AddIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  static bool AddTransparentIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);
  static bool AddCheckableIconButton(ImGuiID id, const char* icon, short state, CALLBACK_FN func);

  static void AddPropertyMiniButton(const char* btnStr, int rowId, 
    const ImVec4& btnColor = ImVec4(0.0, 0.7, 0.0, 1.0));
  static void AddPrimKind(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimType(const pxr::SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  static void AddPrimSpecifier(const pxr::SdfPrimSpecHandle& primSpec, ImGuiComboFlags comboFlags = 0);
  static void AddPrimInstanceable(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimHidden(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimActive(const pxr::SdfPrimSpecHandle& primSpec);
  static void AddPrimName(const pxr::SdfPrimSpecHandle& primSpec);

};

template <typename MatrixType, int DataType, int Rows, int Cols>
pxr::VtValue 
UIUtils::AddMatrixWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  MatrixType buffer = value.Get<MatrixType>();
  bool valueChanged = false;
    
  ImGui::PushID(attribute.GetName().GetText());
  ImGui::InputScalarN("###row0", DataType, buffer.data(), Cols, NULL, NULL, DecimalPrecision);
  valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
  ImGui::InputScalarN("###row1", DataType, buffer.data() + Cols, Cols, NULL, NULL, DecimalPrecision);
  valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
  if (Rows > 2) {
    ImGui::InputScalarN("###row2", DataType, buffer.data() + 2 * Cols, Cols, NULL, NULL, DecimalPrecision);
    valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    if (Rows > 3) {
      ImGui::InputScalarN("###row3", DataType, buffer.data() + 3 * Cols, Cols, NULL, NULL, DecimalPrecision);
      valueChanged |= ImGui::IsItemDeactivatedAfterEdit();
    }
  }
  ImGui::PopID();
    
  if (valueChanged) {
    return pxr::VtValue(buffer);
  }
  return pxr::VtValue();
}

template <typename VectorType, int DataType, int N>
pxr::VtValue 
UIUtils::AddVectorWidget(const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
{
  VectorType buffer;
  attribute.Get<VectorType>(&buffer, timeCode);
  constexpr const char* format = DataType == ImGuiDataType_S32 ? "%d" : DecimalPrecision;
  ImGui::InputScalarN(attribute.GetName().GetText(), DataType, buffer.data(), N,
    NULL, NULL, format, ImGuiInputTextFlags());
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    return pxr::VtValue(VectorType(buffer));
  }
  return pxr::VtValue();
}

// ExtraArgsT is used to pass additional arguments as the function passed as visitor
// might need more than the operation and the item
template <typename PolicyT, typename FuncT, typename... ExtraArgsT>
static void IterateListEditorItems(const pxr::SdfListEditorProxy<PolicyT> &listEditor, 
  const FuncT &call, ExtraArgsT... args) 
{
  // TODO: should we check if the list is already all explicit ??
  for (const typename PolicyT::value_type &item : listEditor.GetExplicitItems()) {
    call(pxr::SdfListOpTypeExplicit, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetOrderedItems()) {
    call(pxr::SdfListOpTypeOrdered, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetAddedItems()) {
    call(pxr::SdfListOpTypeAdded, item, args...); // return "add" as TfToken instead ?
  }
  for (const typename PolicyT::value_type &item : listEditor.GetPrependedItems()) {
    call(pxr::SdfListOpTypePrepended, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetAppendedItems()) {
    call(pxr::SdfListOpTypeAppended, item, args...);
  }
  for (const typename PolicyT::value_type &item : listEditor.GetDeletedItems()) {
    call(pxr::SdfListOpTypeDeleted, item, args...);
  }
}

/// The list available on a SdfListEditor
constexpr int GetListEditorOperationSize() { return 6; }

template <typename IntOrSdfListOpT> inline const char *GetListEditorOperationName(IntOrSdfListOpT index) {
    constexpr const char *names[GetListEditorOperationSize()] = 
      {"explicit", "add", "delete", "ordered", "prepend", "append"};
    return names[static_cast<int>(index)];
}

template <typename IntOrSdfListOpT> inline const char *GetListEditorOperationAbbreviation(IntOrSdfListOpT index) {
    constexpr const char *names[GetListEditorOperationSize()] =
      {"Ex", "Ad", "De", "Or", "Pr", "Ap"};
    return names[static_cast<int>(index)];
}

template <typename PolicyT>
inline void CreateListEditorOperation(pxr::SdfListEditorProxy<PolicyT> &&listEditor, pxr::SdfListOpType op,
                                      typename pxr::SdfListEditorProxy<PolicyT>::value_type item) 
{
  switch (op) {
  case pxr:: SdfListOpTypeAdded:
    listEditor.GetAddedItems().push_back(item);
    break;
  case pxr::SdfListOpTypePrepended:
    listEditor.GetPrependedItems().push_back(item);
    break;
  case pxr::SdfListOpTypeAppended:
      listEditor.GetAppendedItems().push_back(item);
      break;
  case pxr::SdfListOpTypeDeleted:
      listEditor.GetDeletedItems().push_back(item);
      break;
  case pxr::SdfListOpTypeExplicit:
      listEditor.GetExplicitItems().push_back(item);
      break;
  case pxr::SdfListOpTypeOrdered:
      listEditor.GetOrderedItems().push_back(item);
      break;
  default:
      assert(0);
  }
}

template <typename ListEditorT, typename OpOrIntT> 
inline auto GetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_)
 {
    const pxr::SdfListOpType op = static_cast<pxr::SdfListOpType>(op_);
    if (op == pxr::SdfListOpTypeOrdered) {
        return listEditor.GetOrderedItems();
    } else if (op == pxr::SdfListOpTypeAppended) {
        return listEditor.GetAppendedItems();
    } else if (op == pxr::SdfListOpTypeAdded) {
        return listEditor.GetAddedItems();
    } else if (op == pxr::SdfListOpTypePrepended) {
        return listEditor.GetPrependedItems();
    } else if (op == pxr::SdfListOpTypeDeleted) {
        return listEditor.GetDeletedItems();
    }
    return listEditor.GetExplicitItems();
};


template <typename ListEditorT, typename OpOrIntT, typename ItemsT> 
inline void SetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_, const ItemsT &items) 
{
    const pxr::SdfListOpType op = static_cast<pxr::SdfListOpType>(op_);
    if (op == pxr::SdfListOpTypeOrdered) {
        listEditor.SetOrderedItems(items);
    } else if (op == pxr::SdfListOpTypeAppended) {
        listEditor.SetAppendedItems(items);
    } else if (op == pxr::SdfListOpTypeAdded) {
        listEditor.SetAddedItems(items);
    } else if (op == pxr::SdfListOpTypePrepended) {
        listEditor.SetPrependedItems(items);
    } else if (op == pxr::SdfListOpTypeDeleted) {
        listEditor.SetDeletedItems(items);
    } else {
        listEditor.SetExplicitItems(items);
    }
};

template <typename Policy>
void CreateListEditorOperation(pxr::SdfListEditorProxy<Policy>&& listEditor, int operation,
  typename pxr::SdfListEditorProxy<Policy>::value_type item) {
  switch (operation) {
  case 0:
    listEditor.Add(item);
    break;
  case 1:
    listEditor.Prepend(item);
    break;
  case 2:
    listEditor.Append(item);
    break;
  case 3:
    listEditor.Remove(item);
    break;
  case 4:
    listEditor.GetExplicitItems().push_back(item);
    break;
  default:
    assert(0);
  }
}

/// Composition
void AddReferenceSummary(const char *operation, const pxr::SdfReference &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddPayloadSummary(const char *operation, const pxr::SdfPayload &assetPath, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddInheritsSummary(const char *operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);

void AddSpecializesSummary(const char *operation, const pxr::SdfPath &path, 
  const pxr::SdfPrimSpecHandle &primSpec, int &menuItemId);


/// Add modal dialogs to add composition on primspec (reference, payload, inherit, specialize)
void AddPrimCreateReference(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreatePayload(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreateInherit(const pxr::SdfPrimSpecHandle &primSpec);
void AddPrimCreateSpecialize(const pxr::SdfPrimSpecHandle &primSpec);

/// Add multiple tables with the compositions (Reference, Payload, Inherit, Specialize)
void AddPrimCompositions(const pxr::SdfPrimSpecHandle &primSpec);

// Add a text summary of the composition
void AddPrimCompositionSummary(const pxr::SdfPrimSpecHandle &primSpec);

/// Function to convert a hash from usd to ImGuiID with a seed, to avoid collision with path coming from layer and stages.
template <ImU32 seed, typename T> inline ImGuiID ToImGuiID(const T &val) {
    return ImHashData(static_cast<const void *>(&val), sizeof(T), seed);
}

//// Correctly indent the tree nodes using a path. This is used when we are iterating in a list of paths as opposed to a tree.
//// It allocates a vector which might not be optimal, but it should be used only on visible items, that should mitigate the
//// allocation cost
template <ImU32 seed, typename PathT> struct TreeIndenter {
    TreeIndenter(const PathT &path) {
        path.GetPrefixes(&prefixes);
        for (int i = 0; i < prefixes.size(); ++i) {
            ImGui::TreePushOverrideID(ToImGuiID<seed>(prefixes[i].GetHash()));
        }
    }
    ~TreeIndenter() {
        for (int i = 0; i < prefixes.size(); ++i) {
            ImGui::TreePop();
        }
    }
    std::vector<PathT> prefixes;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_UTILS_H