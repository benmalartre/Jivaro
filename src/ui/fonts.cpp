#include "../ui/fonts.h"
#include "../utils/files.h"

JVR_NAMESPACE_OPEN_SCOPE


ImFontAtlas* SHARED_ATLAS = NULL;
ImFont* FONTS_SMALL[NUM_FONT_SIZE] = {};
ImFont* FONTS_MEDIUM[NUM_FONT_SIZE] = {};
ImFont* FONTS_LARGE[NUM_FONT_SIZE] = {};
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
    13.0f, 
    &fontConfig, 
    nullptr);

  // Merge Icons in primary font
  static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig iconsConfig;
  iconsConfig.MergeMode = true;
  iconsConfig.PixelSnapH = true;
  
  SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
    FontAwesomeFree6_compressed_data,
    FontAwesomeFree6_compressed_size,
    12.0f,
    &iconsConfig, 
    iconRanges);
  
  // load graph fonts
  std::string exeFolder = GetInstallationFolder();
  std::string fontPath;
  for (int i = 0; i < NUM_FONT_SIZE; ++i) {
    FONTS_SMALL[i] = SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
      ibmplexsansmediumfree_compressed_data,
      ibmplexsansmediumfree_compressed_size,
      FONT_SIZE_SMALL * FONT_SIZE_FACTOR[i],
      NULL,
      SHARED_ATLAS->GetGlyphRangesDefault());

    FONTS_MEDIUM[i] = SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
      ibmplexsansmediumfree_compressed_data,
      ibmplexsansmediumfree_compressed_size,
      FONT_SIZE_MEDIUM * FONT_SIZE_FACTOR[i],
      NULL,
      SHARED_ATLAS->GetGlyphRangesDefault());

    FONTS_LARGE[i] = SHARED_ATLAS->AddFontFromMemoryCompressedTTF(
      ibmplexsansmediumfree_compressed_data,
      ibmplexsansmediumfree_compressed_size,
      FONT_SIZE_LARGE * FONT_SIZE_FACTOR[i],
      NULL,
      SHARED_ATLAS->GetGlyphRangesDefault());
  }
}

void DeleteFontAtlas()
{
  if (SHARED_ATLAS)delete SHARED_ATLAS;
}


JVR_NAMESPACE_CLOSE_SCOPE
