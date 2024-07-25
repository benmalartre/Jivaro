#ifndef UNDOROUTER_H
#define UNDOROUTER_H

#include <pxr/pxr.h>
#include <pxr/base/tf/declarePtrs.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/base/tf/singleton.h>
#include <pxr/base/tf/weakPtr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>

#include "../common.h"
#include "../command/inverse.h"

JVR_NAMESPACE_OPEN_SCOPE

/// \class UndoRouter
///
/// Class used to track edits to one or more SdfLayers.  This is the primary
/// class in usd undo adapter.  The UndoRouter attaches itself to track
/// edits
/// to a layer by spawning a LayerStateDelegate.  It may batch
/// multiple edits by attaching a UndoBlock to it.  Once the last block has
/// been closed, a UsdQt::UndoStackNotice is emitted.  The application's native
/// undo queue system now knows it's safe to adopt the edits tracked by the
/// router into a local UsdQtUndoInverse object.  When undo is called, this
/// object can invert all the edits it represents and transforms itself into
/// a redo.
///
/// The UndoRouter is the linchpin and it's important to maintain its lifetime
/// as long as there is an UndoBlock, UndoInverse, or UndoLayerStateDelegate
/// that is expecting to forward or receive information from it.
///
/// Here is a quick breakdown of the chain of triggers.
/// Usd Edit => Sdf Edit => Delegate => Router => Notice => Native Undo Listener
///
class UndoRouter {
private:
    int _depth = 0;
    UndoInverse _inversion;

    UndoRouter();

    static void _Mute();
    static void _Unmute();

    int _muteDepth = 0;

    
public:
    UndoRouter (const UndoRouter&) = delete;
    UndoRouter& operator= (const UndoRouter&) = delete;
    static UndoRouter& Get();
    void   AddInverse(std::function<bool()> inverse);
    static bool TrackLayer(const pxr::SdfLayerHandle& layer);
    static bool TransferEdits(UndoInverse* inverse);
    static bool IsMuted();

    friend class UndoBlock;
    friend class UndoInverse;
    friend class UndoStateDelegate;
    friend class pxr::TfSingleton<UndoRouter>;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif