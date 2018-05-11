#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "PythonScriptingEngine.hpp"
#include "Backend.hpp"
#include "ScriptingEngine.hpp"
#include "SharedContext.hpp"

void parseConfigurationTable(PyObject* params, Configuration& config);

namespace engine {
  PyObject* apiInit(PyObject* self, PyObject* params);
  PyObject* apiGetScreenWidth(PyObject* self, PyObject* params);
  PyObject* apiGetScreenHeight(PyObject* self, PyObject* params);
  PyObject* apiDrawCircle(PyObject* self, PyObject* params);
}

static PyMethodDef apiFunctions[] = {
  { "init", engine::apiInit, METH_VARARGS, "initialize the engine" },
  { "getScreenWidth", engine::apiGetScreenWidth, METH_VARARGS, "get the width of the screen" },
  { "getScreenHeight", engine::apiGetScreenHeight, METH_VARARGS, "get the height of the screen" },
  { "drawCircle", engine::apiDrawCircle, METH_VARARGS, "draw a filled circle given center x and y and radius" },
  { 0, 0, 0, 0 }
};

static PyModuleDef engineModule = {
  PyModuleDef_HEAD_INIT, "engine", 0, -1, apiFunctions, 0, 0, 0, 0
};

static PyObject* initializeEngineModule(void) {
  return PyModule_Create(&engineModule);
}

PythonScriptingEngine::PythonScriptingEngine(std::string const& programName)
  : program(nullptr),
    scriptNameObject(nullptr),
    scriptModuleObject(nullptr) {
  program = Py_DecodeLocale(programName.c_str(), 0);

  if (!program) {
    std::stringstream msg;
    msg << "Unable to decode " << programName << std::endl;
    throw std::runtime_error(msg.str());
  }

  Py_SetProgramName(program);
  PyImport_AppendInittab("engine", &initializeEngineModule);
  Py_Initialize();

  PyObject* sysPath = PySys_GetObject("path");
  PyObject* curPath = PyUnicode_FromString(".");
  PyList_Append(sysPath, curPath);
  Py_XDECREF(curPath);
}

PythonScriptingEngine::~PythonScriptingEngine() {
  Py_XDECREF(scriptModuleObject);
  Py_XDECREF(scriptNameObject);

  if (Py_FinalizeEx() < 0) {
    ::exit(120);
  }

  if (program) {
    PyMem_RawFree(program);
    program = nullptr;
  }
}

void PythonScriptingEngine::load(std::string const& filename) {
  // script name must not have .py extension
  std::string scriptName = filename.substr(0, filename.rfind('.'));

  scriptNameObject = PyUnicode_DecodeFSDefault(scriptName.c_str());
  scriptModuleObject = PyImport_Import(scriptNameObject);

  if (!scriptModuleObject) {
    PyErr_Print();
    std::stringstream msg;
    msg << "Unable to load " << filename << std::endl;
    throw std::runtime_error(msg.str());
  }
}

void PythonScriptingEngine::init(Configuration& config) {
  SharedContext::instance->config->copy(config);
}

int PythonScriptingEngine::getScreenWidth() {
  int width = 0;
  SharedContext::instance->backend->getWindowSize(&width, 0);
  return width;
}

int PythonScriptingEngine::getScreenHeight() {
  int height = 0;
  SharedContext::instance->backend->getWindowSize(0, &height);
  return height;
}

void PythonScriptingEngine::drawCircle(int x, int y, int radius) {
  SharedContext::instance->backend->drawCircle(x, y, radius);
}

void PythonScriptingEngine::runCreate() {
  SharedContext& context = *SharedContext::instance;

  PyObject* func = PyObject_GetAttrString(scriptModuleObject, context.config->userCreateFunctionName.c_str());
  if (func && PyCallable_Check(func)) {
    if (!PyObject_CallObject(func, 0)) {
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
    }
  } else {
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
  }
  Py_XDECREF(func);
}

