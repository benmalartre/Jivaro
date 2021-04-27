
#include "common.h"
#include "app/application.h"

AMN_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  glfwInit();
  std::cout << "APP STARTUP" << std::endl;
  AMN_APPLICATION = new Application(1024,720);
  std::cout << "APP CREATED" << std::endl;
  AMN_APPLICATION->Init();
  std::cout << "APP INIT" << std::endl;
  AMN_APPLICATION->MainLoop();
  std::cout << "APP LOOP" << std::endl;
  glfwTerminate();
  return 1;
}