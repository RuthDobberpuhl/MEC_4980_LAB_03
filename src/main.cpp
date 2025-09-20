#define MICRO

//#define NARROW
//#define TRANSPARENT

//////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Include the SparkFun qwiic OLED Library
#include <SparkFun_Qwiic_OLED.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; 

#if defined(TRANSPARENT)
QwiicTransparentOLED myOLED;
const char * deviceName = "Transparent OLED";

#elif defined(NARROW)
QwiicNarrowOLED myOLED;
const char * deviceName = "Narrow OLED";

#else
QwiicMicroOLED myOLED;
const char * deviceName = "Micro OLED";

#endif

int yoffset;
float targetTemperature = 20.0;
char degreeSysC[] = "C";
char degreeSysF[] = "F";
int pinButton = 10;
int pinUp =11;
int pinDown = 12;
int LED = 9;
bool prevPressed = false;
bool prevUp =false;
bool prevDown = false;
bool useFahrenheit = false;

enum MachineStates {
  DisplayTemps, //0
  SetTemp,//1
  ChooseSystem //2
};

MachineStates currentState;

void setup()
{
  Serial.begin(9600);
  while(!Serial) { delay(10); }
  currentState = DisplayTemps;
  pinMode(pinButton,INPUT_PULLDOWN);
  pinMode(pinUp,INPUT_PULLDOWN);
  pinMode(pinDown,INPUT_PULLDOWN);
  pinMode( LED ,OUTPUT);
  delay(500);
  Serial.println("Testing BME sensor"); 
  
  if (! bme.begin(0x77, &Wire)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }
    delay(500);   //Give display time to power on


    Serial.println("\n\r-----------------------------------");

    Serial.print("Running Test #5 on: ");
    Serial.println(String(deviceName));

    if(!myOLED.begin()){

        Serial.println("- Device Begin Failed");
        while(1);
    }

    yoffset = (myOLED.getHeight() - myOLED.getFont()->height)/2;

    delay(1000);
}

// Our testing functions

void scroll_right(void){

    myOLED.scrollStop();
    myOLED.scrollRight(0, 7, SCROLL_INTERVAL_2_FRAMES); 
}

void scroll_right_vert(void){
    myOLED.scrollStop();    
    myOLED.scrollVertRight(0, 7, SCROLL_INTERVAL_3_FRAMES); 
}

void scroll_left(void){
    myOLED.scrollStop();    
    myOLED.scrollLeft(0, 7, SCROLL_INTERVAL_4_FRAMES);
}

void scroll_left_vert(void){
    myOLED.scrollStop();    
    myOLED.scrollVertLeft(0, 7, SCROLL_INTERVAL_5_FRAMES);
}

void scroll_stop(void){
    myOLED.scrollStop();
}

void flip_horz(void){

    for(int i=0; i < 6; i++){
        myOLED.flipHorizontal(!(i & 0x01));
        delay(800);
    }
}

void flip_vert(void){
    for(int i=0; i < 6; i++){
        myOLED.flipVertical(!(i & 0x01));
        delay(800);
    }
}

void invert(void){
    for(int i=0; i < 6; i++){
        myOLED.invert(!(i & 0x01));
        delay(800);
    }    
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Use an array of testing functions, with a title, to run the tests

typedef void (*testFn)(void);
typedef struct _testRoutines{
    void (*testFn)(void);
    const char *title;
}testRoutine;

static const testRoutine testFunctions[] = {
    {scroll_right, "Right>"},
    {scroll_right_vert, "^Right-Up>"},
    {scroll_left, "<Left"},
    {scroll_left_vert, "<Left-Up^"},
    {scroll_stop, "<STOP>"},
    {flip_horz, "-Flip-Horz-"},    
    {flip_vert, "|Flip-Vert|"},    
    {invert, "**INVERT**"}        
}; 

float cToF(float degC){
  return(degC+32.0)*9.0/5.0;
}

void loop()
{
  delay(100);
    float temp = bme.readTemperature();
  // Button to change state (short press cycles modes)
  bool pressed = digitalRead(pinButton);
  if(pressed && !prevPressed){
    currentState = MachineStates(((int)currentState +1) % 3);
    Serial.print("State changed to: ");
    Serial.println((int)currentState);
  }
  prevPressed = pressed;



  char myNewText[50];
  if (currentState == DisplayTemps) {


    sprintf(myNewText,"Tc: %.1f", temp);


    myOLED.erase();
    myOLED.text(3, yoffset,myNewText);

    sprintf(myNewText,"Ttar: %.1f", targetTemperature);  
    myOLED.text(3, yoffset+12,myNewText);
    myOLED.display();

  }  else if (currentState == SetTemp){
      // Up/Down adjust target temperature (in current displayed units)
      bool up = digitalRead(pinUp);
      bool down = digitalRead(pinDown);
      if (up && !prevUp){
        if(useFahrenheit) targetTemperature += 1.0; else targetTemperature += 1.0;
      }

      if(down && !prevDown){
        if(useFahrenheit) targetTemperature -= 1.0; else targetTemperature -= 1.0;

      }

      prevUp = up;
      prevDown = down;

      sprintf(myNewText,"Ttar: %.1f", targetTemperature);
      myOLED.erase();
      myOLED.text(3, yoffset,myNewText);
      myOLED.display();
      
  } else if (currentState == ChooseSystem){
      // In ChooseSystem state: pressing Up toggles Celsius/Fahrenheit
      bool upNow = digitalRead(pinUp);
      if (upNow && !prevUp) {
        useFahrenheit = !useFahrenheit;
        Serial.print("Toggled units. Now Fahrenheit: ");
        Serial.println(useFahrenheit);
      }
      prevUp = upNow;

      const char *degreeSys = useFahrenheit ? degreeSysF : degreeSysC;
      sprintf(myNewText,"System: %s", degreeSys);
      myOLED.erase();
      myOLED.text(3, yoffset,myNewText);
      myOLED.display();
  }

  // Convert temperature for display if needed
  float displayTemp = temp;
  if (useFahrenheit) displayTemp = cToF(temp);

  if (displayTemp <= targetTemperature){
    digitalWrite(LED, HIGH);
  }
  else if (displayTemp > targetTemperature){
    digitalWrite(LED, LOW);
  }

 }
