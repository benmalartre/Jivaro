#ifndef JVR_UNDOBLOCK_H
#define JVR_UNDOBLOCK_H

#include <pxr/pxr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/base/tf/weakPtr.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/base/tf/declarePtrs.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_WEAK_AND_REF_PTRS(UndoRouter);

/// \class UsdUndoBlock
///
/// Similar to an SdfChangeBlock, this will collect multiple edits into a single
/// undo operation.  
///
/// Because edit tracking is done at the Sdf level, it's important to
/// aggressively use UndoBlocks even around single Usd calls.  One Usd call
/// may map to multiple Sdf calls, each spawning their own unique inverse.
///
/// Future refactoring may try to address and smooth over this quirk.
///
class UndoBlock {
private:
    void _Initialize(bool clear=false);

public:
    explicit UndoBlock(bool clear=false);
    ~UndoBlock();
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif  // JVR_UNDOBLOCK_H