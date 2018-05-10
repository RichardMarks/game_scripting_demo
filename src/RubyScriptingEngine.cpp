#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "RubyScriptingEngine.hpp"
#include "Backend.hpp"
#include "ScriptingEngine.hpp"
#include "SharedContext.hpp"

class GlobalFunction {
  public:
    static VALUE call(std::string const& name) {
      GlobalFunction::argc = 0;
      GlobalFunction::lastId = rb_intern(name.c_str());
      GlobalFunction::lastName = name;

      int error = 0;

      VALUE result = rb_protect((VALUE (*)(VALUE))GlobalFunction::wrapper, (VALUE)0, &error);

      if (error) {
        VALUE exception = rb_errinfo();
        rb_set_errinfo(Qnil);
        if (RTEST(exception)) {
          rb_warn("Ruby Script Error: %" PRIsVALUE "", rb_funcall(exception, rb_intern("full_message"), 0));
        }
        throw std::runtime_error("Script Error");
      }

      return result;
    }

    static VALUE callx1(std::string const& name, VALUE x1) {
      GlobalFunction::argc = 1;
      GlobalFunction::lastId = rb_intern(name.c_str());
      GlobalFunction::lastName = name;

      int error = 0;

      VALUE params[1];
      params[0] = x1;

      VALUE result = rb_protect((VALUE (*)(VALUE))GlobalFunction::wrapper, (VALUE)params, &error);

      if (error) {
        VALUE exception = rb_errinfo();
        rb_set_errinfo(Qnil);
        if (RTEST(exception)) {
          rb_warn("Ruby Script Error: %" PRIsVALUE "", rb_funcall(exception, rb_intern("full_message"), 0));
        }
        throw std::runtime_error("Script Error");
      }

      return result;
    }

  private:
    static VALUE wrapper(VALUE params) {
      if (GlobalFunction::argc) {
        VALUE* values = (VALUE*)(params);
        VALUE x1 = values[0];

        return rb_funcall(0, GlobalFunction::lastId, 1, x1);
      }

      return rb_funcall(0, GlobalFunction::lastId, 0, 0);
    }

    static std::string lastName;
    static int lastId;
    static int argc;
};

int GlobalFunction::lastId = 0;
int GlobalFunction::argc = 0;
std::string GlobalFunction::lastName = "";

void parseConfigurationTable(VALUE cfgHash, Configuration& config);

namespace engine {
  VALUE apiInit(VALUE self, VALUE cfgHash);
  VALUE apiGetScreenWidth(VALUE self);
  VALUE apiGetScreenHeight(VALUE self);
  VALUE apiDrawCircle(VALUE self, VALUE xPos, VALUE yPos, VALUE radius);
}

RubyScriptingEngine::RubyScriptingEngine() {
  RUBY_INIT_STACK;

  if (ruby_setup()) {
    std::stringstream msg;
    msg << "Unable to create Ruby VM" << std::endl;
    throw std::runtime_error(msg.str());
  }

  engineModule = rb_define_module("Engine");
  rb_define_module_function(engineModule, "init", RUBY_METHOD_FUNC(engine::apiInit), 1);
  rb_define_module_function(engineModule, "getScreenWidth", RUBY_METHOD_FUNC(engine::apiGetScreenWidth), 0);
  rb_define_module_function(engineModule, "getScreenHeight", RUBY_METHOD_FUNC(engine::apiGetScreenHeight), 0);
  rb_define_module_function(engineModule, "drawCircle", RUBY_METHOD_FUNC(engine::apiDrawCircle), 3);
}

RubyScriptingEngine::~RubyScriptingEngine() {
  ruby_cleanup(0);
}

