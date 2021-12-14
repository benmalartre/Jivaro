#ifndef AMN_UTILS_SCANCODE_H
#define AMN_UTILS_SCANCODE_H

#include "../common.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


AMN_NAMESPACE_OPEN_SCOPE
#define NUM_PRINTABLE_KEYS 26

extern int AMN_KEY_SCANCODES[NUM_PRINTABLE_KEYS];
extern bool AMN_KEY_SCANCODES_INITIALIZED;

#define AMN_INITIALIZE_SCAN_CODES                                               \
    if(AMN_KEY_SCANCODES_INITIALIZED) {                                         \
        for(size_t i = 0; i < NUM_PRINTABLE_KEYS; ++i)   {                       \
            std::cout << i << " : " <<  glfwGetKeyScancode(GLFW_KEY_A + i) << std::endl; \
            AMN_KEY_SCANCODES[i] = glfwGetKeyScancode(GLFW_KEY_A + i);   }       \
        AMN_KEY_SCANCODES_INITIALIZED = true;                                   \
    }                                                                           \


#define AMN_KEY_A AMN_KEY_SCANCODES[0]
#define AMN_KEY_B AMN_KEY_SCANCODES[1]
#define AMN_KEY_C AMN_KEY_SCANCODES[2]
#define AMN_KEY_D AMN_KEY_SCANCODES[3]
#define AMN_KEY_E AMN_KEY_SCANCODES[4]
#define AMN_KEY_F AMN_KEY_SCANCODES[5]
#define AMN_KEY_G AMN_KEY_SCANCODES[6]
#define AMN_KEY_H AMN_KEY_SCANCODES[7]
#define AMN_KEY_I AMN_KEY_SCANCODES[8]
#define AMN_KEY_J AMN_KEY_SCANCODES[9]
#define AMN_KEY_K AMN_KEY_SCANCODES[10]
#define AMN_KEY_L AMN_KEY_SCANCODES[11]
#define AMN_KEY_M AMN_KEY_SCANCODES[12]
#define AMN_KEY_N AMN_KEY_SCANCODES[13]
#define AMN_KEY_O AMN_KEY_SCANCODES[14]
#define AMN_KEY_P AMN_KEY_SCANCODES[15]
#define AMN_KEY_Q AMN_KEY_SCANCODES[16]
#define AMN_KEY_R AMN_KEY_SCANCODES[17]
#define AMN_KEY_S AMN_KEY_SCANCODES[18]
#define AMN_KEY_T AMN_KEY_SCANCODES[19]
#define AMN_KEY_U AMN_KEY_SCANCODES[20]
#define AMN_KEY_V AMN_KEY_SCANCODES[21]
#define AMN_KEY_W AMN_KEY_SCANCODES[22]
#define AMN_KEY_X AMN_KEY_SCANCODES[23]
#define AMN_KEY_Y AMN_KEY_SCANCODES[24]
#define AMN_KEY_Z AMN_KEY_SCANCODES[25]

AMN_NAMESPACE_CLOSE_SCOPE

#endif //AMN_UTILS_SCANCODE_H

