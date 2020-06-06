/******************************************
   ME216M Final Project: Distant Socializing
   By: Cesar Arevalo & Kim Ngo
   Last Updated: 5/26/20
   Inputs:light sensor, 4x buttons
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
// GetClient and PostClient access the same feed, therefore have the same information
HttpClient GetClient = HttpClient(wifi, io_host, io_port);
HttpClient PostClient = HttpClient(wifi, io_host, io_port);

// Strings to GET and POST
String phrase;
String getMessage;
String postMessage;
String messages[7];
int phraseNum;
unsigned long AWAITING_MESSAGE_DURATION = 10000; // 10 seconds

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

#define LIGHT_SENSOR       A0 // put sensor on an analog pin
#define LEFT_BUTTON             2
#define CENTER_BUTTON           4
#define RIGHT_BUTTON            6
#define SELECT_BUTTON          16


#define LIGHT_THRESHOLD_LOW 350
#define LIGHT_THRESHOLD_HIGH 400
unsigned long WAVE_DURATION = 200;
unsigned long BUTTON_DURATION = 100;
unsigned long waveTimer;
unsigned long buttonTimer;
unsigned long messageTimer;

//Events Definitions
#define EVENT_LIGHT   EventManager::kEventUser0
#define EVENT_BUTTON  EventManager::kEventUser1
#define EVENT_MESSAGE EventManager::kEventUser2
#define EVENT_SELECT  EventManager::kEventUser3
#define EVENT_DECODED EventManager::kEventUser4

//Create the Event Manager
EventManager eventManager;

// Create the different states for the state machine
enum SystemState_t {INIT, RECEIVE, DECODE, SEND};

// Create and set a variable to store the current state
SystemState_t currentState = INIT;

void setup() {
  Serial.begin(9600); //initialize serial
  while (!Serial && millis() < 6000); // wait for serial to connect (6s timeout)
  setupWifi();
  //setupOLED();

  eventManager.addListener(EVENT_LIGHT, stateMachine);
  eventManager.addListener(EVENT_BUTTON, stateMachine);
  eventManager.addListener(EVENT_MESSAGE, stateMachine);
  eventManager.addListener(EVENT_SELECT, stateMachine);
  eventManager.addListener(EVENT_DECODED, stateMachine);

  while (!Serial); //wait for serial to connect

  stateMachine(INIT, 0);
  waveTimer = millis();
  buttonTimer = millis();
  messageTimer = millis();
}

void loop() {
  eventManager.processEvent();
  checkEvents();
}

void buildPhrases() {
  String phraseOne    = "I miss you!";
  String phraseTwo    = "How are you?";
  String phraseThree  = "I'm doing well!";
  String phraseFour   = "Ok!";
  String phraseFive   = "Message 1";
  String phraseSix    = "Message 2";
  String phraseSeven  = "Message 3";

  messages[0] = phraseOne;
  messages[1] = phraseTwo;
  messages[2] = phraseThree;
  messages[3] = phraseFour;
  messages[4] = phraseFive;
  messages[5] = phraseSix;
  messages[6] = phraseSeven;
}

void checkEvents() {
  checkButtons();
  checkSelect();
  if (millis() - waveTimer > WAVE_DURATION) {
    checkLight();
    waveTimer = millis(); // reset timer
  }
  if (millis() - messageTimer > AWAITING_MESSAGE_DURATION) {
    checkMessages();
    messageTimer = millis(); // reset timer
  }
}

void checkSelect() {
  bool eventHappened = false;  // Initialize our eventHappened flag to false
  int eventParameter = 0;       // Change type as needed

  // Create variable to hold last button pin reading
  // note: static variables are only initialized once and keep their value between function calls.
  static int lastSelectButton = LOW;

  // Read value of button pin
  int thisSelectButton = digitalRead(SELECT_BUTTON);

  // Check if button was pressed
  if (thisSelectButton != lastSelectButton) {
    if (thisSelectButton == HIGH) {
      // Set flag so event will be posted
      eventHappened = true;
    }
  }

  // Post the event if it occured
  if (eventHappened == true) {
    eventManager.queueEvent(EVENT_SELECT, eventParameter);
  }

  // Update last button reading to current reading
  lastSelectButton = thisSelectButton;

}

void checkLight() {
  bool eventHappened = false;  // Initialize our eventHappened flag to false
  int eventParameter = 0;       // Change type as needed

  // Create variable to hold last pin reading
  // note: static variables are only initialized once and keep their value between function calls.
  static int lastLightValue = 0;   // variable to store the value coming from the sensor

  int thisLightValue = analogRead(LIGHT_SENSOR);

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

void checkButtons() {
  bool eventHappened = false;  // Initialize our eventHappened flag to false
  int eventParameter = 0;       // Change type as needed

  static int lastLeftButton = LOW;
  static int lastRightButton = LOW;
  static int lastCenterButton = LOW;

  buttonTimer = millis();
  int thisLeftButton   = digitalRead(LEFT_BUTTON);
  int thisRightButton  = digitalRead(RIGHT_BUTTON);
  int thisCenterButton = digitalRead(CENTER_BUTTON);

  while (millis() - buttonTimer < BUTTON_DURATION) {
    thisLeftButton   = digitalRead(LEFT_BUTTON);
    thisRightButton  = digitalRead(RIGHT_BUTTON);
    thisCenterButton = digitalRead(CENTER_BUTTON);
  }

  if (lastLeftButton == LOW && lastRightButton == LOW && lastCenterButton == LOW) {
    if (thisLeftButton == HIGH && thisRightButton == LOW && thisCenterButton == LOW) {
      eventHappened = true;
      eventParameter = 1;
    }
    if (thisRightButton == HIGH && thisLeftButton == LOW && thisCenterButton == LOW) {
      eventHappened = true;
      eventParameter = 2;
    }
    if (thisCenterButton == HIGH && thisRightButton == LOW && thisLeftButton == LOW) {
      eventHappened = true;
      eventParameter = 3;
    }
    if (thisRightButton == HIGH && thisLeftButton == HIGH && thisCenterButton == LOW) {
      eventHappened = true;
      eventParameter = 4;
    }
    if (thisRightButton == HIGH && thisCenterButton == HIGH && thisLeftButton == LOW) {
      eventHappened = true;
      eventParameter = 5;
    }
    if (thisLeftButton == HIGH && thisCenterButton == HIGH && thisRightButton == LOW) {
      eventHappened = true;
      eventParameter = 6;
    }
    if (thisLeftButton == HIGH && thisRightButton == HIGH && thisCenterButton == HIGH) {
      eventHappened = true;
      eventParameter = 7;
    }
  }

  if (eventHappened == true) {
    eventManager.queueEvent(EVENT_BUTTON, eventParameter);
  }

  lastRightButton = thisRightButton;
  lastLeftButton = thisLeftButton;
  lastCenterButton = thisCenterButton;
}

/* CheckMessage
    Gets data from feed once and checks if the message is from the other person.
*/
void checkMessages() {
  bool eventHappened = false;  // Initialize our eventHappened flag to false
  int eventParameter = 0;       // Change type as needed

  // Create variable to hold last message
  // note: static variables are only initialized once and keep their value between function calls.
  static String lastMessage = "";   // variable to store the most recent message coming from the feed

  getMessage = GetData();
  if (lastMessage != getMessage && getMessage.indexOf(your_name) == -1) {
    eventHappened = true;
  }

  if (eventHappened == true) {
    Serial.println("New Message received");
    eventManager.queueEvent(EVENT_MESSAGE, eventParameter);
  }

  lastMessage = getMessage;
}

