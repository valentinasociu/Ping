
#include "LedControl.h"
#include "Timer.h"
#include "LiquidCrystal.h"

#define JOYSTICK A1
#define BOUNCE_VERTICAL 1
#define BOUNCE_HORIZONTAL -1
#define NEW_GAME_ANIMATION_SPEED 50
#define HIT_NONE 0
#define HIT_CENTER 1
#define HIT_LEFT 2
#define HIT_RIGHT 3

byte sad[] = 
{
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10011001,
  B10100101,
  B01000010,
  B00111100
};

byte smile[] = 
{
 B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
};

LedControl lc = LedControl(12, 11, 10, 1);
LiquidCrystal lcd(8, 9, 5, 4, 3, 2);

byte direction; 
int xball;
int yball;
int yball_prev;
byte xpad; 
int ball_timer = 0;
int score = 0;
int restart = 0;
int BALL_DELAY = 600;
int level = 0;
int start=  1;
int PADSIZE = 3;
int lim=6;

void NewScore()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LET'S PING PONG!");
  lcd.setCursor(0, 1);
  lcd.print("make a move!");
  delay(3000);
  score = 0;
  level = 0;
}

void ScoreBoard()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCORE: ");
  lcd.setCursor(10, 0);
  lcd.print(score); 
  lcd.setCursor(0, 1);
  lcd.print("LEVEL: ");
  lcd.setCursor(10,1);
  lcd.print(level);   
}

void Display(byte *sprite)
{
  for (int r = 0; r < 8; r++)
    lc.setRow(0, r, sprite[r]);
}

void NewGame() 
{
  while(start)
  {
    int value = analogRead(JOYSTICK); 
    Serial.println(value);
    if(value >= 450 && value <= 550)
       start = 1;  
    else
      start = 0;
  }
  lc.clearDisplay(0);
  xball = random(1, 7);   
  yball = 0;
  direction = random(3, 6);   
  Display(smile);
  delay(1000);
  lc.clearDisplay(0); 
}

int CheckBounce()
{
  if (xball == 0 || yball == 0 || xball == 7 || yball == 6) 
  {
    int bounce = (yball == 0 || yball == 6) ? BOUNCE_HORIZONTAL : BOUNCE_VERTICAL;  //horizontal when it hits left and right margins, vertical when it hits the upper or the lower margin
    return bounce;    //1 or -1
  }
  return 0;
}

int GetHit()
{
  if (yball != 6 || xball < xpad || xball >= xpad + PADSIZE)      //above the pad 
    return HIT_NONE;  
  if (xball == xpad + PADSIZE/2)      //center of the pad
    return HIT_CENTER; 
  return xball < xpad + PADSIZE / 2 ? HIT_LEFT : HIT_RIGHT;
}

bool CheckLoose() 
{
  return yball == 6 && GetHit() == HIT_NONE;      //returns 1 if the ball hit nothing 
}

bool CheckHit()
{
  if(GetHit() == HIT_CENTER || GetHit() == HIT_LEFT || GetHit() == HIT_RIGHT)
      return 1;
  return 0;
}

void MoveBall()
{
  int bounce = CheckBounce(); //1 or -1
  if (bounce) 
  { 
    switch (direction) 
    {
      case 0:
        direction = 4;
        break;
      case 1:
        direction = (bounce == BOUNCE_VERTICAL) ? 7 : 3;
        break;
      case 2:
        direction = 6;
        break;
      case 6:
        direction = 2;
        break;
      case 7:
        direction = (bounce == BOUNCE_VERTICAL) ? 1 : 5;
        break;
      case 5:
        direction = (bounce == BOUNCE_VERTICAL) ? 3 : 7;
        break;
      case 3:
        direction = (bounce == BOUNCE_VERTICAL) ? 5 : 1;
        break;
      case 4:
        direction = 0;
        break;
    }    
  }
  
  switch (GetHit())     //0 = none, 1 = center, 2 = left, 3 = right
  {
    case HIT_LEFT:
      if (direction == 0)   //go right
        direction =  7;
      else if (direction == 1)
        direction = 0;
      break;
    case HIT_RIGHT:
      if (direction == 0)   //go left
        direction = 1; 
      else if (direction == 7) 
        direction = 0;
      break;
  }

  //when the ball hits the margins 
  if ((direction == 0 && xball == 0) || (direction == 4 && xball == 7)) 
    direction++;
  if (direction == 0 && xball == 7) 
    direction = 7;
  if (direction == 4 && xball == 0) 
    direction = 3;
  if (direction == 2 && yball == 0) 
    direction = 3;
  if (direction == 2 && yball == 6) 
    direction = 1;
  if (direction == 6 && yball == 0) 
    direction = 5;
  if (direction == 6 && yball == 6) 
    direction = 7;

  // when the ball hits the corners
  if (xball == 0 && yball == 0) 
    direction = 3;
  if (xball == 0 && yball == 6) 
    direction = 1;
  if (xball == 7 && yball == 6) 
    direction = 7;
  if (xball == 7 && yball == 0) 
    direction = 5;

 //the transition
  yball_prev = yball;
  if (2 < direction && direction < 6)   //goes south
    yball++;
  else if (direction != 6 && direction != 2)  //for 1 and 7 goes north
    yball--;
  if (0 < direction && direction < 4)   //goes right 
    xball++;
  else if (direction != 0 && direction != 4) //goes left
    xball--;
    
  xball = max(0, min(7, xball));    //for x maximum 7
  yball = max(0, min(6, yball));    //for y maximum 6
}

void GameOver()
{
  BALL_DELAY = 600;
  start = 1;
  PADSIZE = 3;
  lim = 6;
  Display(sad);
  delay(2000);
  lc.clearDisplay(0);
  //delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("YOU SCORED ");
  lcd.setCursor(12, 0);
  lcd.print(score); 
  lcd.setCursor(0, 1);
  lcd.print("AT LEVEL  ");
  lcd.setCursor(12, 1);
  lcd.print(level);
  delay(3000);   
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("TRY AGAIN!");
  delay(3000);  
  
}

void Game()
{
  restart=0;
  if (yball_prev != yball) 
    lc.setRow(0, yball_prev, 0);
  lc.setRow(0, yball, byte(1 << (xball)));
  byte padmap = byte(0xFF >> (8 - PADSIZE) << xpad) ;
  lc.setRow(0, 7, padmap);
    
    if(BALL_DELAY>=40)
         BALL_DELAY-=2;
    delay(BALL_DELAY);
    MoveBall(); 
      if(CheckHit())
   {
    score++;
    if(score%5==0)
      level++;
    if(score==10)
      {
        PADSIZE--;   
        lim++;   
      }
   }  
}

    void SetPad() 
{
  xpad = map(analogRead(JOYSTICK), 0, 1023, 0, lim);
}

void setup()
{
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  randomSeed(analogRead(0));  //pseudo-random sequences that repeat exactly
  lcd.begin(16, 2);  
  Serial.begin(9600);
  pinMode(JOYSTICK, INPUT);
  NewScore();  
  NewGame();
  
}

void loop() 
{
  if(start==0)
  {
    ScoreBoard();
    SetPad();
    Game();     
  if (CheckLoose()) 
  {
    GameOver();
    NewScore();
    NewGame();
  }
  delay(30);
  }
}
