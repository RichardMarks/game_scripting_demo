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

#include "lua/lua.hpp"

struct Variant {
  enum Type { STRING, NUMBER, BOOLEAN, NIL };

  Variant::Type valueType;

  union VarValue {
    const char* stringValue;
    double numberValue;
    bool booleanValue;
  };

  VarValue value;

  Variant() {
    valueType = Variant::Type::NIL;
  }
};

void readField(lua_State* L, std::string const& fieldName, Variant& result) {
  // stack: [.., table]
  lua_getfield(L, -1, fieldName.c_str());
  // stack: [.., table, field]
  if (lua_isnil(L, -1)) {
    // std::cerr << "readField(" << fieldName << ") is nil." << std::endl;
    lua_pop(L, 1);
    // stack: [.., table]
    result.valueType = Variant::Type::NIL;
  } else {
    switch(lua_type(L, -1)) {
      case LUA_TSTRING: {
        result.valueType = Variant::Type::STRING;
        result.value.stringValue = std::string(lua_tostring(L, -1)).c_str();
        // std::cerr << "readField(" << fieldName << ") is string: " << result.value.stringValue << std::endl;
        lua_pop(L, 1);
        // stack: [.., table]
      } break;

      case LUA_TBOOLEAN: {
        result.valueType = Variant::Type::BOOLEAN;
        result.value.booleanValue = static_cast<bool>(lua_toboolean(L, -1));
        // std::cerr << "readField(" << fieldName << ") is bool: " << result.value.booleanValue << std::endl;
        lua_pop(L, 1);
        // stack: [.., table]
      } break;

      case LUA_TNUMBER: {
        result.valueType = Variant::Type::NUMBER;
        result.value.numberValue = static_cast<double>(lua_tonumber(L, -1));
        // std::cerr << "readField(" << fieldName << ") is number: " << result.value.numberValue << std::endl;
        lua_pop(L, 1);
        // stack: [.., table]
      } break;

      default:
        std::stringstream msg;
        msg << "Unable to parse configuration table. Field " << fieldName << " is not an expected type." << std::endl;
        throw std::runtime_error(msg.str());
        break;
    }
  }
}

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

  Configuration() {
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

  void copy(Configuration& other) {
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

  void parseTable(lua_State* L) {
    if (!lua_istable(L, -1)) {
      std::stringstream msg;
      msg << "Unable to parse configuration table. There is no table on the top of the stack." << std::endl;
      throw std::runtime_error(msg.str());
    }
    // stack: [.., config]

    auto getInt = [&](int* dst, std::string const& name) {
      Variant result;
      readField(L, name, result);
      if (result.valueType == Variant::Type::NUMBER) {
        *dst = static_cast<int>(result.value.numberValue);
      }
    };

    auto getBoolean = [&](bool* dst, std::string const& name) {
      Variant result;
      readField(L, name, result);
      if (result.valueType == Variant::Type::BOOLEAN) {
        *dst = result.value.booleanValue;
      }
    };

    auto getString = [&](std::string& dst, std::string const& name) {
      Variant result;
      readField(L, name, result);
      if (result.valueType == Variant::Type::STRING) {
        dst.assign(std::string(result.value.stringValue));
      }
    };

    // printf("\nBEFORE PARSING LUA CONFIG\n");
    // print();

    getInt(&screenWidth, "SCREEN_WIDTH");
    getInt(&screenHeight, "SCREEN_HEIGHT");
    getBoolean(&useFullscreen, "USE_FULLSCREEN");
    getBoolean(&debugMode, "DEBUG");
    getString(windowTitle, "WINDOW_TITLE");
    getString(userCreateFunctionName, "create");
    getString(userDestroyFunctionName, "destroy");
    getString(userUpdateFunctionName, "update");
    getString(userRenderFunctionName, "render");

    // printf("\nAFTER PARSING LUA CONFIG\n");
    // print();

    lua_pop(L, 1);
    // stack: [..]
  }

  void print() {
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
};

struct SharedContext {
  static SharedContext* instance;
  Configuration* config;
  Backend* backend;
  lua_State* script;
};

SharedContext* SharedContext::instance = nullptr;

// Engine C API
namespace engine {
  // C side of the API
  void init(Configuration& config);
  int getScreenWidth();
  int getScreenHeight();
  void drawCircle(int x, int y, int radius);

  // Lua side of the API
  int apiInit(lua_State* L);
  int apiGetScreenWidth(lua_State* L);
  int apiGetScreenHeight(lua_State* L);
  int apiDrawCircle(lua_State* L);
}

class Game {
  public:
    Game();
    ~Game();
    void create();
    void destroy();
    void update(float deltaTime);
    void render();

    SharedContext context;
    bool isRunning;
};

Game::Game() {
  // initialize the shared context
  Configuration config;

  context.config = &config;
  #ifdef USE_SDL_BACKEND
  context.backend = new SDLBackend();
  #endif
  context.script = luaL_newstate();

  SharedContext::instance = &context;
  Backend& backend = *context.backend;

  // provide standard libraries to script
  luaL_openlibs(context.script);

  // tell lua about our engine capabilities
  luaL_Reg api[] = {
    { "init", engine::apiInit },
    { "getScreenWidth", engine::apiGetScreenWidth },
    { "getScreenHeight", engine::apiGetScreenHeight },
    { "drawCircle", engine::apiDrawCircle },
    { nullptr, nullptr }
  };
  luaL_newlib(context.script, api);
  lua_setglobal(context.script, "engine");

  // load the game script
  if (luaL_loadfile(context.script, "game.lua")) {
    lua_close(context.script);
    context.script = nullptr;
    std::stringstream msg;
    msg << "Unable to load game.lua" << std::endl;
    throw std::runtime_error(msg.str());
  }

  // run the game script
  if (lua_pcall(context.script, 0, 0, 0)) {
    std::stringstream msg;
    msg << "Error in game.lua: " << std::endl << std::string(lua_tostring(context.script, -1)) << std::endl;
    lua_close(context.script);
    context.script = nullptr;
    throw std::runtime_error(msg.str());
  }

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

  if (context.script != nullptr) {
    lua_close(context.script);
    context.script = nullptr;
  }

  if (context.config->debugMode) {
    std::cout << "Game::~Game()" << std::endl;
  }
}

void Game::create() {
  if (context.config->debugMode) {
    std::cout << "Game::create()" << std::endl;
  }

  lua_getglobal(context.script, context.config->userCreateFunctionName.c_str());
  // stack: [.., function?]
  if (lua_isfunction(context.script, -1)) {
    lua_pcall(context.script, 0, 0, 0);
    // stack: [..]
  } else { lua_pop(context.script, 1); }
}

void Game::destroy() {
  if (context.config->debugMode) {
    std::cout << "Game::destroy()" << std::endl;
  }

  lua_getglobal(context.script, context.config->userDestroyFunctionName.c_str());
  // stack: [.., function?]
  if (lua_isfunction(context.script, -1)) {
    lua_pcall(context.script, 0, 0, 0);
    // stack: [..]
  } else { lua_pop(context.script, 1); }
}

void Game::update(float deltaTime) {
  if (context.config->debugMode) {
    std::cout << "Game::update(" << deltaTime << ")" << std::endl;
  }

  lua_getglobal(context.script, context.config->userUpdateFunctionName.c_str());
  // stack: [.., function?]
  if (lua_isfunction(context.script, -1)) {
    lua_pushnumber(context.script, deltaTime);
    // stack: [.., function, deltaTime]
    lua_pcall(context.script, 1, 0, 0);
    // stack: [..]
  } else { lua_pop(context.script, 1); }
}

void Game::render() {
  if (context.config->debugMode) {
    std::cout << "Game::render(" << std::endl;
  }

  // std::cout << "Game::render()" << std::endl;
  lua_getglobal(context.script, context.config->userRenderFunctionName.c_str());
  // stack: [.., function?]
  if (lua_isfunction(context.script, -1)) {
    lua_pcall(context.script, 0, 0, 0);
    // stack: [..]
  } else { lua_pop(context.script, 1); }
}

int main(int argc, char* argv[]) {
  try {
    Game game;
  } catch(const std::exception& ex) {
    std::cerr << "Runtime Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// Engine C API implementation
namespace engine {
  void init(Configuration& config) {
    SharedContext::instance->config->copy(config);
  }

  int getScreenWidth() {
    int width = 0;
    SharedContext::instance->backend->getWindowSize(&width, 0);
    return width;
  }

  int getScreenHeight() {
    int height = 0;
    SharedContext::instance->backend->getWindowSize(0, &height);
    return height;
  }

  void drawCircle(int x, int y, int radius) {
    SharedContext::instance->backend->drawCircle(x, y, radius);
  }

  int apiInit(lua_State* L) {
    // expected to have been called with a "configuration" table on the top of the stack
    if (lua_gettop(L) != 0) {
      if (lua_istable(L, -1)) {
        Configuration config;
        config.parseTable(L);
        init(config);
      }
    }

    return 0;
  }

  int apiGetScreenWidth(lua_State* L) {
    // pushes the screen width on the stack
    lua_pushnumber(L, getScreenWidth());
    return 1;
  }

  int apiGetScreenHeight(lua_State* L) {
    // pushes the screen height on the stack
    lua_pushnumber(L, getScreenHeight());
    return 1;
  }

  int apiDrawCircle(lua_State* L) {
    // expected to have been called with x, y, radius parameters
    int x = 0;
    int y = 0;
    int radius = 0;

    if (lua_gettop(L) >= 3) {
      if (lua_isnumber(L, -1)) {
        radius = lua_tonumber(L, -1);
        lua_pop(L, 1);
      } else {
        // error
      }

      if (lua_isnumber(L, -1)) {
        y = lua_tonumber(L, -1);
        lua_pop(L, 1);
      } else {
        // error
      }

      if (lua_isnumber(L, -1)) {
        x = lua_tonumber(L, -1);
        lua_pop(L, 1);
      } else {
        // error
      }
    } else {
      // error
    }

    drawCircle(x, y, radius);

    return 0;
  }
}
