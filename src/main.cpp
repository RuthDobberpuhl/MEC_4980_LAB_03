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
char degreeSys[] = "C";
int pinButton = 10;
int pinUp =11;
int pinDown = 12;
int LED = 9;
bool prevPressed = false;
bool prevUp =false;
bool prevDown = false;

enum MachineStates {
  DisplayTemps, //0
  SetTemp,//1
  ChooseSystem //2
};

MachineStates currentState;

void setup()
{
  while(!Serial);
  currentState = DisplayTemps;
  pinMode(pinButton,INPUT_PULLDOWN);
  pinMode(pinUp,INPUT_PULLDOWN);
  pinMode(pinDown,INPUT_PULLDOWN);
  pinMode( LED ,OUTPUT);
  Serial.begin(9600); 
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


 

float cToF(float degC){
  return(degC*9.0/5.0+32.0);
}
float FToc(float degF){
    return(degF-32.0)*5.0/9.0;
}

void loop()
{
  delay(100);
  float temp = bme.readTemperature();
  if (strcmp(degreeSys, "F") == 0) {
        temp = cToF(temp); 
      }
  if(digitalRead(pinButton) && !prevPressed){
    currentState = MachineStates(((int)currentState +1) % 3);
  }

  prevPressed =digitalRead(pinButton);

  char myNewText[50];
  if (currentState == DisplayTemps) {
    sprintf(myNewText,"Tc: %.1f", temp);
    myOLED.erase();
    myOLED.text(3, yoffset,myNewText);
    sprintf(myNewText,"Ttar: %.1f", targetTemperature);  
    myOLED.text(3, yoffset+12,myNewText);
    myOLED.display();
  }  
  else if (currentState == SetTemp){
      if (digitalRead(pinUp) && !prevUp){
           targetTemperature ++;
      }

      if(digitalRead(pinDown)&& !prevDown){
            targetTemperature --;

      }

    sprintf(myNewText,"Ttar: %.1f", targetTemperature);
    myOLED.erase();
    myOLED.text(3, yoffset,myNewText);
    myOLED.display();
  } 
  
  else if (currentState == ChooseSystem){
    
    if (digitalRead(pinUp) && !prevUp){
      if (strcmp(degreeSys, "C") == 0) {
      temp = cToF(temp); 
      targetTemperature = cToF(targetTemperature); 
      strcpy(degreeSys, "F");
      }   
      else if (strcmp(degreeSys, "F") == 0) {
        temp = FToc(temp); 
        targetTemperature = FToc(targetTemperature); 
        strcpy(degreeSys, "C");
      }
    }  

    sprintf(myNewText,"System: %s", degreeSys);
    myOLED.erase();
    myOLED.text(3, yoffset,myNewText);
    myOLED.display();

  }

  if (temp <= targetTemperature){
    digitalWrite(LED, HIGH);
  }
  else if (temp > targetTemperature){
    digitalWrite(LED, LOW);
  }

 }
