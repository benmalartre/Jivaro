#include "icons.h"

AMN_NAMESPACE_OPEN_SCOPE

AmnIconMap AMN_ICONS = AmnIconMap(3);

void IconHoverDatas(pxr::HioImage::StorageSpec* storage, int nchannels)
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
  const std::string& name, ICON_SIZE index)
{
  pxr::HioImageSharedPtr img = pxr::HioImage::OpenForReading(filename);
  size_t s = (index + 1) * 16;
  pxr::HioImage::StorageSpec storage;
  storage.width = s;
  storage.height = s ;
  storage.flipped = false;
  storage.format = pxr::HioFormat::HioFormatInt32Vec4;
  storage.data = new char[storage.width * storage.height * img->GetBytesPerPixel()];

  img->Read(storage);

  //IconHoverDatas(&storage, img->GetBytesPerPixel());

  if (AMN_ICONS[index].find(name) == AMN_ICONS[index].end())
  {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, storage.width, storage.height, 0,
      ICON_FORMAT, ICON_TYPE, storage.data);

    AMN_ICONS[index][name] = { s, tex };
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
    if (pxr::HioImage::IsSupportedImageFile(filename))
    {
      CreateIconFromImage(filename, name, AMN_ICON_SMALL);
      CreateIconFromImage(filename, name, AMN_ICON_MEDIUM);
      CreateIconFromImage(filename, name, AMN_ICON_LARGE);
    }
  }
}

void AMNTerminateIcons()
{

}
AMN_NAMESPACE_CLOSE_SCOPE