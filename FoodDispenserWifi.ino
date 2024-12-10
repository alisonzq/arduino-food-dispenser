#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME "Goldie Dispenser"
#define BLYNK_AUTH_TOKEN ""

#define BLYNK_PRINT Serial
#define RECEIVER_PIN D3

#include <Stepper.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <RTClib.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
int sliderTreatNumber = 0;

char ssid[] = "";
char pass[] = "";

Stepper myStepper(stepsPerRevolution, D5, D6, D7, D8);

RTC_DS3231 rtc;

IRrecv irrecv(RECEIVER_PIN);
decode_results results;

BLYNK_WRITE(V0)
{
  sliderTreatNumber = param.asInt();
}

BLYNK_WRITE(V1)
{
  if(param.asInt()==1) {
    moveStepper(sliderTreatNumber);
  }
}

struct FeedingTime {
  uint8_t hour;
  uint8_t minute;
  bool foodDispensed;
};

FeedingTime feedingTimes[] = {
  {12, 0, false},  // 12:30 PM
  {18, 0, false},   // 6:00 PM
};

const int numFeedingTimes = sizeof(feedingTimes) / sizeof(feedingTimes[0]);

void setup()
{
  Serial.begin(115200);

  Wire.begin();
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  myStepper.setSpeed(15);

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  irrecv.enableIRIn();

}

void loop()
{
  Blynk.run();

  feedByTime();
  feedByRemote();
}

void moveStepper(int numOfTreats) {
  float anglePerStep = 360.0 / stepsPerRevolution; // Calculate angle per step as float
  int steps25degrees = (int)(25 / anglePerStep);  // Calculate steps needed
  int steps = numOfTreats * steps25degrees;
  myStepper.step(steps);                          // Move stepper
}


void feedByTime() {
  DateTime now = rtc.now();
  for (int i = 0; i < numFeedingTimes; i++) {
    if (!feedingTimes[i].foodDispensed && now.hour() == feedingTimes[i].hour && now.minute() == feedingTimes[i].minute) {
      moveStepper(16);
      feedingTimes[i].foodDispensed = true;
    }
  }
}

void feedByRemote() {
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    Serial.print("Received IR signal: ");
    Serial.println(resultToHexidecimal(&results));
    
    if(results.value == 0xFF30CF) {
      moveStepper(1);
    } else if(results.value == 0xFF52AD) {
      moveStepper(16);
    }
    irrecv.resume(); // receive the next value
  }
}



