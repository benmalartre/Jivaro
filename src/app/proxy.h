#ifndef JVR_APPLICATION_PROXY_H
#define JVR_APPLICATION_PROXY_H
#include <cassert>
#include <pxr/usd/sdf/listEditorProxy.h>
#include <pxr/usd/sdf/reference.h>

// ExtraArgsT is used to pass additional arguments as the function passed as visitor
// might need more than the operation and the item
template <typename PolicyT, typename FuncT, typename ...ExtraArgsT>
static void IterateListEditorItems(const SdfListEditorProxy<PolicyT> &listEditor,
    const FuncT &func, ExtraArgsT... args ) {
    // TODO: should we check if the list is already all explicit ??
    for (const typename PolicyT::value_type &item : listEditor.GetExplicitItems()) {
        func("explicit", item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetOrderedItems()) {
        func("ordered", item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetAddedItems()) {
        func("add", item, args...); // return "add" as TfToken instead ?
    }
    for (const typename PolicyT::value_type &item : listEditor.GetPrependedItems()) {
        func("prepend", item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetAppendedItems()) {
        func("append", item, args...);
    }
    for (const typename PolicyT::value_type &item : listEditor.GetDeletedItems()) {
        func("delete", item, args...);
    }
}

/// The operations available on a SdfListEditor
constexpr int GetListEditorOperationSize() { return 4; }
inline const char *GetListEditorOperationName(int index) {
    constexpr const char *names[GetListEditorOperationSize()] = {
        "Add", "Prepend", "Append", "Remove"
    };
    return names[index];
}

template <typename PolicyT>
void CreateListEditorOperation(SdfListEditorProxy<PolicyT> &&listEditor, int operation,
                                typename SdfListEditorProxy<PolicyT>::value_type item) {
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
    default:
        assert(0);
    }
}

#endif // JVR_APPLICATION_PROXY_H