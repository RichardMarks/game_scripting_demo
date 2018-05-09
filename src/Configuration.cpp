#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "Configuration.hpp"

Configuration::Configuration() {
  screenWidth = 640;
  screenHeight = 480;
  useFullscreen = false;
  debugMode = true;
  windowTitle = "Lua Game Scripting Engine v1.0";
  userCreateFunctionName = "create";
  userDestroyFunctionName = "destroy";
  userUpdateFunctionName = "update";
  userRenderFunctionName = "render";
}

void Configuration::copy(Configuration& other) {
  screenWidth = other.screenWidth;
  screenHeight = other.screenHeight;
  useFullscreen = other.useFullscreen;
  debugMode = other.debugMode;
  windowTitle = other.windowTitle;
  userCreateFunctionName = other.userCreateFunctionName;
  userDestroyFunctionName = other.userDestroyFunctionName;
  userUpdateFunctionName = other.userUpdateFunctionName;
  userRenderFunctionName = other.userRenderFunctionName;
}

void Configuration::print() {
  std::cout << "Configuration:" << std::endl
    << "SCREEN_WIDTH: " << screenWidth << std::endl
    << "SCREEN_HEIGHT: " << screenHeight << std::endl
    << "USE_FULLSCREEN: " << (useFullscreen ? "True" : "False") << std::endl
    << "DEBUG: " << (debugMode ? "True" : "False") << std::endl
    << "WINDOW_TITLE: " << windowTitle << std::endl
    << "create: " << userCreateFunctionName << std::endl
    << "destroy: " << userDestroyFunctionName << std::endl
    << "update: " << userUpdateFunctionName << std::endl
    << "render: " << userRenderFunctionName << std::endl;
}
