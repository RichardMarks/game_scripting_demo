# game.py
import engine

class Rect:
  def __init__(self, left, top, right, bottom):
    self.left = left
    self.top = top
    self.right = right
    self.bottom = bottom

class Ball:
  def __init__(self):
    self.x = 0
    self.y = 0
    self.xv = 0
    self.yv = 0
    self.speed = 256
    self.radius = 16

ball = Ball()

def gameCreate():
  global ball
  # get the window size
  screenWidth = engine.getScreenWidth()
  screenHeight = engine.getScreenHeight()

  # store bounds of screen on ball for easy collision detection
  ball.bounds = Rect(ball.radius, ball.radius, screenWidth - ball.radius, screenHeight - ball.radius)

  # center ball in middle of screen
  ball.x = screenWidth / 2
  ball.y = screenHeight / 2

  # start with an upwards right motion
  ball.xv = 1
  ball.yv = -1

def gameDestroy():
  pass

def gameUpdate(deltaTimeInSeconds):
  global ball
  # determine the ball's next position based on it's movement vector
  moveX = ball.xv * ball.speed * deltaTimeInSeconds
  moveY = ball.yv * ball.speed * deltaTimeInSeconds
  nextX = ball.x + moveX
  nextY = ball.y + moveY

  # if ball will hit left or right edges of the bounds
  if nextX <= ball.bounds.left or nextX >= ball.bounds.right:
    # reverse velocity of ball on x axis
    ball.xv = -ball.xv

    # stop the movement of the ball
    nextX = ball.x

  # if the ball will hit top or bottom edges of the bounds
  if nextY <= ball.bounds.top or nextY >= ball.bounds.bottom:
    # reverse velocity of ball on y axis
    ball.yv = -ball.yv

    # stop the movement of the ball
    nextY = ball.y

  # reposition the ball
  ball.x = nextX
  ball.y = nextY

def gameRender():
  global ball

  # must pass integers to the engine function call below
  x = int(ball.x)
  y = int(ball.y)
  radius = int(ball.radius)

  # draw the ball
  engine.drawCircle(x, y, radius)

# engine configuration dict
configuration = {
  'DEBUG': False,
  'SCREEN_WIDTH': 1920 / 2,
  'SCREEN_HEIGHT': 1080 / 2,
  'USE_FULLSCREEN': False,
  'WINDOW_TITLE': 'Python Bouncing Ball Demo',
  'create': 'gameCreate',
  'destroy': 'gameDestroy',
  'update': 'gameUpdate',
  'render': 'gameRender'
}

# the engine will execute this python file on load
# and this function call kicks things off
engine.init(configuration)
