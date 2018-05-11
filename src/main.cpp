#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#ifdef USE_SDL_BACKEND
#include "SDLBackend.hpp"
#endif

#include "SharedContext.hpp"
#include "Configuration.hpp"

#include "LuaScriptingEngine.hpp"
#include "RubyScriptingEngine.hpp"
#include "PythonScriptingEngine.hpp"

class Game {
  public:
    Game(std::string const& programName, std::string const& mainScriptFile);
    ~Game();
    void create();
    void destroy();
    void update(float deltaTime);
    void render();

    Configuration config;
    SharedContext context;
    bool isRunning;
};

Game::Game(std::string const& programName, std::string const& mainScriptFile) {
  // initialize the shared context
  SharedContext::instance = &context;
  context.config = &config;
  #ifdef USE_SDL_BACKEND
  context.backend = new SDLBackend();
  #endif

  std::string scriptExtention = mainScriptFile.substr(mainScriptFile.rfind('.') + 1);

  if (scriptExtention == "lua") {
    context.scripting = new LuaScriptingEngine();
  } else if (scriptExtention == "rb") {
    context.scripting = new RubyScriptingEngine();
  } else if (scriptExtention == "py") {
    context.scripting = new PythonScriptingEngine(programName);
  } else {
    std::stringstream msg;
    msg << "Unsupported script [" << scriptExtention << "] : " << mainScriptFile << std::endl;
    throw std::runtime_error(msg.str());
  }

  context.scripting->load(mainScriptFile);

  Backend& backend = *context.backend;

  if (config.debugMode) {
    std::cout << "Game::Game()" << std::endl;
    config.print();
  }

  backend.init();
  backend.createWindow(
    config.screenWidth,
    config.screenHeight,
    config.useFullscreen,
    config.windowTitle
  );

  create();
  isRunning = true;
  float lastTime = backend.getTimestamp();
  float newTime = 0;
  float deltaTime = 0.0f;
  while (isRunning) {
    newTime = backend.getTimestamp();
    if (newTime - lastTime < 1) {
      deltaTime = (newTime - lastTime);
      backend.preFrameUpdate(deltaTime);
      update(deltaTime);
      backend.postFrameUpdate(deltaTime);
    }
    lastTime = newTime;
    backend.preFrameRender();
    render();
    backend.postFrameRender();
    if (!backend.processEvents()) {
      isRunning = false;
    }
  }
}

Game::~Game() {
  destroy();

  if (context.backend != nullptr) {
    context.backend->shutdown();
    delete context.backend;
    context.backend = nullptr;
  }

  if (context.scripting != nullptr) {
    delete context.scripting;
    context.scripting = nullptr;
  }

  if (context.config->debugMode) {
    std::cout << "Game::~Game()" << std::endl;
  }
}

void Game::create() {
  if (context.config->debugMode) {
    std::cout << "Game::create()" << std::endl;
  }

  context.scripting->runCreate();
}

void Game::destroy() {
  if (context.config->debugMode) {
    std::cout << "Game::destroy()" << std::endl;
  }

  context.scripting->runDestroy();
}

void Game::update(float deltaTime) {
  if (context.config->debugMode) {
    std::cout << "Game::update(" << deltaTime << ")" << std::endl;
  }

  context.scripting->runUpdate(deltaTime);
}

void Game::render() {
  if (context.config->debugMode) {
    std::cout << "Game::render(" << std::endl;
  }

  context.scripting->runRender();
}

int main(int argc, char* argv[]) {
  std::string mainScriptFile = "game.lua";

  if (argc > 1) {
    mainScriptFile.assign(argv[1]);
  }

  try {
    Game game(std::string(argv[0]), mainScriptFile);
  } catch(const std::exception& ex) {
    std::cerr << "Runtime Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
