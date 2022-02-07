#include "../command/block.h"
#include "../command/router.h"


PXR_NAMESPACE_OPEN_SCOPE

void UsdUndoBlock::_Initialize() {
  UsdUndoRouter& router = UsdUndoRouter::Get();
  TF_VERIFY(router._depth >= 0);
  TF_DEBUG(USDQT_DEBUG_UNDOSTACK).Msg(
    "--Opening undo block inverse at depth '%i'.\n", router._depth);
  if (router._depth == 0) {
    if (router._inversion._GetSize() != 0) {
      TF_CODING_ERROR(
        "Opening fragmented undo block. This may be because of an undo "
        "command running inside of an edit block.");
    }
  }
  router._depth++;
}

UsdUndoBlock::UsdUndoBlock() {
  _Initialize();
}

UsdUndoBlock::~UsdUndoBlock() {
  UsdUndoRouter& router = UsdUndoRouter::Get();
  router._depth--;
  TF_VERIFY(router._depth >= 0);
  if (router._depth == 0) {
    if (router._inversion._GetSize() < 1) {
      TF_DEBUG(USDQT_DEBUG_UNDOSTACK)
        .Msg("Skipping sending notice for empty undo block.\n");
    }
    else {
      UsdQt::UndoStackNotice().Send();
      TF_DEBUG(USDQT_DEBUG_UNDOSTACK).Msg("Undo Notice Sent.\n");
      if (router._inversion._GetSize() > 0) {
        TF_CODING_ERROR("All edits have not been adopted. Undo stack may be incomplete.");
        router._inversion._Clear();
      }
    }
  }
  TF_DEBUG(USDQT_DEBUG_UNDOSTACK).Msg(
    "--Closed undo block inverse at depth '%i'.\n", router._depth);
}

PXR_NAMESPACE_CLOSE_SCOPE