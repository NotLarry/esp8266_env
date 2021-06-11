/*********
 *********/
//#include "ESP8266WiFi.h"
//#include "ESP8266WiFiMulti.h"
#include "ESP8266HTTPClient.h"
#include "WiFiClientSecure.h"
//#include "WiFiClientSecureAxTLS.h"
#include "sekrits.h"
#include "Adafruit_MCP9808.h"
#include "Adafruit_SSD1306.h"
#include "ESP8266WiFi.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
WiFiClientSecure client;
//#include "discord.h"
const String discord_webhook = SEKRIT_WEBHOOK;
const String discord_tts = SEKRIT_TTS;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

const char* timeish = "10:33:43";
// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Wifi settings
const char* ssid = SEKRIT_SSID;
const char* password = SEKRIT_PASS;

const long utcOffsetInSeconds = UTC_OFFSETINSECONDS;


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {  // Let us setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  Serial.begin(115200);
  if (!tempsensor.begin()) 
  {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Start Wifi and wait until we're connected
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  displaytext();    // Draw scrolling text
timeClient.begin();

  // Set GPIO 5 as input
  //pinMode(5, INPUT);
  uint8_t GPIO_Pin = D6;

attachInterrupt(digitalPinToInterrupt(GPIO_Pin), IntCallback, RISING);
  sendDiscord("Hello World!");

}  // End of void setup() don't get lost...


void loop() {
 // Read and print out the temperature, then convert to *F
  delay(250);
 Serial.println(timeClient.getFormattedTime());
  tempsensor.shutdown_wake(1);
  delay(2000);
  tempsensor.shutdown_wake(0);

}


void displaytext(void) {
  for (;;) {
  float c = tempsensor.readTempC();
  float f = c * 9.0 / 5.0 + 32;
timeClient.update(); 

delay(1000);
  display.clearDisplay();
//  float c = tempsensor.readTempC();
//  float f = tempsensor.readTempF();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 3
  );
  display.println(timeClient.getFormattedTime());
  display.println(String(f)+ "F");
  display.display();      // Show initial text
  delay(100);

}



}

void IntCallback(){
 Serial.print("Stamp(ms): ");
 Serial.println(millis());
}


void sendDiscord(String content) {
  HTTPClient https;
  Serial.println("[HTTP] Connecting to Discord...");
  Serial.println("[HTTP] Message: " + content);
  Serial.println("[HTTP] TTS: " + discord_tts);
  if (https.begin(client, discord_webhook)) {  // HTTPS
    // start connection and send HTTP header
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST("{\"content\":\"" + content + "\",\"tts\":" + discord_tts + "}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.print("[HTTP] Status code: ");
      Serial.println(httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.print("[HTTP] Response: ");
        Serial.println(payload);
      }
    } else {
      Serial.print("[HTTP] Post... failed, error: ");
      Serial.println(https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTP] Unable to connect\n");
  }

  // End extra scoping block
}
