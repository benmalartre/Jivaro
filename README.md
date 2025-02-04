# Jivaro

This is where I experiment with OpenUsd and software development.
The basic idea is to add an execution system inside a Hydra 2.0 Scene Index.

![UI preview of Jivaro 0](images/bTC9Mry0.gif)

![UI preview of Jivaro 1](images/NGlfc5s1.gif)

![UI preview of Jivaro 2](images/1C3Tsa1W.gif)

![UI preview of Jivaro 3](images/09c9751i.gif)

![UI preview of Jivaro 4](images/frEI1ERm.gif)

![UI preview of Jivaro 5](images/Qp39YGzu.gif)

![UI preview of Jivaro 6](images/EUQJVR12.gif)


Jivaro is develloped and tested on Windows (Windows 10 Pro) and on MacBookPro M1 (MacOs Monterey 12.3.1)

# How to build
1. clone and build USD
2. clone and build glfw
3. clone Jivaro repository
4. create a build folder inside Jivaro folder
5. from this build repo run
    - cmake -DUSD_DIR=path_to_usd_build -DGLFW_DIR=path_to_glfw_build -DPYTHON_BUILD=ON -DCMAKE_INSTALL_PREFIX=path_to_jivaro_install ../
    - cmake --build . --config Release
    - cmake --install .
6. update your system PATHS
    - add path_to_jivaro_install\lib to your PATH
    - add path_to_jivaro_install\lib\usd to PXR_PLUGINPATH_NAME


# MacOS 
on MacOS, before launching the app (and waiting for better solution) you have to tell where to find runtime libraries

1. export DYLD_LIBRARY_PATH=path_to_jivaro_install/lib:path_to_usd_build/lib               
2. export PXR_PLUGINPATH_NAME=path_to_jivaro_install/lib/usd


# Open sources projects
https://github.com/PixarAnimationStudios/OpenUSD

https://github.com/glfw/glfw

https://github.com/ocornut/imgui

# References (codes and papers)
https://github.com/ahmidou

https://github.com/cpichard

https://github.com/janbender

https://github.com/LumaPictures

https://github.com/matiascodesal

https://github.com/matthias-research

https://github.com/mmacklin

https://github.com/raph080

https://github.com/yuki-koyama

