#ifndef BACKEND_H
#define BACKEND_H

#include <string>

// a backend must be implemented for each desired backend library eg SDL, SFML, GLFW, etc...

class Backend {
  public:
    Backend() {}
    virtual ~Backend() {}

    // initialize any libraries
    virtual void init() = 0;

    // create the main game window
    virtual void createWindow(int width, int height, bool fullscreen, std::string const& title) = 0;

    // retrieves the size of the window
    virtual void getWindowSize(int* width, int* height) = 0;

    // returns a timestamp
    virtual float getTimestamp() = 0;

    // shutdown any libraries
    virtual void shutdown() = 0;

    // process any events - return false to stop the main game loop
    virtual bool processEvents() = 0;

    // perform any needed operations before the main game loop update
    virtual void preFrameUpdate(float deltaTime) = 0;

    // perform any needed operations after the main game loop update
    virtual void postFrameUpdate(float deltaTime) = 0;

    // perform any needed operations before the main game loop render
    virtual void preFrameRender() = 0;

    // perform any needed operations after the main game loop render
    virtual void postFrameRender() = 0;

    // draws a filled circle to the screen
    virtual void drawCircle(int x, int y, int radius) = 0;
};

#endif // !BACKEND_H

