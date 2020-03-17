#pragma once

#include "../default.h"
#include "../embree/context.h"
#include "../app/ui.h"
#include "../utils/utils.h"

AMN_NAMESPACE_OPEN_SCOPE

static int 
NUM_MENU_FILE_ITEMS = 5;

static const char* 
MENU_FILE_ITEMS[] = {
  "Open",
  "Save",
  "SaveAs",
  "Import",
  "Export"
};

class AmnMenuUI : public AmnUI
{
  public:
    AmnMenuUI(AmnView* parent);
    ~AmnMenuUI();
    
    // overrides
    void MouseButton(int action, int button, int mods) override{};
    void MouseMove(int x, int y) override{};
    void Draw() override;
  private:

};

AMN_NAMESPACE_CLOSE_SCOPE