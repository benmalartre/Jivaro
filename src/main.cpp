
#include "common.h"
#include "app/application.h"

JVR_NAMESPACE_USING_DIRECTIVE


int main(void)
{
  glfwInit();
  APPLICATION = new Application(1024,720);
  APPLICATION->Init();
  while (APPLICATION->Update());
  glfwTerminate();
  return 1;
}