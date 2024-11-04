#ifndef JVR_APPLICATION_UTILS_H
#define JVR_APPLICATION_UTILS_H


#include <vector>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/timeCode.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace Utils {

// ExtraArgsT is used to pass additional arguments as the function passed as visitor
// might need more than the operation and the item
template <typename PolicyT, typename FuncT, typename... ExtraArgsT>
static void IterateListEditorItems(const SdfListEditorProxy<PolicyT> &listEditor, const FuncT &func, ExtraArgsT... args) {
    // TODO: should we check if the list is already all explicit ??
    for (const typename PolicyT::value_type &item : listEditor.GetExplicitItems()) {
        func(SdfListOpTypeExplicit, item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetOrderedItems()) {
        func(SdfListOpTypeOrdered, item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetAddedItems()) {
        func(SdfListOpTypeAdded, item, args...); // return "add" as TfToken instead ?
    }
    for (const typename PolicyT::value_type &item : listEditor.GetPrependedItems()) {
        func(SdfListOpTypePrepended, item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetAppendedItems()) {
        func(SdfListOpTypeAppended, item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetDeletedItems()) {
        func(SdfListOpTypeDeleted, item, args...);
    }
}

/// The list available on a SdfListEditor
constexpr int SdfListOpSize() { return 6; }

static const char* SdfListOpNames[6] = {
  "explicit", 
  "add", 
  "delete", 
  "ordered", 
  "prepend", 
  "append"
};

static const char* SdfListOpShortNames[6] = {
  "Ex", 
  "Ad", 
  "De", 
  "Or", 
  "Pr", 
  "Ap"
};


template <typename PolicyT>
inline void CreateListEditorOperation(SdfListEditorProxy<PolicyT> &&listEditor, SdfListOpType op,
                                      typename SdfListEditorProxy<PolicyT>::value_type item) {
  switch (op) {
  case SdfListOpTypeAdded:
    listEditor.GetAddedItems().push_back(item);
    break;
  case SdfListOpTypePrepended:
    listEditor.GetPrependedItems().push_back(item);
    break;
  case SdfListOpTypeAppended:
    listEditor.GetAppendedItems().push_back(item);
    break;
  case SdfListOpTypeDeleted:
    listEditor.GetDeletedItems().push_back(item);
    break;
  case SdfListOpTypeExplicit:
    listEditor.GetExplicitItems().push_back(item);
    break;
  case SdfListOpTypeOrdered:
    listEditor.GetOrderedItems().push_back(item);
    break;
  default:
    assert(0);
  }
}

template <typename ListEditorT, typename OpOrIntT> 
inline auto GetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_) {
  const SdfListOpType op = static_cast<SdfListOpType>(op_);
  if (op == SdfListOpTypeOrdered) {
    return listEditor.GetOrderedItems();
  } else if (op == SdfListOpTypeAppended) {
    return listEditor.GetAppendedItems();
  } else if (op == SdfListOpTypeAdded) {
    return listEditor.GetAddedItems();
  } else if (op == SdfListOpTypePrepended) {
    return listEditor.GetPrependedItems();
  } else if (op == SdfListOpTypeDeleted) {
    return listEditor.GetDeletedItems();
  }
  return listEditor.GetExplicitItems();
};

template <typename ListEditorT, typename OpOrIntT, typename ItemsT> 
inline void SetSdfListOpItems(ListEditorT &listEditor, OpOrIntT op_, const ItemsT &items) {
  const SdfListOpType op = static_cast<SdfListOpType>(op_);
  if (op == SdfListOpTypeOrdered) {
    listEditor.SetOrderedItems(items);
  } else if (op == SdfListOpTypeAppended) {
    listEditor.SetAppendedItems(items);
  } else if (op == SdfListOpTypeAdded) {
    listEditor.SetAddedItems(items);
  } else if (op == SdfListOpTypePrepended) {
    listEditor.SetPrependedItems(items);
  } else if (op == SdfListOpTypeDeleted) {
    listEditor.SetDeletedItems(items);
  } else {
    listEditor.SetExplicitItems(items);
  }
};

// Look for a new token. If prefix ends with a number, it will increase its value until
// a valid token is found
std::string FindNextAvailableTokenString(std::string prefix);

// Find usd file format extensions and returns them prefixed with a dot
const std::vector<std::string> GetUsdValidExtensions();

class UsdStage;

struct PrimInfo {
  PrimInfo(const UsdPrim &prim, const UsdTimeCode time);

  bool hasCompositionArcs;  
  bool isActive;  
  bool isImageable;
  bool isDefined;
  bool isAbstract;
  bool isInPrototype;
  bool isInstance;
  bool supportsGuides;
  bool supportsDrawMode;
  bool isVisibilityInherited;
  bool visVaries;
  std::string name;
  std::string typeName;
  std::string displayName;
};


static
std::vector<UsdPrim> _GetAllPrimsOfType(UsdStagePtr const &stage, 
                                        TfType const& schemaType);

static PrimInfo GetPrimInfo(const UsdPrim &prim, const UsdTimeCode time);

}


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_UTILS_H