void getPhraseNum(String message) {
  String testMessage = message.substring(message.indexOf(":"));
  for (int i = 0; i < 7; i++) {
    if (messages[i] == testMessage) {
      phraseNum = i;
      break;
    }
  }
}

void decodePhrase(int param){
  bool eventHappened = false;
  int eventParameter = 0;

  if(param == phraseNum){
    eventHappened = true;
  } else {
    Serial.println("Try again!");
  }

  if(eventHappened == true){
    eventManager.queueEvent(EVENT_DECODED, eventParameter);
  }
  
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

/* GetData
    Sends a get request to the adafruit feed server that is not from the original user and returns getMessage.
    @param none
    @return String of getMessage
*/
String GetData() {
  String output = "";

  // Make sure we're connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {

    // Create a GET request to the getMessage path
    GetClient.get(get_io_feed_path);
    Serial.println("[HTTP] GET... getMessage Requested");

    // read the status code and body of the response
    int statusCode = GetClient.responseStatusCode();
    String response = GetClient.responseBody();
    // CSV string in value,lat,lon,ele format. The lat, lon, and ele values are left blank since they are not set.
    // i.e. "string",0.0,0.0,0.0

    // Check for the OK from the Server
    if (statusCode == 200) {
      Serial.println("[HTTP] GET received reply!");

      String getMessage_string = response.substring(0, response.length() - 4);
      output = getMessage_string;

    } else if (statusCode > 0) {
      // Server issue
      Serial.print("[HTTP] GET... Received response code: ");
      Serial.println(statusCode);
    } else {
      // Client issue
      Serial.print("[HTTP] GET... Failed, error code: ");
      Serial.println(statusCode);
    }
  } else {
    Serial.println("[WIFI] Not connected"); // loop to reconnect
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(wifi_ssid);

      // Connect to WPA/WPA2 network. SSID and password are set in config.h
      WiFi.begin(wifi_ssid, wifi_password);


      // wait 10 seconds for connection:
      delay(10000);
    }
    digitalWrite(LED_BUILTIN, LOW); // turn off light once connected
    Serial.println("WiFi successfully connected");
  }

  return output;
}

