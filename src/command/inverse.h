
#ifndef UNDO_INVERSE_H
#define UNDO_INVERSE_H

#include <functional>
#include <vector>

#include <pxr/pxr.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/base/tf/weakPtr.h>
#include <pxr/base/tf/refPtr.h>
#include <pxr/base/tf/declarePtrs.h>

PXR_NAMESPACE_OPEN_SCOPE

class UndoRouter;

/// \class UndoInverse
/// 
/// An UndoInverse is a list of invertible edits to one or more SdfLayers 
/// which may span multiple stages.
/// 
/// It may contain more than one edit.  When an edit is inverted, say by an undo 
/// operation, it automatically converts itself into a redo operation by
/// tracking edits in the UndoRouter which spawned it.
///
/// This is the object you should store in your application's native undo stack.
/// The implementation of undo and redo should be the same, simply calling
/// inverse.Invert().
///

class UndoInverse {
private:
    std::string _name;
    std::vector<std::function<bool()>> _inversion;
    void _Invert();

    void _Append(std::function<bool()>);
    void _Clear();
    size_t _GetSize() { return _inversion.size(); }
    void _Adopt(const UndoInverse& inversion);
    explicit UndoInverse(UndoRouter& router);
    
public:
    UndoInverse(){}
    /// \brief Apply the inverse functions.
    ///
    /// When Invert() has been called, this object now stores the Inverse of
    /// the Inverse.  Calling Invert() twice in a row should result in the same
    /// state.
    ///
    /// WARNING: This is not reentrant.  When Invert is called, no other threads
    /// may engage in edits that affect the router.  If this warning is ignored, 
    /// inverses may get incorrectly routed.
    void Invert();
    friend class UndoRouter;
    friend class UndoBlock;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif