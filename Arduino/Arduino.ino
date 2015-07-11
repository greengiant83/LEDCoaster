#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include "blue.h"
#define LEDPIN 6
#define FLEXPIN A0

#define MINFLEXVALUE 0
#define MAXFLEXVALUE 1024
#define SHOOTINGTHRESHOLD 0.01
#define DEBOUNCETIMEMILLIS 100
#define INTARGETTHRESHOLD 200
#define MAXCARS 50
#define CARLIFESPAN 4000
#define VELOCITYSCALAR 0.5
#define GRAVITYSCALAR 0.001
#define FRICTION 0.999
#define PLUNGERCOLOR strip.Color(255, 255, 255)
#define PLUNGERHEIGHT 4

struct Car
{
  float position;
  float velocity;
  bool isActive;
  int ttl;
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, LEDPIN, NEO_GRB + NEO_KHZ800);
Car cars[MAXCARS];

float highShootValue;
float shootValue;
boolean isShooting;
boolean isInTargetRange;
int inTargetRangeDuration;
unsigned long lastShotTime;
uint32_t carColor;
uint32_t plungerColor;
uint32_t targetColor;



void setup() 
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  carColor = CARCOLOR;
  plungerColor = PLUNGERCOLOR;
  targetColor = strip.Color(255, 255, 255);
  lastShotTime = 0;
  isInTargetRange = false;
  inTargetRangeDuration = 0;
  
  Serial.begin(9600);
  Serial.println("Coaster ready");
}

void monitorShooter()
{
  int sensorValue = analogRead(FLEXPIN);
  //Serial.println(sensorValue);
  //return;
  float newShootValue = mapfloat(sensorValue, MINFLEXVALUE, MAXFLEXVALUE, 0, 1);
  
  if(!isShooting)
  {
    if(newShootValue >= SHOOTINGTHRESHOLD)
    {
      isShooting = true;
      highShootValue = shootValue;
    }
  }
  
  if(isShooting)
  {
    if(newShootValue > highShootValue) highShootValue = newShootValue;
    if(newShootValue < SHOOTINGTHRESHOLD)
    {
      isShooting = false;
      fireShot(shootValue);
    }
  }

  shootValue = newShootValue;
}

void loop() 
{
  monitorShooter();
  updateCars();
  monitorTarget();
  draw();
}

void updateCars()
{
  isInTargetRange = false;
  for(int i=0;i<MAXCARS;i++)
  {
    if(cars[i].isActive)
    {
      cars[i].ttl--;
      cars[i].velocity *= FRICTION;
      cars[i].velocity += forces[(int)cars[i].position] * GRAVITYSCALAR;
      cars[i].position += cars[i].velocity;

      if(cars[i].position >= TARGETSTART && cars[i].position <= TARGETEND) isInTargetRange = true;

      //Remove the car when its ttl (time to live) counter reaches zero
      if(cars[i].ttl <= 0 || cars[i].position > strip.numPixels() || cars[i].position <= 0) cars[i].isActive = false; 
    }
  }
}

void monitorTarget()
{
  isInTargetRange = false;
  for(int i=0;i<MAXCARS;i++)
  {
    if(cars[i].isActive)
    {
      if(cars[i].position >= TARGETSTART && cars[i].position <= TARGETEND) isInTargetRange = true;
    }
  }

  if(isInTargetRange)
  {
    inTargetRangeDuration++;
    if(inTargetRangeDuration >= INTARGETTHRESHOLD)
    {
      drawScoreAnimation();
      
      //Deactivate cars in the target region
      for(int i=0;i<MAXCARS;i++)
      {
          if(cars[i].isActive && cars[i].position >= TARGETSTART && cars[i].position <= TARGETEND) cars[i].isActive = false;
      }
    }
  }
  else inTargetRangeDuration = 0;
}

void draw()
{
  strip.clear();
  if(SHOULDDRAWTARGET) drawTarget();
  drawCars();
  strip.show();
}

///Note this is a blocking call. Nothing else will happen while the animation is going
void drawScoreAnimation()
{
  int blinkDelay = 200;
  int startIndex = TARGETPOINT;
  int endIndex = TARGETPOINT;
  for(int c=0;c<500;c++)
  {
    for(int i=startIndex;i<=endIndex;i++)
    {
      strip.setPixelColor(i, carColor);
    }
    strip.show();
    delay(blinkDelay);

    for(int i=startIndex;i<=endIndex;i++)
    {
      strip.setPixelColor(i, strip.Color(255, 255, 255));
    }
    strip.show();
    delay(blinkDelay);

    blinkDelay -= 12;
    if(blinkDelay < 0) blinkDelay = 1;

    if(startIndex <= 0 && endIndex >= 150) break;
    startIndex--;
    endIndex++;
  }
}

void drawTarget()
{
  uint32_t drawColor;
  if(isInTargetRange)
    drawColor = strip.Color(255, 255, 0);
  else 
    drawColor = targetColor;
    
  for(int i=TARGETSTART;i<=TARGETEND;i++)
  {
    strip.setPixelColor(i, drawColor);
  }
}

void drawCars()
{
  int plungerDepth = (1 - shootValue) * PLUNGERHEIGHT;
  for(int i=0;i<plungerDepth;i++)
  {
    strip.setPixelColor(i, plungerColor);
  }
  strip.setPixelColor(plungerDepth, carColor);
  
  for(int i=0;i<MAXCARS;i++)
  {
    if(cars[i].isActive)
    {
      strip.setPixelColor((int)cars[i].position, carColor);
    }
  }
}

void fireShot(float shootValue)
{
  if(millis() < lastShotTime + DEBOUNCETIMEMILLIS) return;
  Serial.print("Shot: ");
  Serial.println(shootValue);
  
  for(int i=0;i<MAXCARS;i++)
  {
    if(!cars[i].isActive)
    {
      cars[i].ttl = CARLIFESPAN;
      cars[i].isActive = true;
      cars[i].position = PLUNGERHEIGHT;
      cars[i].velocity = shootValue * VELOCITYSCALAR;
      lastShotTime = millis();
      break;
    }
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
