
#include "common.h"
#include "app/application.h"

PXR_NAMESPACE_USING_DIRECTIVE


int main(void)
{
  glfwInit();
  APPLICATION = new Application(1024,720);
  std::cout << APPLICATION << std::endl;
  std::cout << "INITIALIZE..." << std::endl;
  APPLICATION->Init();
  
  std::cout << "UPDATE..." << std::endl;
  APPLICATION->MainLoop();
  glfwTerminate();
  return 1;
}