/* PostData
    Sends a post request to the Adafruit IO server.
    @param <type> myMessage
*/
void PostData(String myMessage) {
  // Make sure we're connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("[POST] Creating request with value: " + myMessage);
    PostClient.beginRequest();
    PostClient.post(post_io_feed_path2);
    PostClient.sendHeader("X-AIO-Key", io_key2);

    // Tell the server that the type of content we're sending is JSON
    PostClient.sendHeader("Content-Type", "application/json"); // fill in the header type to specify JSON data

    // Format myMessage to JSON
    DynamicJsonDocument doc(500);          // create object with arbitrary size 500
    doc["value"] = myMessage;              // set { "value" : myMessage }
    String formatted_data;
    serializeJson(doc, formatted_data);    // save JSON-formatted String to formatted_data
    PostClient.sendHeader("Content-Length", formatted_data.length()); // fill in the header type to send the length of data

    // Post data, along with headers
    PostClient.beginBody();
    PostClient.print(formatted_data);
    PostClient.endRequest();

    // Handle response from Server
    int statusCode = PostClient.responseStatusCode();
    String response = PostClient.responseBody();
    if (statusCode == 200) {
      // Response indicated OK
      Serial.println("[HTTP] POST was successful!");
    } else if (statusCode > 0) {
      // Server response received
      Serial.print("[HTTP] POST... Received response code: ");
      Serial.println(statusCode);
    } else {
      // httpCode will be negative on Client error
      Serial.print("[HTTP] POST... Failed, error: ");
      Serial.println(statusCode);
    }
  } else {
    Serial.println("[WIFI] Not connected");
  }
}

void stateMachine(int event, int param) {
  SystemState_t nextState = currentState;
  switch (currentState) {
    case INIT:
      pinMode(LEFT_BUTTON, INPUT);
      pinMode(RIGHT_BUTTON, INPUT);
      pinMode(CENTER_BUTTON, INPUT);
      pinMode(SELECT_BUTTON, INPUT);
      pinMode(LIGHT_SENSOR, INPUT);
      buildPhrases();
      nextState = RECEIVE;
      break;

    case RECEIVE:
      if (event == EVENT_MESSAGE) {
        Serial.println(getMessage);
        getPhraseNum(getMessage);
        //Serial.println("Try decoding message!");
        //nextState = DECODE;
      }
      if (event == EVENT_SELECT) { //select
        Serial.println("Ready for message input");
        nextState = SEND;
      }
      break;

    case DECODE:
      if(event == EVENT_BUTTON){
        decodePhrase(param);
      }
      if(event == EVENT_DECODED) {
        Serial.println("Message Decoded!");
        Serial.println(getMessage);
      }
      break;

    case SEND:
      if (event == EVENT_BUTTON) { // confirm sending
        phrase = messages[param - 1];
        Serial.print("Message to send: ");
        Serial.println(phrase);
      }
      if (event == EVENT_SELECT) {
        postMessage = your_name + ": " + phrase;
        PostData(postMessage);
        nextState = RECEIVE;
      }
      break;

    default:
      break;
  }
  currentState = nextState;
}
