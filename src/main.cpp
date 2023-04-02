
#include "common.h"
#include "utils/keys.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE


int main(void)
{
  glfwInit();
  BuildKeyMap();
  APPLICATION = new Application(1024,720);
  std::cout << "##############################################" << std::endl;
  std::cout << "MAIN INIT" << std::endl;
  std::cout << "##############################################" << std::endl;
  APPLICATION->Init();
  std::cout << "##############################################" << std::endl;
  std::cout << "MAIN UPDATE" << std::endl;
  std::cout << "##############################################" << std::endl;
  while (APPLICATION->Update());
  glfwTerminate();
  return 1;
}