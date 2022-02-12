// Stark's TinyPico OnAir Light Device 
// todo list: Battery Code, Status page at root, OTA, MQTT, Deep Sleep, 

#include <Arduino.h>
#include <WiFi.h>
#include <TinyPICO.h>
#include <FastLED.h>

// Init the Wifi and web server
const char* WIFI_NAME= "starkhome";  
const char* WIFI_PASSWORD = "starkenburg";  
WiFiServer server(80);
String header;

// Init the TinyPICO library
TinyPICO tp = TinyPICO();
#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12
#define BAT_CHARGE 34
#define BAT_VOLTAGE 35
#define USB_PLUGGED 9
bool charge = false;
float volts = 100;
bool plugged = true;

// Init Neopixel Strip
#define NUM_LEDS 20
#define DATA_PIN 14
CRGB leds[NUM_LEDS];
String LED_STATE = "OFF";

void setup() {

  pinMode(9, INPUT_PULLUP);   // to read usb plugged in state

// setup FastLED Strip
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();  // clear all pixel data
  FastLED.show();

// Shutdown Onboard Dotstar
  tp.DotStar_SetPower( false );
 
// Serial Monitor begin and Wifi Connect
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(WIFI_NAME);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Successfully connected to WiFi network");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  //light one pixel to act like a power light
  leds[10].setRGB( 24, 24, 24 );
  FastLED.show(); 
}

void loop(){
  WiFiClient client = server.available();   
  if (client) {
    String current_data_line = "";               
    while (client.connected()) {
      if (client.available()) {   
        char new_byte = client.read();             
        Serial.write(new_byte);                    
        header += new_byte;
        if (new_byte == '\n') {       
          if (current_data_line.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            Serial.println("New Connection");
// LED=RED
            if (header.indexOf("LED=RED") >= 0 && header.indexOf("LED=RED") <= 20 ) {
              Serial.println("Call for RED LED");
              LED_STATE = "RED";              
              for(int dot = 0; dot < NUM_LEDS; dot++) { 
                leds[dot].setRGB( 64, 0, 0 );
              }
              FastLED.show(); 
              Serial.println("FastLed Strip RED");
// LED=BLUE
            } else if (header.indexOf("LED=BLUE") >= 0 && header.indexOf("LED=BLUE") <= 20 ) {
              Serial.println("Call for LED BLUE");
              LED_STATE = "BLUE";
              for(int dot = 0; dot < NUM_LEDS; dot++) { 
                leds[dot].setRGB( 0, 0, 64 );
              }
              FastLED.show(); 
              Serial.println("FastLed Strip BLUE");
// LED=GREEN
            } else if (header.indexOf("LED=GREEN") >= 0 && header.indexOf("LED=GREEN") <= 20 ) {
              Serial.println("Call for LED GREEN");
              LED_STATE = "GREEN";
              for(int dot = 0; dot < NUM_LEDS; dot++) { 
                leds[dot].setRGB( 0, 64, 0 );
              }
              FastLED.show(); 
              Serial.println("FastLed Strip GREEN");
// LED=OFF
            } else if (header.indexOf("LED=OFF") >= 0 && header.indexOf("LED=OFF") <= 20 ) {
              Serial.println("Call for LED OFF");
              LED_STATE = "off";
              for(int dot = 0; dot < NUM_LEDS; dot++) { 
                leds[dot].setRGB( 0, 0, 0 );
              }
              FastLED.show(); 
//light one pixel to act like a power light
              leds[10].setRGB( 24, 24, 24 );
  FastLED.show(); 
              Serial.println("FastLed cleared");
            } else {}

// Get Tinypico State
            charge = tp.IsChargingBattery();
            volts = tp.GetBatteryVoltage();
            plugged = digitalRead(9);

// Deliver HTML Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: 2px solid #4CAF50;; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");            
            client.println("</style></head>");
            client.println("<body><center><h1>Stark's OnAir Light</h1></center>");
            client.println("<center><h2>Press on button to turn on led and off button to turn off LED</h3></center><br><br>");            
            if (plugged == true) {
              client.println("USB is CONNECTED </p>");
              Serial.println("USB is CONNECTED");
              if (charge == true) {
                client.println("Battery is CHARGING </p>");
                Serial.println("Battery is CHARGING");
                } else {
                client.println("Battery is IDLE </p>");
                Serial.println("Battery is IDLE");
                }
              } else {
                client.println("USB is NOT CONNECTED </p>");
                Serial.println("USB is NOT CONNECTED");
                client.println("Battery is DISCHARGING </p>");
                Serial.println("Battery is DISCHARGING");
              }
            client.println("Current Voltage is roughly " + String(volts) + "</p>");
            Serial.println("Current Voltage is roughly " + String(volts) + "</p>");
            client.println("<form><center>");
            client.println("<p> LED is " + LED_STATE + "</p>");
            client.println("<center> <button class=\"button\" name=\"LED\" value=\"RED\" type=\"submit\">LED RED</button>");
            client.println("<button class=\"button\" name=\"LED\" value=\"BLUE\" type=\"submit\">LED BLUE</button>");
            client.println("<button class=\"button\" name=\"LED\" value=\"GREEN\" type=\"submit\">LED GREEN</button><br><br>");
            client.println("<button class=\"button\" name=\"LED\" value=\"OFF\" type=\"submit\">LED OFF</button><br><br>");
            client.println("</center></form></body></html>");
            client.println();
            Serial.println("Base Page Delivered"); 
            break;
          } 
          else 
          { 
            current_data_line = "";
          }
        } 
        else if (new_byte != '\r') 
        {  
          current_data_line += new_byte;     
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
