#include "application.h"
#include "../widgets/viewport.h"
#include "../widgets/menu.h"
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>

AMN_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE
const char* AmnApplication::APPLICATION_NAME = "Amnesie";

// constructor
//----------------------------------------------------------------------------
AmnApplication::AmnApplication(unsigned width, unsigned height):
  _mainWindow(NULL), _context(NULL)
{
  EMBREE_CTXT = new AmnUsdEmbreeContext();
  _width = width;
  _height = height;
  _mainWindow = CreateStandardWindow(width, height);
};

AmnApplication::AmnApplication(bool fullscreen):
  _mainWindow(NULL), _context(NULL)
{
  EMBREE_CTXT = new AmnUsdEmbreeContext();
  _mainWindow = CreateFullScreenWindow();
  glfwGetWindowSize(_mainWindow->GetGlfwWindow(), &_width, &_height);
};

// create full screen window
//----------------------------------------------------------------------------
AmnWindow*
AmnApplication::CreateFullScreenWindow()
{
  return AmnWindow::CreateFullScreenWindow();
}

// create standard window
//----------------------------------------------------------------------------
AmnWindow*
AmnApplication::CreateStandardWindow(int width, int height)
{
  return AmnWindow::CreateStandardWindow(width, height);
}

// create simple graph
//----------------------------------------------------------------------------
pxr::GraphNode* CreateGraphNode()
{
  pxr::GraphNode* node = new pxr::GraphNode();
  return node;
}

pxr::GraphGraph* CreateGraphNodeGraph()
{
  pxr::GraphGraph* graph = new pxr::GraphGraph();

  return graph;
}

// add node to graph test
//----------------------------------------------------------------------------
pxr::GraphNode _TestAddNode(pxr::UsdStageRefPtr stage, const std::string& path)
{
  pxr::GraphNode node = GraphNode::Define(stage, pxr::SdfPath(path));
  return node;
}

// create input test
//----------------------------------------------------------------------------
pxr::GraphInput _TestAddInput(GraphNode& node, 
                              const std::string& name, 
                              const GraphAttributeType& type,
                              const pxr::SdfValueTypeName& valueType)
{
  if(type == GraphAttributeType::Parameter)
  {
    pxr::GraphInput input = node.CreateInput(pxr::TfToken(name), valueType);
    return input;
  }
  return pxr::GraphInput();
}

// create output test
//----------------------------------------------------------------------------
pxr::GraphOutput _TestAddOutput(GraphNode& node, 
                                const std::string& name, 
                                const GraphAttributeType& type,
                                const pxr::SdfValueTypeName& valueType)
{
  if(type == GraphAttributeType::Parameter)
  {
    pxr::GraphOutput output = node.CreateOutput(pxr::TfToken(name), valueType);
    return output;
  }
  return pxr::GraphOutput();
}

// init application
//----------------------------------------------------------------------------
void 
AmnApplication::Init()
{
  _mainWindow->SetContext();
  AmnView* mainView = _mainWindow->GetMainView();
  _mainWindow->SplitView(mainView, 10, true);

  AmnMenuUI* menu = new AmnMenuUI(mainView->GetLeft());
  AmnViewportUI* viewport = new AmnViewportUI(mainView->GetRight(), EMBREE);

  /*pxr::UsdStageRefPtr stage =
    UsdStage::CreateNew("/Users/benmalartre/Documents/RnD/amnesie/usd/test.usda");*/
  pxr::UsdStageRefPtr stage = 
    UsdStage::CreateInMemory();
  pxr::GraphGraph graph = pxr::GraphGraph::Define(stage, pxr::SdfPath("/MyNodeGraph"));

  pxr::GraphNode node1 = _TestAddNode(stage, "/MyNodeGraph/node1");
  pxr::GraphInput input1 = _TestAddInput(node1, "input1", GraphAttributeType::Parameter, pxr::SdfValueTypeNames->Float);
  pxr::GraphOutput output1 = _TestAddOutput(node1, "output1", GraphAttributeType::Parameter, pxr::SdfValueTypeNames->Float);

  pxr::GraphNode node2 = _TestAddNode(stage, "/MyNodeGraph/node2");
  pxr::GraphInput input2 = _TestAddInput(node2, "input2", GraphAttributeType::Parameter, pxr::SdfValueTypeNames->Float);
  pxr::GraphOutput output2 = _TestAddOutput(node2, "output2", GraphAttributeType::Parameter, pxr::SdfValueTypeNames->Float);

  input2.ConnectToSource(output1);

  pxr::UsdUINodeGraphNodeAPI nodeUI1(node1);
  pxr::UsdUINodeGraphNodeAPI nodeUI2(node2);
  pxr::UsdAttribute posAttr1  = nodeUI1.CreatePosAttr();
  posAttr1.Set(pxr::GfVec2f(120,60));
  pxr::UsdAttribute posAttr2 = nodeUI2.CreatePosAttr();
  posAttr2.Set(pxr::GfVec2f(240,60));


  stage->Export("/Users/benmalartre/Documents/RnD/amnesie/usd/test2.usda");
  //stage->Save(); 
  
  /*
  _mainWindow->SplitView(mainView->GetRight(), 75, true);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft(), 25, false);
  _mainWindow->SplitView(mainView->GetRight()->GetLeft()->GetRight(), 75, false);
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/kitchen_set/kitchen_set.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/Kitchen_set/assets/Clock/Clock.usd";
  //std::string usdFile = "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/UsdSkelExamples/HumanFemale/HumanFemale.usd";
  */
  EMBREE_CTXT->Resize(1024, 720);
  EMBREE_CTXT->SetFilePath("/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd");
  EMBREE_CTXT->InitDevice();
  EMBREE_CTXT->TraverseStage();
  EMBREE_CTXT->CommitDevice();
  
  embree::FileName outputImageFilename("/Users/benmalartre/Documents/RnD/embree/embree-usd/images/img.011.jpg");
  
  //RenderToFile(outputImageFilename);
  RenderToMemory();
  viewport->SetPixels(EMBREE_CTXT->_width, EMBREE_CTXT->_height, EMBREE_CTXT->_pixels);
  
  //_mainWindow->CollectLeaves();
  //_mainWindow->DummyFill();

 
}

// main loop
void 
AmnApplication::MainLoop()
{
  _mainWindow->MainLoop();
  
}

AMN_NAMESPACE_CLOSE_SCOPE

