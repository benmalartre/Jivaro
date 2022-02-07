

#include <pxr/base/tf/instantiateSingleton.h>
#include <pxr/base/tf/refPtr.h>

#include <boost/range/adaptor/sliced.hpp>

#include "../command/router.h"
#include "../command/block.h"
#include "../command/delegate.h"

using BOOST_NS::adaptors::slice;

PXR_NAMESPACE_OPEN_SCOPE

/*
TF_REGISTRY_FUNCTION(TfType) {
  TfType::Define<UsdQt::UndoStackNotice, TfType::Bases<TfNotice> >();
}

TF_INSTANTIATE_SINGLETON(UsdQtUndoRouter);

namespace UsdQt {
  UndoStackNotice::UndoStackNotice() {}
}
*/
UndoRouter::UndoRouter() {
  // TfDebug::Enable(USDQT_DEBUG_UNDOSTACK);
}

bool UndoRouter::TrackLayer(const SdfLayerHandle& layer) {
  layer->SetStateDelegate(LayerStateDelegate::New());
  return true;
}

void UndoRouter::_AddInverse(std::function<bool()> inverse) {
  UndoBlock undoBlock;
  _inversion._Append(inverse);
}

UndoRouter& UndoRouter::Get() {
  return pxr::TfSingleton<UndoRouter>::GetInstance();
}

bool UndoRouter::TransferEdits(UndoInverse* inverse) {
  inverse->_Adopt(Get()._inversion);
  Get()._inversion._Clear();
  return true;
}

void UndoRouter::_Mute() {
  Get()._muteDepth++;
}

void UndoRouter::_Unmute() {
  Get()._muteDepth--;
  if (Get()._muteDepth < 0) {
    TF_CODING_ERROR("Mute depth error.");
  }
}

bool UndoRouter::IsMuted() {
  return Get()._muteDepth > 0;
}

PXR_NAMESPACE_CLOSE_SCOPE