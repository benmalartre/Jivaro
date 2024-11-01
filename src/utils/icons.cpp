#include "../utils/icons.h"
#include "../utils/files.h"

JVR_NAMESPACE_OPEN_SCOPE

IconList ICONS = IconList(3);

void IconHoverDatas(HioImage::StorageSpec* storage, int nchannels)
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

static uint16_t GetIconResolution(ICON_SIZE size) 
{
  uint16_t s;
  switch(size) {
    case(0):s=16;break;
    case(1):s=24;break;
    case(2):s=32;break;
    default:s=64;break;
  }
  return s;
}

GLuint CreateIconFromImage(const std::string& filename,
  int index, ICON_SIZE size)
{
  std::cout << "create icon from image start " << std::endl;
  HioImageSharedPtr img = HioImage::OpenForReading(filename);
  int s = GetIconResolution(size); 
  HioImage::StorageSpec storage;
  storage.width = s;
  storage.height = s ;
  storage.flipped = false;
  storage.format = img->GetFormat();
  storage.data = new char[(size_t)storage.width * (size_t)storage.height * img->GetBytesPerPixel()];

  img->Read(storage);

  //IconHoverDatas(&storage, img->GetBytesPerPixel());
  if(!ICONS[size].size())
    ICONS[size].resize(ICON_MAX_ID);

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
  std::cout << "create icon from image end " << std::endl;
  return tex;
}

void InitializeIcons()
{
  std::string installDir = GetInstallationFolder();
  std::string iconDir = installDir + SEPARATOR + "icons";
  std::cout << "icon dir : " << iconDir << std::endl;

  for (size_t i=0; i < ICON_MAX_ID; ++i) {
    //ICONS[size][index] = { s, tex };
    GLuint tex_small[3];
    GLuint tex_medium[3];
    GLuint tex_large[3];
    
    for(size_t j=0; j < 3; ++j) {
      std::string filename = 
        iconDir + SEPARATOR + ICON_NAMES[i] + "_" + ICON_SUFFIX[j] + ".png";
      std::cout << filename << std::endl;
      if (FileExists(filename) &&
        HioImage::IsSupportedImageFile(filename)) {
          std::cout << "image is supported.." << std::endl;
        tex_small[j] = CreateIconFromImage(filename, i, ICON_SIZE_SMALL);
        tex_medium[j] = CreateIconFromImage(filename, i, ICON_SIZE_MEDIUM);
        tex_large[j] = CreateIconFromImage(filename, i, ICON_SIZE_LARGE);
      }
      else {
        std::cout << "DID NOT FOUND " << filename << std::endl;
      }
    }
    ICONS[ICON_SIZE_SMALL][i] = Icon {
      GetIconResolution(ICON_SIZE_SMALL), 
      tex_small[0], tex_small[1], tex_small[2]
    };
    ICONS[ICON_SIZE_MEDIUM][i] = Icon {
      GetIconResolution(ICON_SIZE_MEDIUM), 
      tex_medium[0], tex_medium[1], tex_medium[2]
    };
    ICONS[ICON_SIZE_LARGE][i] = Icon {
      GetIconResolution(ICON_SIZE_LARGE), 
      tex_large[0], tex_large[1], tex_large[2]
    };
  }  
}

void TerminateIcons()
{

}

JVR_NAMESPACE_CLOSE_SCOPE