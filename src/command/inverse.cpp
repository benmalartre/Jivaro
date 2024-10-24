#include "../command/inverse.h"
#include "../command/block.h"
#include "../command/router.h"

#include <iostream>

JVR_NAMESPACE_OPEN_SCOPE

void UndoInverse::_Append(std::function<bool()> inverse) {
  _inversion.push_back(inverse);
}

void UndoInverse::_Invert() {
  SdfChangeBlock changeBlock;
  for (std::vector<std::function<bool()>>::reverse_iterator it = 
  _inversion.rbegin(); it != _inversion.rend(); ++it) {
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

  _Invert();
  _Clear();
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

JVR_NAMESPACE_CLOSE_SCOPE