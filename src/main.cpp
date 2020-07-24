
#include "common.h"
#include "app/application.h"

AMN_NAMESPACE_USING_DIRECTIVE

int main(void)
{
  glfwInit();
  AMN_APPLICATION = new Application(1024,720);
  AMN_APPLICATION->Init();
  AMN_APPLICATION->MainLoop();
  glfwTerminate();
  return 1;
}