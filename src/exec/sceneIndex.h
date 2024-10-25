#ifndef JVR_EXEC_SCENEINDEX_H
#define JVR_EXEC_SCENEINDEX_H

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/pxr.h>

JVR_NAMESPACE_OPEN_SCOPE

class ExecSceneIndex;

TF_DECLARE_REF_PTRS(ExecSceneIndex);

class ExecSceneIndex : public HdSingleInputFilteringSceneIndexBase {
  public:
    static ExecSceneIndexRefPtr New(
      const ExecSceneIndexRefPtr &inputSceneIndex)
    {
      return TfCreateRefPtr(
        new ExecSceneIndex(inputSceneIndex));
    }

    ExecSceneIndex(
      const pxr::HdSceneIndexBaseRefPtr &inputSceneIndex);

    virtual pxr::HdSceneIndexPrim GetPrim(
      const pxr::SdfPath &primPath) const override;

    virtual pxr::SdfPathVector GetChildPrimPaths(
      const pxr::SdfPath &primPath) const override;

    void SetScene(Scene* scene);
    void RemoveScene();
    Scene* GetScene() { return _scene; };
    void UpdateScene();

  protected:
    virtual void _PrimsAdded(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::AddedPrimEntries &entries)
      override;

    virtual void _PrimsRemoved(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries)
      override;

    virtual void _PrimsDirtied(
      const pxr::HdSceneIndexBase &sender,
      const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries)
      override;

  private:
    Scene* _scene;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif  // JVR_EXEC_SCENEINDEX_H
