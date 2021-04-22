#ifndef AMN_UI_LAYERS_H
#define AMN_UI_LAYERS_H
#pragma once

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/layer.h>

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

class LayersUI : public BaseUI
{
  public:
    LayersUI(View* parent);
    ~LayersUI();
    
    // overrides
    void MouseButton(int action, int button, int mods) override{};
    void MouseMove(int x, int y) override{};
    bool Draw() override;

    LayerItem& AddLayer(View* view);
  private:
    std::vector<LayerItem> _items;
    MenuItem*             _current;
};

#endif