void RubyScriptingEngine::load(std::string const& filename) {
  ruby_script(filename.c_str());

  VALUE script = rb_str_new_cstr(filename.c_str());
  int state = 0;
  rb_load_protect(script, 0, &state);
  if (state) {
    std::stringstream msg;
    msg << "Unable to load " << filename << std::endl;
    VALUE exception = rb_errinfo();
    rb_set_errinfo(Qnil);
    if (RTEST(exception)) {
      rb_warn("Ruby Script Error: %" PRIsVALUE "", rb_funcall(exception, rb_intern("full_message"), 0));
    }
    throw std::runtime_error(msg.str());
  }
}

void RubyScriptingEngine::init(Configuration& config) {
  SharedContext::instance->config->copy(config);
}

int RubyScriptingEngine::getScreenWidth() {
  int width = 0;
  SharedContext::instance->backend->getWindowSize(&width, 0);
  return width;
}

int RubyScriptingEngine::getScreenHeight() {
  int height = 0;
  SharedContext::instance->backend->getWindowSize(0, &height);
  return height;
}

void RubyScriptingEngine::drawCircle(int x, int y, int radius) {
  SharedContext::instance->backend->drawCircle(x, y, radius);
}

void RubyScriptingEngine::runCreate() {
  SharedContext& context = *SharedContext::instance;

  GlobalFunction::call(context.config->userCreateFunctionName);
}

void RubyScriptingEngine::runDestroy() {
  SharedContext& context = *SharedContext::instance;

  GlobalFunction::call(context.config->userDestroyFunctionName);
}

void RubyScriptingEngine::runUpdate(float deltaTime) {
  SharedContext& context = *SharedContext::instance;

  GlobalFunction::callx1(context.config->userUpdateFunctionName, DBL2NUM(deltaTime));
}

void RubyScriptingEngine::runRender() {
  SharedContext& context = *SharedContext::instance;

  GlobalFunction::call(context.config->userRenderFunctionName);
}

void parseConfigurationTable(VALUE cfgHash, Configuration& config) {
  auto hasKey = [&](std::string const& symbolName) {
    if (rb_funcall(cfgHash, rb_intern("has_key?"), 1, ID2SYM(rb_intern(symbolName.c_str()))) == Qtrue) {
      return true;
    }
    return false;
  };

  auto readField = [&](std::string const& symbolName) {
    return rb_funcall(cfgHash, rb_intern("[]"), 1, ID2SYM(rb_intern(symbolName.c_str())));
  };

  auto getInt = [&](int* dst, std::string const& name) {
    if (hasKey(name)) {
      VALUE result = readField(name);
      *dst = static_cast<int>(NUM2INT(result));
    }
  };

  auto getBoolean = [&](bool* dst, std::string const& name) {
    if (hasKey(name)) {
      VALUE result = readField(name);
      if (RB_TYPE_P(result, T_TRUE)) {
        *dst = true;
      } else if (RB_TYPE_P(result, T_FALSE)) {
        *dst = false;
      }
    }
  };

  auto getString = [&](std::string& dst, std::string const& name) {
    if (hasKey(name)) {
      VALUE result = readField(name);
      if (RB_TYPE_P(result, T_STRING)) {
        dst.assign(std::string(StringValueCStr(result)));
      }
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
}

namespace engine {
  VALUE apiInit(VALUE self, VALUE cfgHash) {
    Configuration config;
    parseConfigurationTable(cfgHash, config);
    SharedContext::instance->scripting->init(config);
    return Qnil;
  }

  VALUE apiGetScreenWidth(VALUE self) {
    VALUE width = INT2NUM(SharedContext::instance->scripting->getScreenWidth());
    return width;
  }

  VALUE apiGetScreenHeight(VALUE self) {
    VALUE height = INT2NUM(SharedContext::instance->scripting->getScreenHeight());
    return height;
  }

  VALUE apiDrawCircle(VALUE self, VALUE xPos, VALUE yPos, VALUE radius) {
    int x = NUM2INT(xPos);
    int y = NUM2INT(yPos);
    int r = NUM2INT(radius);
    SharedContext::instance->scripting->drawCircle(x, y, r);
    return Qnil;
  }
}
