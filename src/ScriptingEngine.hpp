#ifndef SCRIPTINGENGINE_H
#define SCRIPTINGENGINE_H

#include "Configuration.hpp"

// each supported scripting language needs to implement the scripting engine interface
class ScriptingEngine {
  public:
    ScriptingEngine() {}
    virtual ~ScriptingEngine() {}

    virtual void load(std::string const& filename) = 0;

    virtual void init(Configuration& config) = 0;
    virtual int getScreenWidth() = 0;
    virtual int getScreenHeight() = 0;
    virtual void drawCircle(int x, int y, int radius) = 0;
    virtual void runCreate() = 0;
    virtual void runDestroy() = 0;
    virtual void runUpdate(float deltaTime) = 0;
    virtual void runRender() = 0;
};

#endif // !SCRIPTINGENGINE_H

