#include "../exec/menu.h"
#include "../utils/strings.h"
#include "../ui/menu.h"
#include "../tests/push.h"
#include "../tests/raycast.h"
#include "../tests/bvh.h"
#include "../tests/pbd.h"
#include "../tests/hair.h"
#include "../tests/particles.h"
#include "../app/time.h"
#include "../app/index.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE


void AddExecMenu(MenuUI* menu)
{
  MenuUI::Item* testItem = menu->Add("Exec", false, true, NULL);
  MenuUI::Item* subItem;
  subItem = testItem->Add("Push", false, true, std::bind(SetExecCallback, "push"));
  subItem = testItem->Add("Raycast", false, true, std::bind(SetExecCallback, "raycast"));
  subItem = testItem->Add("BVH", false, true, std::bind(SetExecCallback, "bvh"));
  subItem = testItem->Add("PBD", false, true, std::bind(SetExecCallback, "pbd"));
  subItem = testItem->Add("Hair", false, true, std::bind(SetExecCallback, "hair"));
  subItem = testItem->Add("Particles", false, true, std::bind(SetExecCallback, "particles"));
}

void SetExecCallback(const std::string &name)
{
  Index* index = Application::Get()->GetIndex();
  Time* time = Time::Get();
  time->SetActiveTime(time->GetStartTime());
  //_exec = new TestPendulum();
  //_exec = new TestVelocity();
  if(name == "push")index->SetExec(new TestPush());
  else if(name == "raycast")index->SetExec(new TestRaycast());
  else if(name == "bvh")index->SetExec(new TestBVH());
  else if(name == "pbd")index->SetExec(new TestPBD());
  else if(name == "hair")index->SetExec(new TestHair());
  else if(name == "particles")index->SetExec(new TestParticles());
    
  //_exec = new TestPoints();
  //_exec = new TestGrid();
  //_exec = new TestParticles();
  //_exec = new TestInstancer();
  //_exec = new TestRaycast();
  //_exec = new TestPBD();
  //_exec = new TestPush();
  //_exec = new TestHair();
  //_exec = new TestGeodesic();
  //_exec = new TestBVH();

}


JVR_NAMESPACE_CLOSE_SCOPE