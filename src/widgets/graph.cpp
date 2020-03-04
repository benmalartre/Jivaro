#include "graph.h"
#include "../app/view.h"

AMN_NAMESPACE_OPEN_SCOPE

AmnGraphUI::AmnGraphUI(AmnView* parent, const std::string& filename):
AmnUI(parent, "Graph")
{
  _filename = filename;
}

AmnGraphUI::~AmnGraphUI(){}

void AmnGraphUI::Event()
{
  std::cerr << "AmnGraphUI EVENT!" << std::endl;
};

void AmnGraphUI::Draw()
{
  bool opened;
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;
  

  ImGui::Begin(_name.c_str(), &opened, flags);
  pxr::GfVec4f color(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1, 
    1.f
  );

  //ImGui::TestDummyView(&opened, _parent->GetMin(), _parent->GetMax(), color);
  ImGui::TestGraphNodes(&opened, _parent->GetMin(), _parent->GetMax());
  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::End();
};

void AmnGraphUI::Init(const std::vector<pxr::UsdStageRefPtr>& stages)
{
  if(_stages.size())
    for(auto stage: _stages) delete stage;

  _stages.resize(stages.size());
  for(int i=0;i<stages.size();++i)
  {
    _stages[i] = new AmnGraphStageUI(stages[i]);
  }
  std::cout << "####### NUM STAGES : " << _stages.size() << std::endl;
};

AmnGraphStageUI* AmnGraphUI::GetStage(int index)
{
  if(index>=0 && index < _stages.size()) return _stages[index];
  else return NULL;
}
  
void AmnGraphUI::FillBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
}

AMN_NAMESPACE_CLOSE_SCOPE