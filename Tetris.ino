#include <FastLED.h>

#define LED_PIN  5
#define UP_PIN 10
#define RIGHT_PIN 9
#define DOWN_PIN 11
#define LEFT_PIN 8 

#define BRIGHTNESS 6

#define NUM_LEDS 256

int up = 0;         // variable for reading the pushbutton status
int right = 0; 
int down = 0; 
int left = 0; 

struct Point
{int x,y;} a[4], b[4], c[4];

int n[2];

int figures[7][4] =
{
  2,4,5,7,//Z
  3,5,6,7,//L
  2,3,4,5,//O
  3,5,4,6,//S
  1,3,5,7,//I
  2,3,5,7,//J
  3,5,4,7,//T
};

int dx = 0; bool rotate = false; int rotate_dir = -1; bool create_shape = true; int score = 0;
//timer variables
unsigned long timer = 0, move_tick = 500; int ticks = 1;
//input flag variables
bool rotate_in_use = false;
bool move_in_use = false;

const int MatrixWidth = 16;
const int MatrixHeight = 16;
const int N = 16 ,M = 10, S=5;//N=height//M=Width//S=Field starting X//if S=5 Field is (5,0)->(5+M-1,0)
//Field of play
int FIELD[N][M] = {0};
//leds array
CRGB leds[NUM_LEDS];

//returns the number of led on the matrix that is on (x,y)
int XY(int x, int y)
{
  int i = 0;
  if(y % 2 == 0)
    i = y * MatrixWidth + x;
  else
    i = (y+1) * MatrixWidth - (x+1);
  return i;
}  
//checks if a shape is colliding with somthing
bool col_check()
{
  for(int i = 0; i < 4; i++)
  {
    if(a[i].x < S || a[i].x >= (S+M) || a[i].y >= N)
      return 0;
      else if(FIELD[a[i].y][a[i].x - S] != 0)
        return 0;
  }
    return 1;
}
void PrintField()
{
  for(int i = N-1; i > 0; i--)
  {
    for(int j = 0; j < M; j++)
      Serial.print(FIELD[i][j]);
    Serial.println("");
  }
  Serial.println("XXXXXXXXXXXXXXXXXXXXXX");
}
void DrawField()
{
  for(int i = N-1; i > 0; i--)
  {
    for(int j = S; j < M + S; j++)
    {
      leds[XY(j,i)] = CRGB::Black;
      if(FIELD[i][j - S] != 0)
        leds[ XY(j, i)]  = CHSV((FIELD[i][j - S]-1)*32,255,255);
    }
  }
}

void setup() {
  randomSeed(analogRead(0));
  random();
  //setup input and output
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );
  pinMode(UP_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(LEFT_PIN, INPUT);
  Serial.begin(9600);
  //create first shape//
  n[0] = random()%7;
  n[1] = random()%7;
  for(int i = 0; i < 4; i++)
  {
    a[i].x = figures[n[0]][i] % 2 + S;
    a[i].y = figures[n[0]][i] / 2;
    c[i].x = figures[n[1]][i] % 2 + 1;
    c[i].y = figures[n[1]][i] / 2 + 1;
  }
  for(int i = 0; i < 4; i++)
  {
    //
    leds[ XY(a[i].x, a[i].y)]  = CHSV(n[0]*32,255,255);
    leds[ XY(c[i].x, c[i].y)]  = CHSV(n[1]*32,255,255);
  }
  create_shape = false;
  //////////////UI////////////////////////
  for(int i=0; i<N; i++)
  {
    leds[XY(MatrixWidth - M -2,i)] = CRGB :: CadetBlue;
    leds[XY(MatrixWidth - 1,i)] = CRGB :: CadetBlue;
  }
  for(int i=0; i<=3; i++)
    for(int j=0; j<=5;j++)
    {
      if(j == 0 || j==5)
        leds[XY(i,j)] = CRGB :: Goldenrod;
      else if(i==0 || i==3)
        leds[XY(i,j)] = CRGB :: Goldenrod;
    }
}

