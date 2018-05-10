#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

struct Configuration {
  int screenWidth;
  int screenHeight;
  bool useFullscreen;
  bool debugMode;
  std::string windowTitle;
  std::string userCreateFunctionName;
  std::string userDestroyFunctionName;
  std::string userUpdateFunctionName;
  std::string userRenderFunctionName;

  Configuration();
  void copy(Configuration& other);
  void print();
};

#endif // !CONFIGURATION_H
