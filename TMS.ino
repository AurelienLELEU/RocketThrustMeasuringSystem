#include <SD.h>
#include <SPI.h>
#include <HX711_ADC.h>

volatile long StartTime = 0;
volatile long CurrentTime = 0;
volatile long Pulses = 0;
int PulseWidth = 0;
File data;
float loadCellData;
float timeCount = 0;
boolean sdBeginSuccess = true;

enum Pinout {
  RCPin = 2,
  HX711_dt = 4,
  HX711_sck = 5,
  LEDRedPin = 7,
  LEDGreenPin = 8,
  LEDBluePin = 6,
  FusePin = 3,
  HX711_cs = 10
};

HX711_ADC LoadCell(HX711_dt, HX711_sck);

void setup() {
  Serial.begin(57600);
  
  pinMode(Pinout::LEDRedPin, OUTPUT);
  pinMode(Pinout::LEDGreenPin, OUTPUT);
  pinMode(Pinout::LEDBluePin, OUTPUT);
  pinMode(Pinout::FusePin, OUTPUT);
  pinMode(Pinout::HX711_cs, OUTPUT);
  pinMode(Pinout::RCPin, INPUT);

  LEDTest();

  attachInterrupt(digitalPinToInterrupt(Pinout::RCPin), PulseTimer, CHANGE);

  LoadCell.begin();
  boolean _tare = true;
  unsigned long stabilizingtime = 5000;
  LoadCell.start(stabilizingtime, _tare);
  LoadCell.setCalFactor(213.11);  //LoadCell CalFactor is found by calibrating the scale with an example program

  if (SD.begin(Pinout::HX711_cs)) {  // initialize the SD card
    Serial.println("SD card initialized");
    Green();
  } else {
    Red();
    sdBeginSuccess = false;
    Serial.println("SD card failed to initialize");
    while (1)
      ;
  }

  if (SD.exists("Data.txt")) {  // check if any previous data.txt file exists
    SD.remove("Data.txt"); // removes any previous data.txt file
  }
}

void loop() {
  if (sdBeginSuccess) {
    data = SD.open("Data.txt", FILE_WRITE); // create data.txt file and open it to write
    if (data) {
      while (timeCount <= 40) {
        if (Pulses < 2000) {
          PulseWidth = Pulses;
        }
        if (PulseWidth > 1500) { // if pulseWidth is above 1500 micro seconds, it pauses the countdown
          if (timeCount == 0) {
            Green();
          } else {
            Red();
          }
        } else {
          timeCount = timeCount + .1; // time incremental
          delay(100);
          if (timeCount < 5) {
            if (timeCount - int(timeCount) < 0.5) {
              Red();
            } else {
              ClearLEDs();
            }
          }
          if (timeCount >= 5 && timeCount < 10) {
            if (timeCount - int(timeCount) < 0.25) {
              Red();
              Blue();
              Green();
            } else {
              if (timeCount - int(timeCount) >= 0.5 && timeCount - int(timeCount) < 0.75) {
                Red();
                Blue();
                Green();
              } else {
                ClearLEDs();
              }
            }
          }
          if (timeCount >= 10) {
            if (timeCount > 10 && timeCount < 11) {
              digitalWrite(Pinout::FusePin, HIGH); // ignition start
            }
            if (timeCount >= 12) {
              digitalWrite(Pinout::FusePin, LOW); // stopping ignition 
            }
            LoadCell.update(); //update load cell value
            loadCellData = LoadCell.getData(); // gets load cell data
            data.print(timeCount); // prints the timestamp to the data.txt file
            data.print("/"); // prints a "/" as a separator for timestamp/load columns in the data.txt file
            data.println(loadCellData); // prints the load value to the data.txt file
          }
          if (timeCount > 40) { // after 40 seconds (so 30 seconds of measuring load)
            data.close();  // the program saves and closes the data.txt file
            Blue(); // the program turns on the blue LED to inform the user that data collection is finished
            sdBeginSuccess = false;
            return;
          }
        }
      }
    } else {
      sdBeginSuccess = false;
      Serial.println("Error: could not open SD card");
      Red();
      return;
    }
  }
}

void PulseTimer() {
  CurrentTime = micros();
  if (CurrentTime > StartTime) {
    Pulses = CurrentTime - StartTime;
    StartTime = CurrentTime;
  }
}

void LEDTest() {
  digitalWrite(Pinout::LEDRedPin, HIGH);
  digitalWrite(Pinout::LEDGreenPin, HIGH);
  digitalWrite(Pinout::LEDBluePin, HIGH);
  delay(1500);
  digitalWrite(Pinout::LEDRedPin, LOW);
  digitalWrite(Pinout::LEDGreenPin, LOW);
  digitalWrite(Pinout::LEDBluePin, LOW);
}

void ClearLEDs() {
  digitalWrite(Pinout::LEDRedPin, LOW);
  digitalWrite(Pinout::LEDGreenPin, LOW);
  digitalWrite(Pinout::LEDBluePin, LOW);
}

void Red() {
  digitalWrite(Pinout::LEDRedPin, HIGH);
}

void Blue() {
  digitalWrite(Pinout::LEDBluePin, HIGH);
}

void Green() {
  digitalWrite(Pinout::LEDGreenPin, HIGH);
}
