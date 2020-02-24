#include "window.h"


namespace AMN {
  // fullscreen window constructor
  //----------------------------------------------------------------------------
  Window::Window(bool fullscreen) :
  _pixels(nullptr), _debounce(0)
  {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS,mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS,mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS,mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE,mode->refreshRate);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    _window = glfwCreateWindow(mode->width, mode->height, "AMINA.0.0",  monitor, NULL);
    _width = mode->width;
    _height = mode->height;
    if(_window)
    {
      // window datas
      GetContextVersionInfos();
      glfwSetWindowUserPointer(_window, this);
      
      // set current opengl context
      glfwMakeContextCurrent(_window);
      Resize(_width,_height);

      // load opengl functions
      gl3wInit();

      // setup callbacks
      glfwSetMouseButtonCallback(_window, ClickCallback);
      //glfwSetScrollCallback(_window, ImGui_ImplGlfw_ScrollCallback);
      glfwSetScrollCallback(_window, ScrollCallback);
      glfwSetKeyCallback(_window, KeyboardCallback);
      //glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
      glfwSetCharCallback(_window, CharCallback);
      glfwSetCursorPosCallback(_window, MotionCallback);
      glfwSetWindowSizeCallback(_window, ReshapeCallback);

      // imgui
      SetupImgui();
    }
  }

  // width/height window constructor
  //----------------------------------------------------------------------------
  Window::Window(int width, int height):
  _pixels(nullptr), _debounce(0)
  {
    _width = width;
    _height = height;
  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    
    _window = glfwCreateWindow(_width,_height,"AMINA.0.0",NULL,NULL);
  
    if(_window)
    {
      // window datas
      GetContextVersionInfos();
      glfwSetWindowUserPointer(_window, this);
      
      // set current opengl context
      glfwMakeContextCurrent(_window);
      Resize(_width,_height);

      // load opengl functions
      gl3wInit();

      // setup callbacks
      glfwSetMouseButtonCallback(_window, ClickCallback);
      //glfwSetScrollCallback(_window, ImGui_ImplGlfw_ScrollCallback);
      glfwSetScrollCallback(_window, ScrollCallback);
      glfwSetKeyCallback(_window, KeyboardCallback);
      //glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
      glfwSetCharCallback(_window, CharCallback);
      glfwSetCursorPosCallback(_window, MotionCallback);
      glfwSetWindowSizeCallback(_window, ReshapeCallback);

      // imgui
      SetupImgui();
    }
  }

  // window destructor
  //----------------------------------------------------------------------------
  Window::~Window()
  {
    _views.clear();
    if(_pixels)embree::alignedFree(_pixels);
    if(_window)glfwDestroyWindow(_window);
  }

  // create full screen window
  //----------------------------------------------------------------------------
  AMN_EXPORT Window* 
  Window::CreateFullScreenWindow()
  {
    return new Window(true);
  }

  // create standard window
  //----------------------------------------------------------------------------
  AMN_EXPORT Window*
  Window::CreateStandardWindow(int width, int height)
  {
    return new Window(width, height);
  }

  // Resize
  //----------------------------------------------------------------------------
  void 
  Window::Resize(unsigned width, unsigned height)
  {
    if (width == _width && height == _height && _pixels)
      return;

    if (_pixels) embree::alignedFree(_pixels);
    _width = width;
    _height = height;
    _pixels = 
      (unsigned*) embree::alignedMalloc(_width*_height*sizeof(unsigned),64);
  
  }

  // get context version infos
  //----------------------------------------------------------------------------
  void 
  Window::GetContextVersionInfos()
  {
    // get GLFW, OpenGL version And GLEW Version:
      _iOpenGLMajor = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MAJOR);
      _iOpenGLMinor = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MINOR);
      _iOpenGLRevision = glfwGetWindowAttrib(_window, GLFW_CONTEXT_REVISION);
  
      // print them
      std::cout << "Status: Using GLFW Version "    
        << glfwGetVersionString() << std::endl;

      std::cout << "Status: Using OpenGL Version: " << _iOpenGLMajor  << "," 
        << _iOpenGLMinor << ", Revision : " << _iOpenGLRevision << std::endl;
  }

  // get parent Window stored in user data
  //----------------------------------------------------------------------------
  Window* 
  Window::GetUserData(GLFWwindow* window)
  {
    return static_cast<Window*>(glfwGetWindowUserPointer(window));
  }

  // setup imgui
  //----------------------------------------------------------------------------
  void 
  Window::SetupImgui()
  {
    // detup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    _io = &(ImGui::GetIO());
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // setup imgui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(_window, glfwGetCurrentContext());
    ImGui_ImplOpenGL3_Init("#version 330");
  }

  // clear imgui
  //----------------------------------------------------------------------------
  void 
  Window::ClearImgui()
  {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void
  Window::TestImgui(int index)
  {
    // Start the imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    bool dummy = true;
    float color[3] = {1.0f, 0.5f, 0.5f};
    if (index == 0)
      ImGui::ShowDemoWindow(&dummy);

    else if(index == 1)
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &dummy);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &dummy);

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&color); // Edit 3 floats representing a color

      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }
      
    else
    {
      ImGui::Begin("Another Window", &dummy);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        dummy = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
    glClearColor(color[0], color[1], color[2], 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(_window);

  }

  void Window::MainLoop()
  {
    // Enable the OpenGL context for the current window
    _guiId = 1;
    TestImgui(0);
    while(!glfwWindowShouldClose(_window))
    {
      glfwPollEvents();
      //AMN::DisplayCallback(_window);
      // If you press escape the window will close
      if (glfwGetKey(_window, GLFW_KEY_ESCAPE))
      {
        glfwSetWindowShouldClose(_window, true);
      }
      else if(glfwGetKey(_window, GLFW_KEY_SPACE))
      {
       if(!_debounce){
         _guiId++;
         _debounce = true;
       }
       
      }
      TestImgui(_guiId % 3);
      
    }
  }

  // keyboard event callback
  //----------------------------------------------------------------------------
  void 
  KeyboardCallback(
    GLFWwindow* window, 
    int key, 
    int scancode, 
    int action, 
    int mods
  )
  {
    Window* parent = (Window*)glfwGetWindowUserPointer(window);

    //ImGui_ImplGlfw_KeyCallback(window_in,key,scancode,action,mods);
    //if (ImGui::GetIO().WantCaptureKeyboard) return;
    if(action == GLFW_RELEASE)
    {
      parent->SetDebounce(false);
    }
    //if (action == GLFW_PRESS)
    //{
    //  /* call tutorial keyboard handler */
    //  //device_key_pressed(key);
//
    //  if (mods & GLFW_MOD_CONTROL)
    //  {
    //    switch (key) {
    //    case GLFW_KEY_UP        :  break;
    //    case GLFW_KEY_DOWN      :  break;
    //    case GLFW_KEY_LEFT      :  break;
    //    case GLFW_KEY_RIGHT     :  break;
    //    }
    //  }
    //  else
    //  {
    //    switch (key) 
    //    {
    //    /*
    //    case GLFW_KEY_LEFT      : app->camera.rotate(-0.02f,0.0f); break;
    //    case GLFW_KEY_RIGHT     : app->camera.rotate(+0.02f,0.0f); break;
    //    case GLFW_KEY_UP        : app->camera.move(0.0f,0.0f,+speed); break;
    //    case GLFW_KEY_DOWN      : app->camera.move(0.0f,0.0f,-speed); break;
    //    case GLFW_KEY_PAGE_UP   : app->speed *= 1.2f; break;
    //    case GLFW_KEY_PAGE_DOWN : app->speed /= 1.2f; break;
//
    //    case GLFW_KEY_W : ; break;
    //    case GLFW_KEY_S : moveDelta.z = -1.0f; break;
    //    case GLFW_KEY_A : moveDelta.x = -1.0f; break;
    //    case GLFW_KEY_D : moveDelta.x = +1.0f; break;
    //    */
    //    case GLFW_KEY_F :
    //      std::cout << "KEY F" << std::endl;
    //      glfwDestroyWindow(window);
    //      if (parent->IsFullScreen()) {
    //        parent = parent->CreateStandardWindow(
    //          parent->GetWidth(),parent->GetHeight());
    //      }
    //      else {
    //        parent = parent->CreateFullScreenWindow();
    //      }
    //      window = parent->GetWindow();
    //      glfwMakeContextCurrent(window);
    //      glfwSetWindowUserPointer(window, parent);
    //      parent->SetFullScreen(!parent->IsFullScreen());
    //      //app->_fullscreen = !app->_fullscreen;
    //      break;
    //      
    //    /*  
    //    case GLFW_KEY_C : std::cout << camera.str() << std::endl; break;
    //    case GLFW_KEY_HOME: g_debug=clamp(g_debug+0.01f); PRINT(g_debug); break;
    //    case GLFW_KEY_END : g_debug=clamp(g_debug-0.01f); PRINT(g_debug); break;
    //      
    //    case GLFW_KEY_SPACE: {
    //      Ref<Image> image = new Image4uc(width, height, (Col4uc*)pixels, true, "", true);
    //      storeImage(image, "screenshot.tga");
    //      break;
    //    }
    //      
    //    case GLFW_KEY_ESCAPE:
    //    */
    //    case GLFW_KEY_Q: 
    //    std::cout << "KEY Q" << std::endl;
    //      glfwSetWindowShouldClose(window,1);
    //      break;
    //    }
    //  }
    //}
    //else if (action == GLFW_RELEASE)
    //{
    //  std::cout << "KEY RELEASE" << std::endl;
    //  parent->SetDebounce(false);
    //  
    //  switch (key)
    //  {
    //  case GLFW_KEY_W : moveDelta.z = 0.0f; break;
    //  case GLFW_KEY_S : moveDelta.z = 0.0f; break;
    //  case GLFW_KEY_A : moveDelta.x = 0.0f; break;
    //  case GLFW_KEY_D : moveDelta.x = 0.0f; break;
    //  }
    //  
    //}
  }
  
  void 
  ClickCallback(GLFWwindow* window, int button, int action, int mods)
  { 
    Window* parent = Window::GetUserData(window);
    std::cout << "CLICK CLICK" << std::endl;
    
    // feed inputs to dear imgui, start new frame
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    //ImGui::NewFrame();

    // render your GUI
    //ImGui::Begin("Demo window");
    //ImGui::Button("Hello!");
    //ImGui::End();
    

    double x,y;
    glfwGetCursorPos(window,&x,&y);

    /*
    if (action == GLFW_RELEASE)
    {
      mouseMode = 0;
    }
    else if (action == GLFW_PRESS)
    {
      if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        ISPCCamera ispccamera = camera.getISPCCamera(width,height);
        Vec3fa p; bool hit = device_pick(float(x),float(y),ispccamera,p);

        if (hit) {
          Vec3fa delta = p - camera.to;
          Vec3fa right = normalize(ispccamera.xfm.l.vx);
          Vec3fa up    = normalize(ispccamera.xfm.l.vy);
          camera.to = p;
          camera.from += dot(delta,right)*right + dot(delta,up)*up;
        }
      }
      else
      {
        clickX = x; clickY = y;
        if      (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_SHIFT) mouseMode = 1;
        else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL ) mouseMode = 3;
        else if (button == GLFW_MOUSE_BUTTON_LEFT) mouseMode = 4;
      }
    }
    */
  }

  void 
  ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
  {

  }

  void 
  CharCallback(GLFWwindow* window, unsigned c)
  {

  }

  void 
  MotionCallback(GLFWwindow* window, double x, double y)
  {
    /*
    //if (ImGui::GetIO().WantCaptureMouse) return;
  
    float dClickX = float(clickX - x), dClickY = float(clickY - y);
    clickX = x; clickY = y;

    switch (mouseMode) {
    case 1: camera.rotateOrbit(-0.005f*dClickX,0.005f*dClickY); break;
    case 2: break;
    case 3: camera.dolly(-dClickY); break;
    case 4: camera.rotate(-0.005f*dClickX,0.005f*dClickY); break;
    }
    */
  }

  void 
  DisplayCallback(GLFWwindow* window)
  {
    //std::cout << "DISPLAY !!!" << std::endl;
    /*
    // update camera 
    camera.move(moveDelta.x*speed, moveDelta.y*speed, moveDelta.z*speed);
    ISPCCamera ispccamera = camera.getISPCCamera(width,height,true);
     if (print_camera)
      std::cout << camera.str() << std::endl;

    // render image using ISPC
    initRayStats();
    double t0 = getSeconds();
    render(pixels,width,height,float(time0-t0),ispccamera);
    double dt0 = getSeconds()-t0;
    avg_render_time.add(dt0);
    double mrayps = double(getNumRays())/(1000000.0*dt0);
    avg_mrayps.add(mrayps);

    // draw pixels to screen 
    glDrawPixels(width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);

    ImGui_ImplGlfwGL2_NewFrame();
    
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    //window_flags |= ImGuiWindowFlags_NoScrollbar;
    //window_flags |= ImGuiWindowFlags_MenuBar;
    //window_flags |= ImGuiWindowFlags_NoMove;
    //window_flags |= ImGuiWindowFlags_NoResize;
    //window_flags |= ImGuiWindowFlags_NoCollapse;
    //window_flags |= ImGuiWindowFlags_NoNav;
    
    //ImGui::GetStyle().WindowBorderSize = 0.0f;
    //ImGui::SetNextWindowPos(ImVec2(width-200,0));
    //ImGui::SetNextWindowSize(ImVec2(200,height));
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::Begin("Embree", nullptr, window_flags);
    drawGUI();
    ImGui::Text("%3.2f fps",1.0f/avg_render_time.get());
#if defined(RAY_STATS)
    ImGui::Text("%3.2f Mray/s",avg_mrayps.get());
#endif
    ImGui::End();
     
    //ImGui::ShowDemoWindow();
        
    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
    */
    glClearColor(
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      1.f
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);

#ifdef __APPLE__
    // work around glfw issue #1334
    // https://github.com/glfw/glfw/issues/1334
    static bool macMoved = false;

    if (!macMoved) {
      int x, y;
      glfwGetWindowPos(window, &x, &y);
      glfwSetWindowPos(window, ++x, y);
      macMoved = true;
    }
#endif
    /*
    double dt1 = embree::getSeconds()-t0;
    avg_frame_time.add(dt1);

    if (print_frame_rate)
    {
      std::ostringstream stream;
      stream.setf(std::ios::fixed, std::ios::floatfield);
      stream.precision(2);
      stream << "render: ";
      stream << 1.0f/dt0 << " fps, ";
      stream << dt0*1000.0f << " ms, ";
#if defined(RAY_STATS)
      stream << mrayps << " Mray/s, ";
#endif
      stream << "display: ";
      stream << 1.0f/dt1 << " fps, ";
      stream << dt1*1000.0f << " ms, ";
      stream << width << "x" << height << " pixels";
      std::cout << stream.str() << std::endl;
    } 
    */
  }

  void 
  ReshapeCallback(GLFWwindow* window, int, int)
  {
    /*
    Window* parent = (Window*)glfwGetWindowUserPointer(window);
    int width,height;
    glfwGetFramebufferSize(window, &width, &height);
    parent->Resize(width,height);
    glViewport(0, 0, width, height);
    parent->SetWidth(width); 
    parent->SetHeight(height);
    */
  }

} // namespace AMN