#ifndef JVR_EXEC_SCENEINDEX_H
#define JVR_EXEC_SCENEINDEX_H

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Execution;

TF_DECLARE_REF_PTRS(ExecSceneIndex);

class ExecSceneIndex : public HdSingleInputFilteringSceneIndexBase {
  public:
    static ExecSceneIndexRefPtr New(
      const HdSceneIndexBaseRefPtr  &inputSceneIndex){
        return TfCreateRefPtr(new ExecSceneIndex(inputSceneIndex));}

    ExecSceneIndex(const HdSceneIndexBaseRefPtr  &inputSceneIndex);
    ~ExecSceneIndex();

    void Populate(bool populate);
    void SetExec(Execution* exec);

    virtual HdSceneIndexPrim GetPrim(
      const SdfPath &primPath) const override;

    virtual SdfPathVector GetChildPrimPaths(
      const SdfPath &primPath) const override;

  protected:
    HdSceneIndexPrim _CreateGridPrim();

    virtual void _PrimsAdded(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::AddedPrimEntries &entries) override;

    virtual void _PrimsRemoved(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries) override;

    virtual void _PrimsDirtied(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries) override;

    virtual void _SystemMessage(
        const TfToken &messageType,
        const HdDataSourceBaseHandle &args) override;

  private:
    Execution* _exec;

    SdfPath _gridPath;
    HdSceneIndexPrim _prim;
    bool _isPopulated;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif  // JVR_EXEC_SCENEINDEX_H
