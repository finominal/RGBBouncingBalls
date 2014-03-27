#include <Adafruit_WS2801.h>
#include <SPI.h>

#define p(x) Serial.print(x) 
#define pl(x) Serial.println(x) 


//control
int numPixels = 84;
Adafruit_WS2801 strip = Adafruit_WS2801(numPixels, 6, 7);

//leds
int brightness = 255;
const uint32_t red = Color(brightness,0,0);
const uint32_t green = Color(0,brightness,0);
const uint32_t blue = Color(0,0,brightness);
const uint32_t purple = Color(brightness,0,brightness);
const uint32_t yellow = Color(brightness,brightness,0);
const uint32_t black = Color(0,0,0);

//physics
float gravity = -9.8;
float stickHeight = 2550;
int bottom = 0;

//objects
struct ball {
  uint32_t color;
  float loc; //location in mm
  float lastLoc; 
  int lastLocCounter;
  int pos;
  float velocity;
  bool bouncing; //bouncing, or moving up?
  int wait;
  int waiting;//current pause, count frames
  float friction;
  
  //millis returns an unsigned long 
  long lastFrameTime;
  long frameTime;
};

ball balls[5];

//animation types
enum program {bouncingBalls};
program CURRENT_PROGRAM = bouncingBalls;

void setup()
{
  Serial.begin(115200);
  Serial.print("Start");
  strip.begin(); 
  strip.show();
}

void loop()
{
  BounceBalls();
}

void BounceBalls()
{
  InitializeBalls();
  
  while(CURRENT_PROGRAM == bouncingBalls)
  { 
    for(int i = 0; i<3; i++)//for each ball
    { 
      if(balls[i].bouncing)
      {
        if(balls[i].waiting > 0) 
        {
          balls[i].waiting--;
          strip.setPixelColor(balls[i].pos, Combine(balls[i].color,strip.getPixelColor(balls[i].pos)));
          balls[i].lastFrameTime = millis();//reset compare last so we can start a new bounce  
        }
        else
        {
          balls[i].frameTime = millis();

          //Drop science, yo.
          balls[i].velocity += ((gravity * (balls[i].frameTime - balls[i].lastFrameTime))/1000);//apply gravity to the objects velocity
          balls[i].loc = balls[i].loc + balls[i].velocity * (numPixels/(stickHeight/1000)); //move location based on current velocity
   
          //check if at/passed the bottom, and reverse direction
          if(balls[i].loc < 0)
          {
            balls[i].loc = 0;//hit the bottom
            balls[i].velocity *= balls[i].friction;//impact friction
          }
          balls[i].pos = map(balls[i].loc,1,stickHeight,bottom,numPixels-1);//normalize the position
          
          //update strip by combining colors
          strip.setPixelColor(balls[i].pos, Combine(balls[i].color,strip.getPixelColor(balls[i].pos)));
          //check if at the bottom itterator
          
          //track if ball is at rest
          if(balls[i].loc == balls[i].lastLoc) {balls[i].lastLocCounter++; } else {balls[i].lastLocCounter = 0;}
          
          //update this location for use in the next frame
          balls[i].lastLoc = balls[i].loc;
          
          //if ball has been at the bottom for 100 frames, change the bouncing state to start rise. 
          if(balls[i].lastLocCounter>100) 
          { 
            balls[i].bouncing = false; //reset 
            balls[i].waiting = 80;//wait 80 frames, about 1 second.
          }
          
          //record the frame time, used in next itteration to determin velocity. 
          balls[i].lastFrameTime = balls[i].frameTime;
        }
      }
      else //not bouncing anymore, move up and pause 
      {
        if(balls[i].waiting > 0) 
        {
          balls[i].waiting--;
          //set the ball to be at the bottom position while waiting
          strip.setPixelColor(balls[i].pos, Combine(balls[i].color,strip.getPixelColor(balls[i].pos)));
        }
        else
        {
           if(balls[i].pos == numPixels-1) //are we at the top yet?
           {
             balls[i].bouncing = true;
             balls[i].waiting = random(100, 500);//how long (frames) to wait at the top of the stick for
             balls[i].friction = random(78, 97);
             balls[i].friction /= -100; //random returns positive int, we require negative float
             //Serial.print(i);Serial.print("f="); Serial.println(balls[i].friction);
             balls[i].loc = stickHeight;//reset location as we forced up via transalted position (pos)
             balls[i].velocity = 0;
           }
           else//not at the top, move ball up
           {
             balls[i].pos++;
             strip.setPixelColor(balls[i].pos, Combine(balls[i].color,strip.getPixelColor(balls[i].pos)));
             balls[i].waiting = balls[i].wait;
           }
        }
      }
    }
  
    //show what has been written to the strip.
    strip.show();
  
    //after pushing colors to the strip, clear all the balls from the strip ready to draw the next frame
    for(int i = 0; i<5; i++)strip.setPixelColor(balls[i].pos,black);
  
  }//while(CURRENT_PROGRAM = bouncingBalls)
}


/*

struct ball {
  uint32_t color;
  float loc;
  float lastLoc; 
  int pos;
  int lastLocCounter;
  float velocity;
  bool bouncing; //bouncing, or moving up?
  int wait;
  int waiting;//current pause, count frames
  float friction;
  float frameTime;
  float lastFrameTime;
};
*/

void InitializeBalls()
{
  //friction  = -0.8 
  ball redBall =   {red,stickHeight,0,numPixels-1,0,0,true,3,0  ,-0.93,0,millis()};
  ball greenBall = {green,stickHeight,0,numPixels-1,0,0,true,3,250,-0.94,0,millis()};
  ball blueBall =  {blue,stickHeight,0,numPixels-1,0,0,true,3,500,-0.95,0,millis()};
  ball purleBall =  {purple,stickHeight,0,numPixels-1,0,0,true,3,650,-0.96,0,millis()};
  ball yellowBall =  {yellow,stickHeight,0,numPixels-1,0,0,true,3,850,-0.91,0,millis()};
  
  balls[0] = redBall;
  balls[1] = greenBall;
  balls[2] = blueBall;
  balls[3] = purleBall;
  balls[4] = yellowBall;
}

uint32_t Combine(uint32_t c1, uint32_t c2)
{
  uint32_t result = c1; 
  result |= c2;
  return result;
}


// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}



