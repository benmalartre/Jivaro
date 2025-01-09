#ifndef JVR_UI_PREFERENCES_H
#define JVR_UI_PREFERENCES_H

#include "../ui/ui.h"
#include "../utils/prefs.h"

JVR_NAMESPACE_OPEN_SCOPE

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
GetSummaryStats(SdfLayerHandle const &layer)
{
    SummaryStats stats;
    layer->Traverse(
        SdfPath::AbsoluteRootPath(), [&stats, &layer](
          SdfPath const &path) {
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
class PreferenceUI : public BaseUI
{
  public:
    PreferenceUI(View* parent);
    ~PreferenceUI();
    
    // overrides
    /*
    void MouseButton(int action, int button, int mods) override;
    void MouseMove(int x, int y) override;
    */
    bool Draw() override;
    void DrawNavigation(SdfLayerRefPtr layer);


  private:
    static ImGuiWindowFlags               _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif