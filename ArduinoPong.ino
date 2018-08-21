/*
   Arduino Pong
   by Pete Lamonica
   
   modified by duboisvb
   updated by James Bruce (http://www.makeuseof.com/tag/author/jbruce)
   updated by Marco Parenzan (http://github.com/marcoparenzan/arduinopong)
   
   A simple implementation of Pong on the Arduino using a TV for output.
*/

#include <TVout.h>
#include <fontALL.h>

#define WHEEL_ONE_PIN 0 //analog
#define WHEEL_TWO_PIN 1 //analog
#define BUTTON_ONE_PIN 2 //digital to start game

#define LEFT_PADDLE_X 3
#define RIGHT_PADDLE_X (TV.hres()-4)

#define PADDLE_WIDTH 1
#define PADDLE_HEIGHT 14

#define IN_GAME 0 //in game state - draw constants of the game box
#define IN_MENU 1 //in menu state
#define GAME_OVER 2 //game over state

#define LEFT_SCORE_X (TV.hres()/2-10)
#define RIGHT_SCORE_X (TV.hres()/2+10)
#define SCORE_Y 4

#define PLAY_TO 7

TVout TV;

boolean button1Status = false;

int leftPaddleRead = 0;
int leftPaddleY = 0;
int rightPaddleRead = 0;
int rightPaddleY = 0;

int ballX = 0;
int ballY = 0;
int ballVolX;
int ballVolY;

int leftScore = 0;
int rightScore = 0;

int state = IN_MENU;

void renderMenu() {
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.print(10, 5, "Arduino Pong");
  TV.select_font(font4x6);
  TV.print(22, 35, "Version by");
  TV.print(22, 45, "Marco Parenzan");
  TV.print(22, 65, "Press button");
  TV.print(22, 75, "to start");
}

void renderPaddle(int x, int y) {
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, y, x + i, y + PADDLE_HEIGHT, 1);
  }
}

void renderBall(int x, int y) {
  TV.set_pixel(x, y, 2);
}

void renderScore(int x, int score) {
  if (score < 10)
    TV.print_char(x, SCORE_Y, '0' + score);
  else
    TV.print_char(x, SCORE_Y, score);
}

void renderGame() {
  TV.clear_screen();
  TV.select_font(font4x6);

  //draw net
  for (int i = 1; i < TV.vres() - 4; i += 6) {
    TV.draw_line(TV.hres() / 2, i, TV.hres() / 2, i + 3, 1);
  }
  // had to make box a bit smaller to fit tv
  TV.draw_line(0, 0, 0, 95, 1 ); // left
  TV.draw_line(0, 0, 126, 0, 1 ); // top
  TV.draw_line(126, 0, 126, 95, 1 ); // right
  TV.draw_line(0, 95, 126, 95, 1 ); // bottom

  //draw right paddle
  renderPaddle(LEFT_PADDLE_X - PADDLE_WIDTH, leftPaddleY);

  //draw left paddle
  renderPaddle(RIGHT_PADDLE_X, rightPaddleY);

  //draw ball
  renderBall(ballX, ballY);

  //draw scores
  renderScore(LEFT_SCORE_X, leftScore);
  renderScore(RIGHT_SCORE_X, rightScore);
}

void renderGameOver() {
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.print(29, 25, "GAME");
  TV.print(68, 25, "OVER");
}

void updateMenu() {
  if (button1Status != 0) {
    //reset the scores
    leftScore = 0;
    rightScore = 0;
    ballX = TV.hres() / 2;
    ballY = TV.vres() / 2;
    ballVolX = 1;
    ballVolY = 1;
    leftPaddleRead = analogRead(WHEEL_ONE_PIN);
    rightPaddleRead = analogRead(WHEEL_TWO_PIN);


    state = IN_GAME;
    delay(1000);
    return;
  }
}

void updateGame() {
  if (button1Status != 0) {
    state = IN_MENU;
    delay(1000);
    return;
  }

  // test left side for wall hit
  if ((ballVolX < 0) && (ballX == (LEFT_PADDLE_X + PADDLE_WIDTH)) && (ballY >= leftPaddleY) && (ballY <= (leftPaddleY + PADDLE_HEIGHT))) {
    ballVolX = -ballVolX;
    ballVolY += 2 * ((ballY - leftPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
    playBall();
  }
  // test right side for wall hit
  else if ((ballVolX > 0) && (ballX == RIGHT_PADDLE_X) && (ballY >= rightPaddleY) && (ballY <= (rightPaddleY + PADDLE_HEIGHT))) {
    ballVolX = -ballVolX;
    ballVolY += 2 * ((ballY - rightPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
    playBall();
  }
  else if ((ballVolX < 0) && (ballX <= 1)) {
    rightScore++;
    playScored();
    if (rightScore == PLAY_TO) {
      state = GAME_OVER;
    }
    ballVolX = -ballVolX;
  }
  // test right side for wall hit
  else if ((ballVolX > 0) && (ballX >= TV.hres() - 1)) {
    leftScore++;
    playScored();
    if (leftScore == PLAY_TO) {
      state = GAME_OVER;
    }
    ballVolX = -ballVolX;
  }
  else if ((ballVolY < 0 && ballY <= 1) || (ballVolY > 0 && (ballY >= TV.vres() - 1))) {
    // change if hit top or bottom
    ballVolY = -ballVolY;
    playBall();
  }
  else {
    ballX += ballVolX;
    ballY += ballVolY;
  }
}

void playScored() {
  delay(100);
  TV.tone( 500, 300 );
}

void playBall() {
  delay(100);
  TV.tone( 2000, 30 );
}

void updateGameOver() {
  if (button1Status != 0) {
    delay(1000);
    state = IN_MENU;
  }
}

void setup()  {
  Serial.begin(9600);
  TV.begin(_NTSC);
}

void processSystemButtons() {
  button1Status = digitalRead(BUTTON_ONE_PIN);
}

void loop() {
  processSystemButtons();

  if (state == IN_GAME) {
    leftPaddleRead = 0.8 * leftPaddleRead + 0.2 * analogRead(WHEEL_ONE_PIN);
    leftPaddleY = ((leftPaddleRead / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;
    rightPaddleRead = 0.8 * rightPaddleRead + 0.2 * analogRead(WHEEL_TWO_PIN);
    rightPaddleY = ((rightPaddleRead / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;

    renderGame();
    updateGame();
  }
  else if (state == IN_MENU) {
    renderMenu();
    updateMenu();
  }
  else if (state == GAME_OVER) {
    renderGameOver();
    updateGameOver();
  }

  delay(20);
}
