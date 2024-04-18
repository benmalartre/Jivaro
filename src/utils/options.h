#ifndef JVR_UTILS_OPTIONS_H
#define JVR_UTILS_OPTIONS_H

#include <vector>
#include <string>

JVR_NAMESPACE_OPEN_SCOPE

class CommandLineOptions {
  public:
    CommandLineOptions(int argc, char *const *argv);

    const std::vector<std::string> &stages() { return _stages; }

  private:
    std::vector<std::string> _stages;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_OPTIONS_H