void PythonScriptingEngine::runDestroy() {
  SharedContext& context = *SharedContext::instance;

  PyObject* func = PyObject_GetAttrString(scriptModuleObject, context.config->userDestroyFunctionName.c_str());
  if (func && PyCallable_Check(func)) {
    if (!PyObject_CallObject(func, 0)) {
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
    }
  } else {
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
  }
  Py_XDECREF(func);
}

void PythonScriptingEngine::runUpdate(float deltaTime) {
  SharedContext& context = *SharedContext::instance;

  PyObject* func = PyObject_GetAttrString(scriptModuleObject, context.config->userUpdateFunctionName.c_str());
  if (func && PyCallable_Check(func)) {
    PyObject* args = Py_BuildValue("(f)", deltaTime);
    if (!PyObject_CallObject(func, args)) {
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
    }
    Py_XDECREF(args);
  } else {
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
  }
  Py_XDECREF(func);
}

void PythonScriptingEngine::runRender() {
  SharedContext& context = *SharedContext::instance;

  PyObject* func = PyObject_GetAttrString(scriptModuleObject, context.config->userRenderFunctionName.c_str());
  if (func && PyCallable_Check(func)) {
    if (!PyObject_CallObject(func, 0)) {
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
    }
  } else {
    if (PyErr_Occurred()) {
      PyErr_Print();
    }
  }
  Py_XDECREF(func);
}

void parseConfigurationTable(PyObject* params, Configuration& config) {
  if (!PyDict_Check(params)) {
    throw std::runtime_error("configuration table is not a dict");
  }
  auto hasKey = [&](std::string const& keyName) {
    PyObject* key = PyUnicode_FromString(keyName.c_str());
    if (PyDict_Contains(params, key)) {
      Py_XDECREF(key);
      return true;
    }
    Py_XDECREF(key);
    return false;
  };

  auto readField = [&](std::string const& keyName) {
    return PyDict_GetItemString(params, keyName.c_str());
  };

  auto getInt = [&](int* dst, std::string const& name) {
    if (hasKey(name)) {
      PyObject* result = readField(name);
      *dst = static_cast<int>(PyLong_AsLong(result));
      Py_XDECREF(result);
    }
  };

  auto getBoolean = [&](bool* dst, std::string const& name) {
    if (hasKey(name)) {
      PyObject* result = readField(name);
      if (result == Py_True) {
        *dst = true;
      } else {
        *dst = false;
      }
      Py_XDECREF(result);
    }
  };

  auto getString = [&](std::string& dst, std::string const& name) {
    if (hasKey(name)) {
      PyObject* result = readField(name);
      PyObject* ascii = PyUnicode_AsASCIIString(result);
      dst.assign(std::string(PyBytes_AsString(ascii)));
      Py_XDECREF(ascii);
      Py_XDECREF(result);
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
  PyObject* apiInit(PyObject* self, PyObject* params) {
    PyObject* cfgDict;
    if (!PyArg_ParseTuple(params, "O", &cfgDict)) {
      return 0;
    }
    Configuration config;
    parseConfigurationTable(cfgDict, config);
    SharedContext::instance->scripting->init(config);
    return self;
  }

  PyObject* apiGetScreenWidth(PyObject* self, PyObject* params) {
    return PyLong_FromLong(SharedContext::instance->scripting->getScreenWidth());
  }

  PyObject* apiGetScreenHeight(PyObject* self, PyObject* params) {
    return PyLong_FromLong(SharedContext::instance->scripting->getScreenHeight());
  }

  PyObject* apiDrawCircle(PyObject* self, PyObject* params) {
    int x, y, radius;
    if (!PyArg_ParseTuple(params, "iii", &x, &y, &radius)) {
      return 0;
    }
    SharedContext::instance->scripting->drawCircle(x, y, radius);
    return self;
  }
}
