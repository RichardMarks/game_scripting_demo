#ifndef PYTHONSCRIPTINGENGINE_H
#define PYTHONSCRIPTINGENGINE_H

#include <Python.h>

#include "ScriptingEngine.hpp"

class PythonScriptingEngine : public ScriptingEngine {
  public:
    PythonScriptingEngine(std::string const& programName);
    virtual ~PythonScriptingEngine();

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
    wchar_t* program;
    PyObject* scriptNameObject;
    PyObject* scriptModuleObject;
};

#endif // !PYTHONSCRIPTINGENGINE_H

