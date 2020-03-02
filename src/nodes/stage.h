#include "../default.h"
#include "../graph/node.h"

PXR_NAMESPACE_OPEN_SCOPE

class GraphNodeStage : public GraphNode {
  enum STAGE_LIFETIME{
    DISK,
    MEMORY
  };

  private:
    UsdStageRefPtr stage;
    std::string fileName;
};

PXR_NAMESPACE_CLOSE_SCOPE