#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "LuaScriptingEngine.hpp"
#include "Backend.hpp"
#include "ScriptingEngine.hpp"
#include "SharedContext.hpp"


void parseConfigurationTable(lua_State* L, Configuration& config);

// Engine C API
namespace engine {
  // Lua side of the API
  int apiInit(lua_State* L);
  int apiGetScreenWidth(lua_State* L);
  int apiGetScreenHeight(lua_State* L);
  int apiDrawCircle(lua_State* L);
}

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

LuaScriptingEngine::LuaScriptingEngine()
  : L(nullptr) {
  L = luaL_newstate();

  // provide standard libraries to script
  luaL_openlibs(L);

  // tell lua about our engine capabilities
  luaL_Reg api[] = {
    { "init", engine::apiInit },
    { "getScreenWidth", engine::apiGetScreenWidth },
    { "getScreenHeight", engine::apiGetScreenHeight },
    { "drawCircle", engine::apiDrawCircle },
    { nullptr, nullptr }
  };

  luaL_newlib(L, api);
  lua_setglobal(L, "engine");
}

LuaScriptingEngine::~LuaScriptingEngine() {
  if (L != nullptr) {
    lua_close(L);
    L = nullptr;
  }
}

void LuaScriptingEngine::load(std::string const& filename) {
  // load the game script
  if (luaL_loadfile(L, filename.c_str())) {
    lua_close(L);
    L = nullptr;
    std::stringstream msg;
    msg << "Unable to load " << filename << std::endl;
    throw std::runtime_error(msg.str());
  }

  // run the game script
  if (lua_pcall(L, 0, 0, 0)) {
    std::stringstream msg;
    msg << "Error in " << filename << ": " << std::endl << std::string(lua_tostring(L, -1)) << std::endl;
    lua_close(L);
    L = nullptr;
    throw std::runtime_error(msg.str());
  }
}

void LuaScriptingEngine::init(Configuration& config) {
  SharedContext::instance->config->copy(config);
}

int LuaScriptingEngine::getScreenWidth() {
  int width = 0;
  SharedContext::instance->backend->getWindowSize(&width, 0);
  return width;
}

int LuaScriptingEngine::getScreenHeight() {
  int height = 0;
  SharedContext::instance->backend->getWindowSize(0, &height);
  return height;
}

void LuaScriptingEngine::drawCircle(int x, int y, int radius) {
  SharedContext::instance->backend->drawCircle(x, y, radius);
}

void LuaScriptingEngine::runCreate() {
  SharedContext& context = *SharedContext::instance;

  lua_getglobal(L, context.config->userCreateFunctionName.c_str());
  // stack: [.., function?]

  if (lua_isfunction(L, -1)) {
    lua_pcall(L, 0, 0, 0);
    // stack: [..]
  } else {
    lua_pop(L, 1);
    // stack: [..]
  }
}

void LuaScriptingEngine::runDestroy() {
  SharedContext& context = *SharedContext::instance;

  lua_getglobal(L, context.config->userDestroyFunctionName.c_str());
  // stack: [.., function?]

  if (lua_isfunction(L, -1)) {
    lua_pcall(L, 0, 0, 0);
    // stack: [..]
  } else {
    lua_pop(L, 1);
    // stack: [..]
  }
}

void LuaScriptingEngine::runUpdate(float deltaTime) {
  SharedContext& context = *SharedContext::instance;

  lua_getglobal(L, context.config->userUpdateFunctionName.c_str());
  // stack: [.., function?]

  if (lua_isfunction(L, -1)) {
    lua_pushnumber(L, deltaTime);
    // stack: [.., function, deltaTime]

    lua_pcall(L, 1, 0, 0);
    // stack: [..]
  } else {
    lua_pop(L, 1);
    // stack: [..]
  }
}

void LuaScriptingEngine::runRender() {
  SharedContext& context = *SharedContext::instance;

  lua_getglobal(L, context.config->userRenderFunctionName.c_str());
  // stack: [.., function?]

  if (lua_isfunction(L, -1)) {
    lua_pcall(L, 0, 0, 0);
    // stack: [..]
  } else {
    lua_pop(L, 1);
    // stack: [..]
  }
}

void parseConfigurationTable(lua_State* L, Configuration& config) {
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

  getInt(&config.screenWidth, "SCREEN_WIDTH");
  getInt(&config.screenHeight, "SCREEN_HEIGHT");
  getBoolean(&config.useFullscreen, "USE_FULLSCREEN");
  getBoolean(&config.debugMode, "DEBUG");
  getString(config.windowTitle, "WINDOW_TITLE");
  getString(config.userCreateFunctionName, "create");
  getString(config.userDestroyFunctionName, "destroy");
  getString(config.userUpdateFunctionName, "update");
  getString(config.userRenderFunctionName, "render");

  lua_pop(L, 1);
  // stack: [..]
}


// Engine C API implementation
namespace engine {
  int apiInit(lua_State* L) {
    // expected to have been called with a "configuration" table on the top of the stack
    if (lua_gettop(L) != 0) {
      if (lua_istable(L, -1)) {
        Configuration config;
        parseConfigurationTable(L, config);
        SharedContext::instance->scripting->init(config);
      }
    }

    return 0;
  }

  int apiGetScreenWidth(lua_State* L) {
    // pushes the screen width on the stack
    lua_pushnumber(L, SharedContext::instance->scripting->getScreenWidth());
    return 1;
  }

  int apiGetScreenHeight(lua_State* L) {
    // pushes the screen height on the stack
    lua_pushnumber(L, SharedContext::instance->scripting->getScreenHeight());
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

    SharedContext::instance->scripting->drawCircle(x, y, radius);

    return 0;
  }
}
