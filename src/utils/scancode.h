#ifndef JVR_UTILS_SCANCODE_H
#define JVR_UTILS_SCANCODE_H

#include "../common.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


JVR_NAMESPACE_OPEN_SCOPE
#define NUM_PRINTABLE_KEYS 26

extern int KEY_SCANCODES[NUM_PRINTABLE_KEYS];
extern bool KEY_SCANCODES_INITIALIZED;

#define INITIALIZE_SCAN_CODES                                               \
    if(KEY_SCANCODES_INITIALIZED) {                                         \
        for(size_t i = 0; i < NUM_PRINTABLE_KEYS; ++i)   {                       \
            std::cout << i << " : " <<  glfwGetKeyScancode(GLFW_KEY_A + i) << std::endl; \
            KEY_SCANCODES[i] = glfwGetKeyScancode(GLFW_KEY_A + i);   }       \
        KEY_SCANCODES_INITIALIZED = true;                                   \
    }                                                                           \


#define KEY_A KEY_SCANCODES[0]
#define KEY_B KEY_SCANCODES[1]
#define KEY_C KEY_SCANCODES[2]
#define KEY_D KEY_SCANCODES[3]
#define KEY_E KEY_SCANCODES[4]
#define KEY_F KEY_SCANCODES[5]
#define KEY_G KEY_SCANCODES[6]
#define KEY_H KEY_SCANCODES[7]
#define KEY_I KEY_SCANCODES[8]
#define KEY_J KEY_SCANCODES[9]
#define KEY_K KEY_SCANCODES[10]
#define KEY_L KEY_SCANCODES[11]
#define KEY_M KEY_SCANCODES[12]
#define KEY_N KEY_SCANCODES[13]
#define KEY_O KEY_SCANCODES[14]
#define KEY_P KEY_SCANCODES[15]
#define KEY_Q KEY_SCANCODES[16]
#define KEY_R KEY_SCANCODES[17]
#define KEY_S KEY_SCANCODES[18]
#define KEY_T KEY_SCANCODES[19]
#define KEY_U KEY_SCANCODES[20]
#define KEY_V KEY_SCANCODES[21]
#define KEY_W KEY_SCANCODES[22]
#define KEY_X KEY_SCANCODES[23]
#define KEY_Y KEY_SCANCODES[24]
#define KEY_Z KEY_SCANCODES[25]

JVR_NAMESPACE_CLOSE_SCOPE

#endif //JVR_UTILS_SCANCODE_H