void loop() {
  //Create new Shape//
  if(create_shape)
  { 
    n[0] = n[1];
    n[1] = random() % 7;
    for(int i = 0; i < 4; i++)
    {
      a[i].x = figures[n[0]][i] % 2 + S;
      a[i].y = figures[n[0]][i] / 2;
      c[i].x = figures[n[1]][i] % 2 + 1;
      c[i].y = figures[n[1]][i] / 2 + 1;
    }
    for(int i = 0; i < 4; i++)
    {
      leds[ XY(c[i].x, c[i].y)]  = CHSV(n[1]*32,255,255);
    } 
    create_shape = false;
  }
  ////////////input/////////////
  up = digitalRead(UP_PIN);
  right = digitalRead(RIGHT_PIN);
  down = digitalRead(DOWN_PIN);
  left = digitalRead(LEFT_PIN);
  if(right == LOW)
    dx = -1;
  if(left == LOW)
    dx = 1;
  if(up == LOW)
    rotate = true;
  //////////////move/////////////
  if(!move_in_use)
  {
    move_in_use = true;
    for(int i = 0; i < 4; i++)
    {
      b[i] = a[i];
      a[i].x += dx;
    }
    if(!col_check())
      for(int i = 0; i < 4; i++)
        a[i] = b[i];
  }
  //reset flag
  if(left == HIGH && right == HIGH)
  {
    move_in_use = false;
    dx = 0;
  } 
  /////////////rotate///////////(left:dir=1, right:dir=-1)//
  if(rotate && !rotate_in_use)
  {
    rotate_in_use = true;
    Point p = a[1];
    for(int i = 0; i < 4; i++)
    {
      int x = a[i].y - p.y;
      int y = a[i].x - p.x;
      a[i].x = p.x - (x * rotate_dir);
      a[i].y = p.y + (y * rotate_dir);
    }
    if(!col_check())
      for(int i = 0; i < 4; i++)
        a[i] = b[i];
  }
  //resets flag
  if(up == HIGH)
  {
    rotate = false;
    rotate_in_use = false;
  }
  
  //Timer for game ticks
  if(timer > move_tick * ticks)
  {
    for(int i = 0; i < 4; i++)
    {
      b[i] = a[i];
      a[i].y += 1;
    }
    if(!col_check())
    {
      for(int i = 0; i < 4; i++)
      {
        a[i] = b[i];
        FIELD[b[i].y][b[i].x - S] = n[0]+1;
      }
      create_shape = true;
    }
    ticks += 1;
  }
  timer = millis();
  ///////////showLeds////////////////////////////
  for(int i = 0; i < 4; i++)
  {
    leds[ XY(a[i].x, a[i].y)]  = CHSV(n[0]*32,255,255);
  }
  ////////////Checks for full line////////////////
  int k=N-1;
  bool need_refresh = false;
  for(int i = N-1; i > 0;i--)
  {
    int count = 0;
    for(int j = 0; j < M; j++)
    {
      if(FIELD[i][j])
        count ++;
      FIELD[k][j]=FIELD[i][j];
    }
    if(count<M)
    {
      k--;
    }
    else
    {
      need_refresh = true;
      score ++;
    }
  }
  if(need_refresh)
    DrawField();
  ///////////Score UI///////////////
  int score_division = score;
  int temp = score_division;
  if(score_division / 100 != 0)
  {
    for(int i = 15; i > 15-score_division/100; i--)
    {
      temp -= 100;
      leds[XY(0,i)] = CRGB::Turquoise;
    }
  }
  score_division = temp;
  if(score_division / 10 != 0)
  {
    for(int i = 15; i > 15-score_division/10; i--)
    {
      temp -= 10;
      leds[XY(1,i)] = CRGB::DarkGoldenrod;
    }
  }
  score_division = temp;
  if(score_division / 4 != 0)
  {
    for(int i = 15; i > 15-score_division/4; i--)
    {
      temp -= 4;
      leds[XY(2,i)] = CRGB::Silver;
    }
  }
  score_division = temp;
  if(score_division % 4 != 0)
    for(int i = 15; i > 15-score_division%4; i--)
      leds[XY(3,i)] = CRGB::Sienna;
  FastLED.show();
  //remove previous leds
  if(!create_shape)
    for(int i = 0; i < 4; i++)
    {
      leds[ XY(a[i].x, a[i].y)]  = CRGB::Black;
    }

  if(score % 100 == 0)
    for(int i = 15; i > 15-score/100; i--)
      leds[XY(0,i)] = CRGB::Black;
  if(score % 10 == 0)
    for(int i = 15; i > 15-score/10; i--)
      leds[XY(1,i)] = CRGB::Black;
  if(score % 4 == 0)
    for(int i = 15; i > 15-score/4; i--)
      leds[XY(2,i)] = CRGB::Black;
  if(score % 4 != 0)
    for(int i = 15; i > 15-score%4; i--)
      leds[XY(3,i)] = CRGB::Black;
  
  if(create_shape)
    for(int i = 0; i < 4; i++)
    {
      leds[ XY(c[i].x, c[i].y)]  = CRGB::Black;
    }
}
