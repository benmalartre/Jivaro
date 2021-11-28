//
// Copyright 2020 benmalartre
//
// Unlicensed
//

#include "pxr/pxr.h"
#include "pxr/base/tf/hashmap.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/imaging/hd/drawItem.h"
#include "pxr/imaging/hd/renderPass.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/plugin/LoFi/drawItem.h"
#include "pxr/imaging/plugin/LoFi/shaderCode.h"
#include "pxr/imaging/plugin/LoFi/codeGen.h"
#include "pxr/imaging/plugin/LoFi/resourceRegistry.h"
#include "pxr/imaging/plugin/LoFi/shader.h"
#include <map>

PXR_NAMESPACE_OPEN_SCOPE

/// \class LoFiRenderPass
///
class LoFiRenderPass final : public HdRenderPass 
{
public:
  /// Renderpass constructor.
  ///   \param index The render index containing scene data to render.
  ///   \param collection The initial rprim collection for this renderpass.
  LoFiRenderPass( HdRenderIndex *index,
                  HdRprimCollection const &collection);

  /// Renderpass destructor.
  virtual ~LoFiRenderPass();

protected:
  /// Setup simple GLSL program
  TfToken _GetShaderPath(char const * shader);
  void _GetShaderCode(const TfToken& path, const TfToken& name);
  
  LoFiGLSLProgramSharedPtr _SetupGLSLProgram(const LoFiBinder* binder);

  /// Mark collection as dirty for update
  virtual void _MarkCollectionDirty() override; 

  /// Draw the scene with the bound renderpass state.
  ///   \param renderPassState Input parameters (including viewer parameters)
  ///                          for this renderpass.
  ///   \param renderTags Which rendertags should be drawn this pass.
  virtual void _Execute(HdRenderPassStateSharedPtr const& renderPassState,
                TfTokenVector const &renderTags) override;

private:
  GlfDrawTargetRefPtr                            _drawTarget;

  // draw items are organized by glsl program
  std::map<TfToken, LoFiGLSLProgramSharedPtr>    _programs;
  typedef std::map<TfToken, LoFiDrawItemPtrSet>  _ProgramDrawItemsMap;
  _ProgramDrawItemsMap                           _programDrawItemsMap;

  // -----------------------------------------------------------------------
  // Change tracking state.
  int _collectionVersion;
  int _renderTagVersion;
  bool _collectionChanged;
  HdRenderIndex::HdDrawItemPtrVector _drawItems;
  size_t _drawItemCount;
  bool _drawItemsChanged;
};

PXR_NAMESPACE_CLOSE_SCOPE
