#ifndef JVR_TEST_LOCATION_H
#define JVR_TEST_LOCATION_H

#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

class TestLocation : public Execution {
  void InitExec() override;
  void UpdateExec(double time) override;
  void TerminateExec() override;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_TEST_LOCATION_H