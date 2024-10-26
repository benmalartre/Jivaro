#ifndef JVR_UI_CONTENTBROWSER_H
#define JVR_UI_CONTENTBROWSER_H

#include <bitset>
#include "../common.h"
#include "../utils/icons.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../ui/textFilter.h"

JVR_NAMESPACE_OPEN_SCOPE

static size_t CONTENTBROWSER_LINE_HEIGHT = 20;

struct ContentBrowserOptions {
  bool _filterAnonymous = true;
  bool _filterFiles = true;
  bool _filterUnmodified = true;
  bool _filterModified = true;
  bool _filterStage = true;
  bool _filterLayer = true;
  bool _showAssetName = false;
  bool _showIdentifier = true;
  bool _showDisplayName = false;
  bool _showRealPath = false;
};

class ContentBrowserUI : public BaseUI
{
public:
  ContentBrowserUI(View* parent);
  ~ContentBrowserUI() override;

  void MouseButton(int action, int button, int mods) override{};
  void MouseMove(int x, int y) override{};
  bool Draw() override;

private:
  static ImGuiWindowFlags _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_CONTENTBROWSER_H