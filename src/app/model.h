#ifndef JVR_APPLICATION_MODEL_H
#define JVR_APPLICATION_MODEL_H

#include <pxr/base/gf/vec3d.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

#include "../common.h"
#include "../app/time.h"
#include "../app/notice.h"
#include "../app/selection.h"
#include "../exec/execution.h"
#include "../exec/sceneIndex.h"
#include "../command/manager.h"


JVR_NAMESPACE_OPEN_SCOPE

class Engine;
class Scene;

class Model : public TfWeakBase
{
public:
  static const char* name;
  // constructor
  Model();

  // destructor
  ~Model();

  void Init();
  void Update(const float time);

  // browse file
  std::string BrowseFile(int x, int y, const char* folder, const char* filters[], 
    const int numFilters, const char* name="Browse", bool readOrWrite=false);

  void New(const std::string& filename);
  void Open(const std::string& filename);
  void Save();
  void SaveAs(const std::string& filename);

  // selection
  Selection* GetSelection(){return &_selection;};
  void SetSelection(const SdfPathVector& selection);
  void ToggleSelection(const SdfPathVector& selection);
  void AddToSelection(const SdfPathVector& path);
  void RemoveFromSelection(const SdfPathVector& path);
  void ClearSelection();
  GfBBox3d GetSelectionBoundingBox();
  GfBBox3d GetStageBoundingBox();

  // commands
  void AddCommand(std::shared_ptr<Command> command);
  void AddDeferredCommand(CALLBACK_FN fn);
  void ExecuteDeferredCommands();

  // engines
  void AddEngine(Engine* engine);
  void RemoveEngine(Engine* engine);
  void SetActiveEngine(Engine* engine);
  Engine* GetActiveEngine();
  std::vector<Engine*> GetEngines() { return _engines; };

  // stage cache
  UsdStageRefPtr& GetStage(){return _stage;};
  void SetStage(UsdStageRefPtr& stage);


  // execution
  void ToggleExec();
  void SetExec(bool state);
  bool GetExec();

  virtual void InitExec();
  virtual void UpdateExec(float time);
  virtual void TerminateExec();
  virtual void SendExecViewEvent(const ViewEventData *data);

    // usd stages
  //std::vector<UsdStageRefPtr>& GetStages(){return _stages;};
  UsdStageRefPtr GetDisplayStage();
  UsdStageRefPtr GetWorkStage();

  SdfLayerRefPtr GetCurrentLayer();

  void AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex);
  HdSceneIndexBaseRefPtr GetEditableSceneIndex();

  void SetCurrentSceneIndex(HdSceneIndexBaseRefPtr sceneIndex);
  HdSceneIndexBaseRefPtr GetFinalSceneIndex();
  HdSceneIndexPrim GetPrim(SdfPath primPath);

  UsdPrim GetUsdPrim(SdfPath primPath);
  UsdPrimRange GetAllPrims();

protected:
  void _SetEmptyStage();
  void _LoadUsdStage(const std::string usdFilePath);


private:

  // filename
  std::string                       _filename;

  // selection 
  Selection                         _selection;

  // stage
  UsdStageRefPtr                    _stage;
  SdfLayerRefPtr                    _layer;

  // engines
  std::vector<Engine*>              _engines;
  Engine*                           _activeEngine;

  // model
  SdfLayerRefPtr                    _rootLayer, _sessionLayer;
  UsdImagingStageSceneIndexRefPtr   _stageSceneIndex;
  HdSceneIndexBaseRefPtr            _editableSceneIndex;
  HdMergingSceneIndexRefPtr         _sceneIndexBases;
  HdMergingSceneIndexRefPtr         _finalSceneIndex;

  // execution
  bool                              _execute;
  Execution*                        _exec;
  ExecSceneIndexRefPtr              _execSceneIndex;
  float                             _lastTime;

};

JVR_NAMESPACE_CLOSE_SCOPE // namespace JVR

#endif // JVR_APPLICATION_APPLICATION_H

