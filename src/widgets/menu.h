#pragma once

#include "../default.h"
#include "../context.h"
#include "../ui.h"
#include "../utils.h"

namespace AMN {
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

  class MenuUI : public UI
  {
    public:
      MenuUI(View* parent);
      ~MenuUI();
     
      // overrides
      void Event() override;
      void Draw() override;
    private:

  };
} // namespace AMN
