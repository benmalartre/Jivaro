#include "../utils/options.h"

JVR_NAMESPACE_OPEN_SCOPE

CommandLineOptions::CommandLineOptions(int argc, char *const *argv) {
    for (int i = 1; i < argc; ++i) {
        _stages.push_back(argv[i]);
    }
}

JVR_NAMESPACE_CLOSE_SCOPE