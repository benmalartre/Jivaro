#include "window.h"
#include "view.h"
#include "splitter.h"
#include "imgui/imgui_custom.h"
#include "imgui/imgui_test.h"
//#include "glutils.h"

namespace AMN {

  // fullscreen window constructor
  //----------------------------------------------------------------------------
  Window::Window(bool fullscreen) :
  _pixels(nullptr), _debounce(0),_mainView(NULL), _activeView(NULL), 
  _pickImage(0),_cursor(NULL),_splitter(NULL)
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
    
    Init();
  }

  // width/height window constructor
  //----------------------------------------------------------------------------
  Window::Window(int width, int height):
  _pixels(nullptr), _debounce(0),_mainView(NULL), _activeView(NULL), 
  _pickImage(0),_cursor(NULL), _splitter(NULL)
  {
    _width = width;
    _height = height;
  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    
    _window = glfwCreateWindow(_width,_height,"AMINA.0.0",NULL,NULL);
    Init();
  }

  // initialize
  //----------------------------------------------------------------------------
  void 
  Window::Init()
  {
    if(_window)
    {
      // create main splittable view
      _mainView = new View(NULL, pxr::GfVec2i(0,0), pxr::GfVec2i(_width, _height));
      SplitView(_mainView, 10, true);
      SplitView(_mainView->GetRight(), 75, true);
      SplitView(_mainView->GetRight()->GetLeft(), 25, false);
      SplitView(_mainView->GetRight()->GetLeft()->GetRight(), 75, false);

      _splitter = new Splitter();
      _splitter->BuildMap(_mainView);

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
      glfwSetCursorPosCallback(_window, MouseMoveCallback);
      glfwSetWindowSizeCallback(_window, ResizeCallback);

      // imgui
      SetupImgui();

      // screen space quad
      ScreenSpaceQuad();
    }
  }

  // window destructor
  //----------------------------------------------------------------------------
  Window::~Window()
  {
    if(_splitter)delete _splitter;
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

    _mainView->Resize(0, 0, _width, _height);
  }

  // split view
  //----------------------------------------------------------------------------
  void 
  Window::SplitView(View* view, unsigned perc, bool horizontal )
  {
    if(!view->IsLeaf())
    {
      std::cerr << "Can't split non-leaf view! Sorry!!!" << std::endl;
      return;
    }
    view->SetLeaf();
    view->SetPerc(perc);
    if(horizontal)
    {
      view->SetHorizontal();
      view->Split();
    }
    else
    {
      view->ClearHorizontal();
      view->Split();
    }
  }

  // build split map
  //----------------------------------------------------------------------------
  void 
  Window::RebuildSplittersMap()
  {
    _splitter->BuildMap(_mainView);
  }

  /*
  // add splitter
  //----------------------------------------------------------------------------
  void
  Window::AddSplitter(int x, int y, int w, int h, int perc)
  {
    Splitter splitter(this, x, y, w, h, perc);
    _splitters.push_back(splitter);
  }

  // get splitter ptr
  //----------------------------------------------------------------------------
  Splitter* Window::GetSplitterPtr(int index)
  {
    if(index>=0 && index <_splitters.size()) return &_splitters[index];
    else return NULL;
  }
  */

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

  // draw
  //----------------------------------------------------------------------------
  void 
  Window::Draw()
  {
    // start the imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    
    ImGui::NewFrame();
    ImGui::SetWindowSize(pxr::GfVec2i(GetWidth(), GetHeight()));
    ImGui::SetWindowPos(pxr::GfVec2i(0,0));

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = pxr::GfVec2i(0,0);
    style.FramePadding = pxr::GfVec2i(0,0);

    if(_mainView)_mainView->Draw();
    

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  }

  // draw pick image
  //----------------------------------------------------------------------------
  void 
  Window::DrawPickImage()
  {
    if(!_pickImage)
    {
      glDeleteTextures(1, &_pickImage);

      //glDeleteTextures(1 &_pickImage);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glUseProgram(SCREENSPACEQUAD_PROGRAM_SHADER);

      glGenTextures(1, &_pickImage);
      glBindTexture(GL_TEXTURE_2D, _pickImage);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                      GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                      GL_NEAREST);
    }

    glUseProgram(SCREENSPACEQUAD_PROGRAM_SHADER);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,_pickImage);
    glTexImage2D(	GL_TEXTURE_2D,
                    0,
                    GL_RGBA,
                    _splitter->GetWidth(),
                    _splitter->GetHeight(),
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    _splitter->GetPixels());

    glUniform1i(glGetUniformLocation(SCREENSPACEQUAD_PROGRAM_SHADER,"tex"),0);
    //glClear(GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  
    DrawScreenSpaceQuad();
  }

  void 
  Window::ScreenSpaceQuad()
  {
    SetupScreenSpaceQuadShader();
    SetupScreenSpaceQuad();
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
    ImGui::StyleColorsAmina();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.f;      
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;

    ImGui::SetNextWindowBgAlpha(1.f);

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
    {
      bool opened;
      View* view = GetMainView();
      int flags = 0;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoMove;

      ImGui::Begin("FUCK", &opened, flags);
      ImGui::SetWindowSize(pxr::GfVec2i(GetWidth(), GetHeight()));
      ImGui::SetWindowPos(pxr::GfVec2i(0,0));
      /*ImGui::TestDummyView(&opened, pxr::GfVec2i(0,0), 
        pxr::GfVec2i(GetWidth(), GetHeight()), view->GetColor4());*/
      ImGui::End();
    }
      //ImGui::ShowDemoWindow(&dummy);

    else if(index == 1)
    {
      static float f = 0.0f;
      static int counter = 0;
      bool opened;
      ImGuiWindowFlags flags = 0;
      flags |= ImGuiWindowFlags_NoResize;
      //flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoMove;

      
      ImGui::Begin("Hello, FUCKIN BITCH!", &opened, flags);                          // Create a window called "Hello, world!" and append into it.
      ImGui::SetWindowSize(pxr::GfVec2i(GetWidth(), GetHeight()));
      ImGui::SetWindowPos(pxr::GfVec2i(0,0));
      /*
      int N = 2;
      const char* labels[2] = {"Label1", "Label2"};
      const char* tooltips[2] = {"ToolTip1", "ToolTip2"};


      int optionalHoveredIndex;

      int selectedIndex = 0;
      bool B = ImGui::TabLabels(2, &labels[0], selectedIndex, &tooltips[0], true, &optionalHoveredIndex);

    
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
      */

      /*
      for(int i=0;i<GetNumSplitters(); ++i)
      {
        std::string name = "Je M'appelle "+std::to_string(i);
        ImGui::Begin(name.c_str(), &opened, flags);
        Splitter* splitter = GetSplitterPtr(i);
        ImGui::TestGrapNodes(&opened, splitter->GetMin(), splitter->GetMin() + 
          splitter->GetMax() * splitter->GetPerc() * 0.01);
        ImGui::SetWindowSize(splitter->GetMax() - splitter->GetMin());
        ImGui::SetWindowPos(splitter->GetMin());
        ImGui::End();

      }
      */
      
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
    //glClearColor(color[0], color[1], color[2], 1.f);
    //glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  }

  void Window::MakeTextureFromPixels(void)
  {
    /*
    int i, j, c;
    for (i = 0; i < GetHeight().; ++i) 
    {
      for (j = 0; j < GetWidth(); ++j) 
      {
        c = ((((i&0x8)==0)^((j&0x8))==0))*255;
        checkImage[i][j][0] = (GLubyte) c;
        checkImage[i][j][1] = (GLubyte) c;
        checkImage[i][j][2] = (GLubyte) c;
        checkImage[i][j][3] = (GLubyte) 255;
      }
    }
    */
  }

  bool Window::UpdateActiveTool(int mouseX, int mouseY)
  {
    if(_activeTool == DRAG)
    {
      if(_activeView)
      {
        _activeView->GetPercFromMousePosition(mouseX, mouseY);
        _splitter->BuildMap(_mainView);
        _splitter->Resize(_mainView);
        this->DrawPickImage();
      }
    }
  }

  void Window::MainLoop()
  {
    // Enable the OpenGL context for the current window
    _guiId = 1;
    TestImgui(0);
    while(!glfwWindowShouldClose(_window))
    {
      glfwPollEvents();
      glClearColor(1.f,1.f, 1.f,1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // If you press escape the window will close
      if (glfwGetKey(_window, GLFW_KEY_ESCAPE))
      {
        glfwSetWindowShouldClose(_window, true);
      }
      else if(glfwGetKey(_window, GLFW_KEY_SPACE))
      {
        if(!_debounce)
        {
          _guiId++;
          _debounce = true;
        }
      }
      
      _splitter->Draw();
      //TestImgui(_guiId % 3);
      glfwSwapBuffers(_window);
    }
  }

  // pick splitter
  //----------------------------------------------------------------------------
  bool 
  Window::PickSplitter(double mouseX, double mouseY)
  {
    _clickX = mouseX;
    _clickY = mouseY;
    int pixelValue = _splitter->GetPixelValue(mouseX, mouseY);
    if(pixelValue)
    {
      if(_cursor) glfwDestroyCursor(_cursor);
      _activeView = _splitter->GetViewByIndex(pixelValue - 1);
      if (_activeView->IsHorizontal())
        _cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
      else 
        _cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
      glfwSetCursor(_window, _cursor);
      return true;
    }
    else
    {
      if(_cursor) glfwDestroyCursor(_cursor);
      _cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR );
      glfwSetCursor(_window, _cursor);
      return false;
    }
    glfwSwapBuffers(_window);
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

    double x,y;
    glfwGetCursorPos(window,&x,&y);

    if (action == GLFW_RELEASE)
    {
      parent->SetActiveTool(NONE);
    }
    else if (action == GLFW_PRESS)
    {
      if (button == GLFW_MOUSE_BUTTON_RIGHT)
      {
        std::cerr << "MOUSE BUTTON RIGHT PRESSED ! " << std::endl;
      }
      else
      {
        if(parent->PickSplitter(x, y))
        {
          parent->SetActiveTool(DRAG);
        }
      }
        /*
        if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_SHIFT)
           _mouseMode = 1;
        else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL ) 
          _mouseMode = 3;
        else if (button == GLFW_MOUSE_BUTTON_LEFT) 
          _mouseMode = 4;
        */
    }
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
  MouseMoveCallback(GLFWwindow* window, double x, double y)
  {
    //if (ImGui::GetIO().WantCaptureMouse) return;
  
    Window* parent = Window::GetUserData(window);
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if(parent->GetActiveTool() != NONE)
    {
      parent->UpdateActiveTool(x, y);
    }

  }

  void 
  DisplayCallback(GLFWwindow* window)
  {
    Window* parent = (Window*)glfwGetWindowUserPointer(window);
   
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
   /*
    glClearColor(
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      1.f
    );
    */

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
  ResizeCallback(GLFWwindow* window, int width, int height)
  {
/*
    Window* parent = (Window*)glfwGetWindowUserPointer(window);
    //int width,height;
    glfwGetFramebufferSize(window, &width, &height);

    parent->Resize(width,height);
    //parent->DrawPickImage(true);
    //parent->RebuildSplittersMap();
    glViewport(0, 0, width, height);
   */ 
  }

} // namespace AMN