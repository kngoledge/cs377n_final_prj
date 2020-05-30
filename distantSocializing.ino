/******************************************
   ME216M Final Project: Distant Socializing
   By: Cesar Arevalo & Kim Ngo
   Last Updated: 5/26/20
   Inputs: ultrasonic sensor, light sensor,
   4x buttons
   Outputs: OLED Screen, LED array
   Summary: This program monitors and responds
   to events.
 ******************************************/

//---------------------------------
//         Libraries
//---------------------------------
//Time Library
#include <TimeLib.h>

//Ultrasonic Library
#include "Ultrasonic.h"

//Event Manager Library
#include <EventManager.h>

// OLED Libraries
#include <U8g2lib.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Wireless Libraries
#include <ArduinoJson.h>
#include "config.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

//---------------------------------
//         Wireless Setup
//---------------------------------
WiFiSSLClient wifi;
//HttpClient GetClient = HttpClient(wifi, ow_host, ow_port);

//---------------------------------
//         OLED Variables
//---------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

//--------------------------
//  Event & State Variables
//--------------------------

#define ULTRASONIC_SENSOR_PIN 6 // pin on which connector is placed NOTE: digital
#define LIGHT_SENSOR_PIN A0 // put sensor on an analog pin

#define LIGHT_THRESHOLD_LOW 350
#define LIGHT_THRESHOLD_HIGH 400
unsigned long WAVE_DURATION = 200;
unsigned long waveTimer;


//Ultrasonic Sensor Setup
Ultrasonic sensor(ULTRASONIC_SENSOR_PIN);

//Events Defitions
#define EVENT_ULTRA EventManager::kEventUser0
#define EVENT_LIGHT EventManager::kEventUser1

//Create the Event Manager
EventManager eventManager;

// Create the different states for the state machine
enum SystemState_t {INIT, SENSING_INPUT, SLEEP};

// Create and set a variable to store the current state
SystemState_t currentState = INIT;

void setup() {
  Serial.begin(9600); //initialize serial
  //setupWifi();
  //setupOLED();

  eventManager.addListener(EVENT_ULTRA, stateMachine);
  eventManager.addListener(EVENT_LIGHT, stateMachine);

  while (!Serial); //wait for serial to connect

  stateMachine(INIT, 0);
  waveTimer = millis();
}

void loop() {
  eventManager.processEvent();
  checkEvents();
}

void checkEvents() {
  if (millis() - waveTimer > WAVE_DURATION) {
    checkLight();
    waveTimer = millis(); // reset timer
  }
}

void checkLight() {
  bool eventHappened = false;  // Initialize our eventHappened flag to false
  int eventParameter = 0;       // Change type as needed

  // Create variable to hold last pin reading
  // note: static variables are only initialized once and keep their value between function calls.
  static int lastLightValue = 0;   // variable to store the value coming from the sensor

  int thisLightValue = analogRead(LIGHT_SENSOR_PIN);

  if (lastLightValue < LIGHT_THRESHOLD_LOW && lastLightValue != 0) {
    if (thisLightValue > LIGHT_THRESHOLD_HIGH ) {
      eventHappened = true;
    }
  }

  if (lastLightValue > LIGHT_THRESHOLD_HIGH) {
    if (thisLightValue < LIGHT_THRESHOLD_LOW ) {
      eventHappened = true;
    }
  }

  if (eventHappened == true) {
    Serial.println("Switch Triggered");
    eventManager.queueEvent(EVENT_LIGHT, eventParameter);
  }

//  Serial.print("Last:");
//  Serial.println(lastLightValue);
//  Serial.print("This:");
//  Serial.println(thisLightValue);

  lastLightValue = thisLightValue;
}

void setupWifi() {
  while (!Serial && millis() < 6000); // wait for serial to connect (6s timeout)

  // Light LED while trying to connect
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true); // don't continue
  }

  // Check for firmware version
  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifi_ssid);

    // Connect to WPA/WPA2 network. SSID and password are set in config.h
    WiFi.begin(wifi_ssid, wifi_password);
    /*** Use the line below instead if no password required! ***/
    // WiFi.begin(wifi_ssid);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("WiFi successfully connected");
  digitalWrite(LED_BUILTIN, LOW); // turn off light once connected
}

void setupOLED() {
  oled.begin();

  // Clear the buffer and configure font/color
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.setFontRefHeightExtendedText();
  oled.setDrawColor(1);
  oled.setFontPosTop();
  oled.setFontDirection(0);
  oled.sendBuffer();
}

void stateMachine(int event, int param) {
  SystemState_t nextState = currentState;
  switch (currentState) {
    case INIT:
      break;

    case SENSING_INPUT:
      break;

    case SLEEP:
      break;

    default:
      break;
  }
  currentState = nextState;
}
