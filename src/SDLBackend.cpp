#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include <SDL2/SDL.h>

#include "SDLBackend.hpp"

// initialize any libraries
void SDLBackend::init() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::stringstream msg;
    msg << "Unable to initialize SDL2: " << SDL_GetError() << std::endl;
    throw std::runtime_error(msg.str());
  }
}

// create the main game window
void SDLBackend::createWindow(int width, int height, bool fullscreen, std::string const& title) {
  window = SDL_CreateWindow(
    title.c_str(),
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    width,
    height,
    fullscreen ? SDL_WINDOW_FULLSCREEN : (SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)
  );

  if (!window) {
    std::stringstream msg;
    msg << "Unable to create the main window: " << SDL_GetError() << std::endl;
    throw std::runtime_error(msg.str());
  }

  renderer = SDL_CreateRenderer(
    window,
    -1,
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
  );

  if (!renderer) {
    std::stringstream msg;
    msg << "Unable to create the main renderer: " << SDL_GetError() << std::endl;
    throw std::runtime_error(msg.str());
  }

  SDL_RenderSetLogicalSize(renderer, width, height);
}

// retrieves the size of the window
void SDLBackend::getWindowSize(int* width, int* height) {
  SDL_GetWindowSize(window, width, height);
}

float SDLBackend::getTimestamp() {
  return static_cast<float>(SDL_GetTicks() * 0.001f);
}

// shutdown any libraries
void SDLBackend::shutdown() {
  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }

  if (window != nullptr) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  SDL_Quit();
}

// process any events - return false to stop the main game loop
bool SDLBackend::processEvents() {
  while (SDL_PollEvent(&sdlEvent)) {
    switch (sdlEvent.type) {
      case SDL_QUIT: {
        return false;
      } break;

      case SDL_KEYDOWN: {
        switch (sdlEvent.key.keysym.scancode) {
          case SDL_SCANCODE_ESCAPE: {
            return false;
          } break;
          default: break;
        }
      } break;

      default: break;
    }
  }

  return true;
}

// perform any needed operations before the main game loop update
void SDLBackend::preFrameUpdate(float deltaTime) {

}

// perform any needed operations after the main game loop update
void SDLBackend::postFrameUpdate(float deltaTime) {

}

// perform any needed operations before the main game loop render
void SDLBackend::preFrameRender() {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  // things drawn on the screen will be white
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
}

// perform any needed operations after the main game loop render
void SDLBackend::postFrameRender() {
  SDL_RenderPresent(renderer);
}

// draws a filled circle to the screen
void SDLBackend::drawCircle(int x, int y, int radius) {
  int px = radius - 1;
  int py = 0;
  int tx = 1;
  int ty = 1;
  int err = tx - (radius << 1);
  while (px >= py) {
    SDL_RenderDrawLine(renderer, x, y, x + px, y - py);
    SDL_RenderDrawLine(renderer, x, y, x + px, y + py);
    SDL_RenderDrawLine(renderer, x, y, x - px, y - py);
    SDL_RenderDrawLine(renderer, x, y, x - px, y + py);
    SDL_RenderDrawLine(renderer, x, y, x + py, y - px);
    SDL_RenderDrawLine(renderer, x, y, x + py, y + px);
    SDL_RenderDrawLine(renderer, x, y, x - py, y - px);
    SDL_RenderDrawLine(renderer, x, y, x - py, y + px);

    if (err <= 0) {
      py += 1;
      err += ty;
      ty += 2;
    } else if (err > 0) {
      px -= 1;
      tx += 2;
      err += tx - (radius << 1);
    }
  }
}
