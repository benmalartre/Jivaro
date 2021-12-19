#ifndef JVR_UTILS_SCANCODE_H
#define JVR_UTILS_SCANCODE_H

#include "../common.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


JVR_NAMESPACE_OPEN_SCOPE

static int MAPPED_KEYS[GLFW_KEY_LAST+1];
static bool KEY_MAP_INITIALIZED = false;

static void BuildKeyMap()
{
  for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
    MAPPED_KEYS[key] = key;
    if (key < 65 || key > 90)continue;
    const char* keyName = glfwGetKeyName(key, 0);
    if (keyName[0] == 'a') MAPPED_KEYS[key] = GLFW_KEY_A;
    else if (keyName[0] == 'b') MAPPED_KEYS[key] = GLFW_KEY_B;
    else if (keyName[0] == 'c') MAPPED_KEYS[key] = GLFW_KEY_C;
    else if (keyName[0] == 'd') MAPPED_KEYS[key] = GLFW_KEY_D;
    else if (keyName[0] == 'e') MAPPED_KEYS[key] = GLFW_KEY_E;
    else if (keyName[0] == 'f') MAPPED_KEYS[key] = GLFW_KEY_F;
    else if (keyName[0] == 'g') MAPPED_KEYS[key] = GLFW_KEY_G;
    else if (keyName[0] == 'h') MAPPED_KEYS[key] = GLFW_KEY_H;
    else if (keyName[0] == 'i') MAPPED_KEYS[key] = GLFW_KEY_I;
    else if (keyName[0] == 'j') MAPPED_KEYS[key] = GLFW_KEY_J;
    else if (keyName[0] == 'k') MAPPED_KEYS[key] = GLFW_KEY_K;
    else if (keyName[0] == 'l') MAPPED_KEYS[key] = GLFW_KEY_L;
    else if (keyName[0] == 'm') MAPPED_KEYS[key] = GLFW_KEY_M;
    else if (keyName[0] == 'n') MAPPED_KEYS[key] = GLFW_KEY_N;
    else if (keyName[0] == 'o') MAPPED_KEYS[key] = GLFW_KEY_O;
    else if (keyName[0] == 'p') MAPPED_KEYS[key] = GLFW_KEY_P;
    else if (keyName[0] == 'q') MAPPED_KEYS[key] = GLFW_KEY_Q;
    else if (keyName[0] == 'r') MAPPED_KEYS[key] = GLFW_KEY_R;
    else if (keyName[0] == 's') MAPPED_KEYS[key] = GLFW_KEY_S;
    else if (keyName[0] == 't') MAPPED_KEYS[key] = GLFW_KEY_T;
    else if (keyName[0] == 'u') MAPPED_KEYS[key] = GLFW_KEY_U;
    else if (keyName[0] == 'v') MAPPED_KEYS[key] = GLFW_KEY_V;
    else if (keyName[0] == 'w') MAPPED_KEYS[key] = GLFW_KEY_W;
    else if (keyName[0] == 'x') MAPPED_KEYS[key] = GLFW_KEY_X;
    else if (keyName[0] == 'y') MAPPED_KEYS[key] = GLFW_KEY_Y;
    else if (keyName[0] == 'z') MAPPED_KEYS[key] = GLFW_KEY_Z;
  }
}

static int GetMappedKey(int key)
{
  return MAPPED_KEYS[key];
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_UTILS_SCANCODE_H

