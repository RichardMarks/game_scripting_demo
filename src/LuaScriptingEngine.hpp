#ifndef LUASCRIPTINGENGINE_H
#define LUASCRIPTINGENGINE_H

#include "ScriptingEngine.hpp"
#include "lua/lua.hpp"

class LuaScriptingEngine : public ScriptingEngine {
  public:
    LuaScriptingEngine();
    virtual ~LuaScriptingEngine();

    virtual void load(std::string const& filename);
    virtual void init(Configuration& config);
    virtual int getScreenWidth();
    virtual int getScreenHeight();
    virtual void drawCircle(int x, int y, int radius);
    virtual void runCreate();
    virtual void runDestroy();
    virtual void runUpdate(float deltaTime);
    virtual void runRender();

    lua_State* L;
};

#endif // !LUASCRIPTINGENGINE_H

