-- game.lua

-- local game state
local ball = {
  -- position of ball's center
  x = 0,
  y = 0,
  -- normalized velocity of ball
  xv = 1,
  yv = 1,
  -- speed of ball in pixels per second
  speed = 256,
  -- size of ball's radius in pixels
  radius = 16
}

function gameCreate()
  -- get the window size
  screenWidth = engine:getScreenWidth()
  screenHeight = engine:getScreenHeight()

  -- store bounds of screen on ball for easy collision detection
  ball.bounds = {
    left = ball.radius,
    top = ball.radius,
    right = screenWidth - ball.radius,
    bottom = screenHeight - ball.radius
  }

  -- center ball in middle of screen
  ball.x = screenWidth / 2
  ball.y = screenHeight / 2

  -- start with an upwards right motion
  ball.xv = 1
  ball.yv = -1
end

function gameDestroy()
  -- we don't create any objects, so nothing to destroy
end

function gameUpdate(deltaTimeInSeconds)
  -- determine the ball's next position based on it's movement vector
  local moveX = ball.xv * ball.speed * deltaTimeInSeconds
  local moveY = ball.yv * ball.speed * deltaTimeInSeconds
  local nextX = ball.x + moveX
  local nextY = ball.y + moveY

  -- if ball will hit left or right edges of the bounds
  if nextX <= ball.bounds.left or nextX >= ball.bounds.right then
    -- reverse velocity of ball on x axis
    ball.xv = -ball.xv

    -- stop the movement of the ball
    nextX = ball.x
  end

  -- if the ball will hit top or bottom edges of the bounds
  if nextY <= ball.bounds.top or nextY >= ball.bounds.bottom then
    -- reverse velocity of ball on y axis
    ball.yv = -ball.yv

    -- stop the movement of the ball
    nextY = ball.y
  end

  -- reposition the ball
  ball.x = nextX
  ball.y = nextY
end

function gameRender()
  -- draw the ball
  engine:drawCircle(ball.x, ball.y, ball.radius)
end

-- engine configuration table
local configuration = {
  DEBUG = false,
  SCREEN_WIDTH = 1920 / 2,
  SCREEN_HEIGHT = 1080 / 2,
  USE_FULLSCREEN = false,
  WINDOW_TITLE = "Lua Bouncing Ball Demo",
  create = "gameCreate",
  destroy = "gameDestroy",
  update = "gameUpdate",
  render = "gameRender"
}

-- the engine will execute this lua file on load
-- and this function call kicks things off
engine:init(configuration)
