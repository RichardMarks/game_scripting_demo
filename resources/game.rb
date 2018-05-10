# game.rb

class Rect
  attr_accessor :left, :top, :right, :bottom
  def initialize(left, top, right, bottom)
    @left = left
    @top = top
    @right = right
    @bottom = bottom
  end
end

class Ball
  def initialize(x, y, xv, yv, speed, radius)
    # position of the ball's center
    @x = x
    @y = y
    # normalized velocity of ball
    @xv = xv
    @yv = yv
    # speed of ball in pixels per second
    @speed = speed
    # size of ball's radius in pixels
    @radius = radius
  end

  def update(deltaTimeInSeconds)
    # determine the ball's next position based on it's movement vector
    moveX = @xv * @speed * deltaTimeInSeconds
    moveY = @yv * @speed * deltaTimeInSeconds
    nextX = @x + moveX
    nextY = @y + moveY

    # if ball will hit left or right edges of the bounds
    if nextX <= @bounds.left or nextX >= @bounds.right then
      # reverse velocity of ball on x axis
      @xv = -@xv

      # stop the movement of the ball
      nextX = @x
    end

    # if the ball will hit top or bottom edges of the bounds
    if nextY <= @bounds.top or nextY >= @bounds.bottom then
      # reverse velocity of ball on y axis
      @yv = -@yv

      # stop the movement of the ball
      nextY = @y
    end

    # reposition the ball
    @x = nextX
    @y = nextY
  end

  def draw
    Engine::drawCircle(@x, @y, @radius)
  end

  def setBounds(left, top, right, bottom)
    @bounds = Rect.new(left, top, right, bottom)
  end
end

def gameCreate()
  screenWidth = Engine::getScreenWidth()
  screenHeight = Engine::getScreenHeight()
  x = screenWidth / 2
  y = screenHeight / 2
  xv = 1
  yv = -1
  speed = 256
  radius = 16

  $ball = Ball.new(x, y, xv, yv, speed, radius)
  $ball.setBounds(radius, radius, screenWidth - radius, screenHeight - radius)
end

def gameDestroy()
  $ball = nil
end

def gameUpdate(deltaTimeInSeconds)
  $ball.update(deltaTimeInSeconds)
end

def gameRender()
  $ball.draw()
end

# engine configuration hash
configuration = {
  :DEBUG => false,
  :SCREEN_WIDTH => 1920 / 2,
  :SCREEN_HEIGHT => 1080 / 2,
  :USE_FULLSCREEN => false,
  :WINDOW_TITLE => 'Ruby Bouncing Ball Demo',
  :create => 'gameCreate',
  :destroy => 'gameDestroy',
  :update => 'gameUpdate',
  :render => 'gameRender'
}

# the engine will execute this ruby file on load
# and this function call kicks things off
Engine::init(configuration)
