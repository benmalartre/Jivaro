#include "icons.h"

AMN_NAMESPACE_OPEN_SCOPE

std::map<std::string, Icon> AMN_ICONS;

void IconHoverDatas(pxr::GlfImage::StorageSpec* storage, int nchannels)
{
  uint32_t* pixels = (uint32_t*)storage->data;
  if (nchannels == 4)
  {
    for (int y = 0; y<storage->height; ++y)
    {
      for (int x = 0; x<storage->width; ++x)
      {
        uint32_t index = y * storage->width + x;
        uint32_t pixel = pixels[index];
        uint8_t alpha = (pixel & 0xFF000000) >> 24;
        if (alpha > 0)
        {
          uint8_t blue = (pixel & 0x00FF0000) >> 16;
          uint8_t green = (pixel & 0x0000FF00) >> 8;
          uint8_t red = (pixel & 0x000000FF);

          pixels[index] = 0xFF0000FF;
        }
      }
    }
  }
}

void CreateIconFromImage(const std::string& filename,
  const std::string& name, ICON_SIZE size)
{
  pxr::GlfImageSharedPtr img = pxr::GlfImage::OpenForReading(filename);

  pxr::GlfImage::StorageSpec storage;
  storage.width = size;
  storage.height = size;
  storage.flipped = false;
  storage.type = ICON_TYPE;
  storage.format = ICON_FORMAT;
  storage.data = new char[size * size * img->GetBytesPerPixel()];

  img->Read(storage);

  //IconHoverDatas(&storage, img->GetBytesPerPixel());

  if (AMN_ICONS.find(name) == AMN_ICONS.end())
  {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
      ICON_FORMAT, ICON_TYPE, storage.data);

    AMN_ICONS[name] = { size, tex };
  }
}

void AMNInitializeIcons()
{
  std::string installDir = GetInstallationFolder();
  std::string iconDir = installDir + "/../../icons";
  std::vector<std::string> filenames;
  int n = GetFilesInDirectory(iconDir.c_str(), filenames);

  for (const auto f : filenames)
  {
    std::string name = GetFileName(f);
    std::string filename = iconDir + SEPARATOR + f;
    if (pxr::GlfImage::IsSupportedImageFile(filename))
    {
      CreateIconFromImage(filename, name, ICON_MID);
    }
  }
}

void AMNTerminateIcons()
{

}
AMN_NAMESPACE_CLOSE_SCOPE