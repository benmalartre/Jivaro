
#include "common.h"
#include "app/application.h"

PXR_NAMESPACE_USING_DIRECTIVE


int main(void)
{
  glfwInit();
  APPLICATION = new Application(1024,720);
  APPLICATION->Init();
  APPLICATION->MainLoop();
  glfwTerminate();
  return 1;
}