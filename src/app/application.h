#pragma once

#include "default.h"
#include "window.h"
#include "view.h"
#include "camera.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../widgets/graph.h"
#include "../embree/device.h"
#include "../embree/context.h"
#include "../embree/prim.h"
#include "../embree/mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

class TimelineUI;

class Application
{
public:
  static const char* APPLICATION_NAME;
  // constructor
  Application(unsigned width, unsigned height);
  Application(bool fullscreen=true);

  // destructor
  ~Application();

    // create a fullscreen window
  static Window* CreateFullScreenWindow();

  // create a standard window of specified size
  static Window* CreateStandardWindow(int width, int height);
  
  // init application
  void Init();
  void Term();

  // update application
  void Update();

  // the main loop
  void MainLoop();

  // cleanup
  void CleanUp();

  // time
  inline float GetMinTime(){return _minTime;};
  inline float GetStartTime(){return _startTime;};
  inline float GetEndTime(){return _endTime;};
  inline float GetMaxTime(){return _maxTime;};
  inline float GetCurrentTime(){return _currentTime;};
  inline float GetFPS(){return _fps;};
  inline float GetSpeed(){return _speed;};
  inline bool GetLoop(){return _loop;};

  inline void SetMinTime(float time){_minTime = time;};
  inline void SetStartTime(float time){_startTime = time;};
  inline void SetEndTime(float time){_endTime = time;};
  inline void SetMaxTime(float time){_maxTime = time;};
  inline void SetCurrentTime(float time){_currentTime = time;};
  inline void SetFPS(float fps){_fps = fps;};
  inline void SetSpeed(float speed){_speed = speed;};
  inline void SetLoop(bool loop){_loop = loop;};

  void PreviousFrame();
  void NextFrame();
  void FirstFrame();
  void LastFrame();
  void StartPlayBack(bool backward=false);
  void StopPlayBack();
  void PlayBack();
  bool IsPlaying(){return _playback;};

private:
  std::string                       _fileName;
  Window*                           _mainWindow;
  std::vector<Window*>              _childWindow;
  UsdEmbreeContext*                 _context;
  std::vector<pxr::UsdStageRefPtr>  _stages;
  GraphUI*                          _test;
  
  // time
  pxr::TfStopwatch                  _stopWatch;
  float                             _currentTime;
  float                             _startTime;
  float                             _endTime;
  float                             _minTime;
  float                             _maxTime;
  float                             _fps;
  float                             _speed;
  bool                              _loop;
  bool                              _playForwardOrBackward;
  bool                              _playback;
  TimelineUI*                       _timeline;
};

AMN_NAMESPACE_CLOSE_SCOPE // namespace pxr
