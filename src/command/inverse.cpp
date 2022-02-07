#include "../command/inverse.h"
#include "../command/block.h"
#include "../command/router.h"

PXR_NAMESPACE_OPEN_SCOPE

void UndoInverse::_Append(std::function<bool()> inverse) {
  _inversion.push_back(inverse);
}

void UndoInverse::_Invert() {
  pxr::SdfChangeBlock changeBlock;
  for (std::vector<std::function<bool()>>::reverse_iterator it = _inversion.rbegin();
    it != _inversion.rend(); ++it) {
    (*it)();
  }
}

void UndoInverse::Invert() {
  UndoRouter& router = UndoRouter::Get();
  if (router._depth != 0) {
      TF_CODING_ERROR(
          "Inversion during open edit block may result in corrupted undo "
          "stack.");
  }

  // open up an edit change block to capture the inverse of the inversion
  UndoBlock editBlock;
  _Invert();
  _Clear();
  // adopt the edits and clear the listeners inversion tracker.
  // when the change block is popped, no notices will be sent
  // TODO: Do we want a more explicit version of this that
  // explicitly marks that we are inverting an undo/redo as
  // opposed to a new edit?
  _Adopt(router._inversion);
  router._inversion._Clear();
}

void UndoInverse::_Clear() { _inversion.clear(); }

void UndoInverse::_Adopt(const UndoInverse& inversion) {
  for (const auto& inverse : inversion._inversion) {
    _inversion.push_back(inverse);
  }
}

UndoInverse::UndoInverse(UndoRouter& router) {
    _Adopt(router._inversion);
    router._inversion._Clear();
}

PXR_NAMESPACE_CLOSE_SCOPE