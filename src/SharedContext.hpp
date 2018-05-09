#ifndef SHAREDCONTEXT_H
#define SHAREDCONTEXT_H

struct Configuration;
class Backend;
class ScriptingEngine;

struct SharedContext {
  static SharedContext* instance;
  Configuration* config;
  Backend* backend;
  ScriptingEngine* scripting;
};

#endif // !SHAREDCONTEXT_H

