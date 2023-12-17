#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include "IRremote.h"

// Define the servo pin
const int servoPin = 9; // Replace with your actual servo pin
const int receiver = 7;

RTC_DS3231 rtc;
Servo myservo;
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

// Define the scheduled feeding times
struct FeedingTime {
  uint8_t hour;
  uint8_t minute;
  bool foodDispensed;
};

// Add your desired feeding times here
FeedingTime feedingTimes[] = {
  {12, 30, false},  // 12:30 PM
  {18, 0, false},   // 6:00 PM
};

const int numFeedingTimes = sizeof(feedingTimes) / sizeof(feedingTimes[0]);

void setup() {
  Wire.begin();
  rtc.begin();
  myservo.attach(servoPin);
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  irrecv.enableIRIn();
}

void loop() {
  feedByTime();
  feedByRemote();
}

void feedByTime() {
  DateTime now = rtc.now();

  for (int i = 0; i < numFeedingTimes; i++) {
    if (!feedingTimes[i].foodDispensed && now.hour() == feedingTimes[i].hour && now.minute() == feedingTimes[i].minute) {
      dispenseFood(5000);
      feedingTimes[i].foodDispensed = true;
      delay(1000); // Wait to avoid multiple dispenses in quick succession
    }
  }
}

void feedByRemote() {
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    if(results.value == 0xFF30CF) {
      dispenseFood(1000);
    } else if(results.value == 0xFF52AD) {
      dispenseFood(3000);
    }
    irrecv.resume(); // receive the next value
  }
}

void dispenseFood(int d) {
  myservo.write(0); // Open the food dispenser
  delay(d); // Wait for food to be dispensed (adjust as needed)
  myservo.write(90); // Close the food dispenser
}
