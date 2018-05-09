#ifndef SDLBACKEND_H
#define SDLBACKEND_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include <SDL2/SDL.h>

#include "Backend.hpp"

class SDLBackend : public Backend {
  public:
    SDLBackend()
      : window(nullptr),
        renderer(nullptr) {
    }

    virtual ~SDLBackend() {}

    // initialize any libraries
    virtual void init();

    // create the main game window
    virtual void createWindow(int width, int height, bool fullscreen, std::string const& title);

    // retrieves the size of the window
    virtual void getWindowSize(int* width, int* height);

    // returns a timestamp
    virtual float getTimestamp();

    // shutdown any libraries
    virtual void shutdown();

    // process any events - return false to stop the main game loop
    virtual bool processEvents();

    // perform any needed operations before the main game loop update
    virtual void preFrameUpdate(float deltaTime);

    // perform any needed operations after the main game loop update
    virtual void postFrameUpdate(float deltaTime);

    // perform any needed operations before the main game loop render
    virtual void preFrameRender();

    // perform any needed operations after the main game loop render
    virtual void postFrameRender();

    // draws a filled circle to the screen
    virtual void drawCircle(int x, int y, int radius);

  protected:
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Event sdlEvent;
};

#endif // !SDLBACKEND_H

