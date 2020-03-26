#pragma once

#include "../app/ui.h"
#include "../utils/ui.h"

AMN_NAMESPACE_OPEN_SCOPE

class MenuUI : public BaseUI
{
  public:
    MenuUI(View* parent);
    ~MenuUI();
    
    // overrides
    void MouseButton(int action, int button, int mods) override{};
    void MouseMove(int x, int y) override{};
    void Draw() override;
  private:
    bool _showDemoWindow;
};

AMN_NAMESPACE_CLOSE_SCOPE