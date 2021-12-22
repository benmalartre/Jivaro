#ifndef JVR_UI_LAYERS_H
#define JVR_UI_LAYERS_H

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/layer.h>

#include "../ui/ui.h"

PXR_NAMESPACE_OPEN_SCOPE

/*
struct SummaryStats
{
    size_t numSpecs = 0;
    size_t numPrimSpecs = 0;
    size_t numPropertySpecs = 0;
    size_t numFields = 0;
    size_t numSampleTimes = 0;
};

SummaryStats
GetSummaryStats(pxr::SdfLayerHandle const &layer)
{
    SummaryStats stats;
    layer->Traverse(
        pxr::SdfPath::AbsoluteRootPath(), [&stats, &layer](
          pxr::SdfPath const &path) {
            ++stats.numSpecs;
            stats.numPrimSpecs += path.IsPrimPath();
            stats.numPropertySpecs += path.IsPropertyPath();
            stats.numFields += layer->ListFields(path).size();
        });
    stats.numSampleTimes = layer->ListAllTimeSamples().size();
    return stats;
}

struct LayerItem {

};
*/
class View;
class LayersUI : public BaseUI
{
  public:
    LayersUI(View* parent);
    ~LayersUI();
    
    // overrides
    /*
    void MouseButton(int action, int button, int mods) override;
    void MouseMove(int x, int y) override;
    */
    bool Draw() override;


  private:
    static ImGuiWindowFlags               _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif