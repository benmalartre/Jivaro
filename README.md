# Jivaro

This is where i experiment with usd and software devellopment

![UI preview of Jivaro](NGlfc5s1.gif)

Jivaro is develloped and tested on Windows (Windows 10 Pro) and on MacBookPro M1 (MacOs Monterey 12.3.1)

# How to build
1- clone and build USD
2- clone and build glfw
2- clone Jivaro repository
3- create a build folder inside Jivaro folder
4- from this build repo run:
    -cmake -DUSD_DIR=path_to_usd_build -DGLFW_DIR=path_to_glfw_build ../
    -cmake --build . --config Release
    -cmake --install . -prefix path_to_jivaro_build\install

5-
    -add path_to_jivaro_install\lib to your PATH
    -add path_to_jivaro_install\lib\usd to PXR_PLUGINPATH_NAME


