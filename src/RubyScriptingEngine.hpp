#ifndef RUBYSCRIPTINGENGINE_H
#define RUBYSCRIPTINGENGINE_H

#include <ruby.h>

#include "ScriptingEngine.hpp"

class RubyScriptingEngine : public ScriptingEngine {
  public:
    RubyScriptingEngine();
    virtual ~RubyScriptingEngine();

    virtual void load(std::string const& filename);
    virtual void init(Configuration& config);
    virtual int getScreenWidth();
    virtual int getScreenHeight();
    virtual void drawCircle(int x, int y, int radius);
    virtual void runCreate();
    virtual void runDestroy();
    virtual void runUpdate(float deltaTime);
    virtual void runRender();

  protected:
    VALUE engineModule;
};

#endif // !RUBYSCRIPTINGENGINE_H

