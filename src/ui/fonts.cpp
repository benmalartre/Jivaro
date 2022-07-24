#include "../ui/fonts.h"
#include "../utils/files.h"

JVR_NAMESPACE_OPEN_SCOPE


ImFontAtlas* SHARED_ATLAS = NULL;
ImFont* FONTS[NUM_FONT_SIZE] = {};
ImFont* DEFAULT_FONT = NULL;

//
// Shared Font Atlas
//
void CreateFontAtlas()
{
  SHARED_ATLAS = new ImFontAtlas();
  
  // Font
  ImFontConfig fontConfig;

  // Load primary font
  //auto fontDefault = io.Fonts->AddFontDefault(&fontConfig); // DroidSans
  DEFAULT_FONT = SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
    ibmplexsansmediumfree_compressed_data,
    ibmplexsansmediumfree_compressed_size, 
    16.0f, 
    &fontConfig, 
    nullptr);

  // Merge Icons in primary font
  static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig iconsConfig;
  iconsConfig.MergeMode = true;
  iconsConfig.PixelSnapH = true;
  SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
    fontawesomefree5_compressed_data, 
    fontawesomefree5_compressed_size, 
    16.0f,
    &iconsConfig, 
    iconRanges);

  // load fonts
  std::string exeFolder = GetInstallationFolder();
  std::string fontPath;
  for (int i = 0; i < NUM_FONT_SIZE; ++i) {
    fontPath = exeFolder + "/fonts/roboto/Roboto-Regular.ttf";
    //fontPath = exeFolder + "/fonts/opensans/OpenSans-Regular.ttf";
    FONTS[i] = SHARED_ATLAS->AddFontFromFileTTF(
      fontPath.c_str(),
      FONT_SIZE[i],
      NULL,
      SHARED_ATLAS->GetGlyphRangesDefault()
    );
  }
}

void DeleteFontAtlas()
{
  if (SHARED_ATLAS)delete SHARED_ATLAS;
}


JVR_NAMESPACE_CLOSE_SCOPE
