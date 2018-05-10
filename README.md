# Game Scripting Demo

## Quickstart

```
$ git clone https://github.com/RichardMarks/game_scripting_demo.git demo
$ cd demo
$ make
$ cd ./bin
$ ./game
```

> Protip: setup an alias to run the game `alias run="cd ./bin && ./game && cd .."` so you can quickly re-run the game using just `run`

## What next?
Modify the `game.lua` file in the `resources` directory and run `make resources` before running the game again.

## How does it work?
The C++ side loads the `game.lua` file and reads the script.
It assumes nothing, and will do nothing (other than open a window that you can close with ESC) if you don't tell it to.

The C++ side _provides_ to the lua side a global `engine` table object with the following methods:

+ `engine:init` - accepts a configuration table object to configure the engine on startup
+ `engine:getScreenWidth` returns an integer of the width of the window
+ `engine:getScreenHeight` returns an integer of the height of the window
+ `engine:drawCircle` - accepts the x and y position of the circle and the radius of the circle to draw a circle on the screen

## Configuration

The engine may be configured from the lua side by passing a table to the `engine:init` method with any of the following fields:

+ `SCREEN_WIDTH` - an integer. specifies the width of the window
+ `SCREEN_HEIGHT` - an integer. specifies the height of the window
+ `WINDOW_TITLE` - a string. specifies the window title text
+ `DEBUG` - a boolean. specifies if you want verbose debugging text dumped to stdout
+ `USE_FULLSCREEN` a boolean. specifies if you want to run in fullscreen (true) or windowed (false)
+ `create` a string. specifies the name of the function to call for the engine's `create` lifecycle event
+ `destroy` a string. specifies the name of the function to call for the engine's `destroy` lifecycle event
+ `update` a string. specifies the name of the function to call for the engine's `update` lifecycle event
+ `render` a string. specifies the name of the function to call for the engine's `render` lifecycle event

## Engine Lifecycle Events

The engine has 4 lifecycle events which your script can "hook" to provide functionality.

### create
The `create` lifecycle event happens once after the script has been loaded and exeuted before the main game loop starts.

This is where you should setup any objects in your game

### destroy
The `destroy` lifecycle event happens once just before the game process finishes execution

This is where you should release any allocated resources or data structures

### update
The `update` lifecycle event happens once every frame and the engine passes the time since the last update in seconds to the callback

This is where you should move your things in your game

### render
The `render` lifycycle event happens once every frame after the update event

This is where you should draw things in your game


## LICENSE
MIT License (c) 2018, Richard Marks
See the included LICENSE.md file for more details.
