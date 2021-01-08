#include <WiFiNINA.h>
#include <WS2812FX.h>

#include "utility/wifi_drv.h"

#define LED_COUNT 16
#define LED_PIN 6

#define STATUS_AVAILABLE 0
#define STATUS_BUSY 1
#define STATUS_INTERUPTIBLE 2
#define STATUS_OFF 3
#define STATUS_UNKNOWN 4

#define POLL_MILLIS 15000

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int cycle = 0;
char hostName[] = "10.0.0.134"; // pi or server address
int status = WL_IDLE_STATUS;
String inString = "";
int light_status = -1;
unsigned long poll_timer = 0;

WiFiClient client;

void setup() {
  Serial.println("Wifi connection");
  ws2812fx.init();
  set_connecting();
  ws2812fx.start();
  ws2812fx.service();
  resetConnection();
}

void resetConnection() {
  while ( status != WL_CONNECTED && status != WL_CONNECT_FAILED && status != WL_NO_SSID_AVAIL) {
    Serial.print("Attempting to connect to Wifi");

    // Connect to WPA/WPA2 network:
    status = WiFi.begin("", "");
    delay(5000);
  }
  Serial.println("connected!");
  printStatus();
}

void printStatus() {
  Serial.print("Wifi status: ");
  switch(status){
    case WL_IDLE_STATUS:
      Serial.println("Idle"); break;
    case WL_NO_SSID_AVAIL:
      Serial.println("SSID not found"); break;
    case WL_SCAN_COMPLETED:
      Serial.println("Scan complete"); break;
    case WL_CONNECTED:
      Serial.println("Connected"); break;
    case WL_CONNECT_FAILED:
      Serial.println("Conneciton Failed"); break;
    case WL_CONNECTION_LOST:
      Serial.println("Connection Lost"); break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected"); break;
    default:
      Serial.print("Unknown - ");
      Serial.println(status);
  }
}

void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();
  inString = "";

  if (status != WL_CONNECTED) {
    resetConnection();
  }

  // if there's a successful connection:
  if (client.connect(hostName, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.println("GET /status HTTP/1.1");
    client.println("Host: 10.0.0.134");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    while (!client.available()) {
      Serial.print(".");
      delay(10);
    }
    char prevchar1 = 0;
    char prevchar2 = 0;
    char prevchar3 = 0;
    bool inbody = false;
    while (client.available()) {
      char c = client.read();
      if (c == '\n' && prevchar1 == '\r' && prevchar2 == '\n' && prevchar3 == '\r'){
        inbody = true;
      }
      prevchar3 = prevchar2;
      prevchar2 = prevchar1;
      prevchar1 = c;
      if(inbody){
        if (isDigit(c)) {
          inString += c;
        }
      }
    }
    set_status(inString.toInt());
    Serial.print("Value: ");
    Serial.println(light_status);
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    status = WiFi.status();
    if(status == WL_CONNECTION_LOST) {
      WiFi.disconnect();
    }
    set_status(STATUS_UNKNOWN);
    printStatus();
  }

}

void loop() {
  unsigned long now = millis();
  if(now < poll_timer) {
    // overflow, just reset poll_timer to 0 and carry on (~once every 50 days)
    poll_timer = 0;
  }else if(now > poll_timer + POLL_MILLIS){
    // refresh
    poll_timer = now;
    Serial.println("fetching data");
    httpRequest();
  }
  ws2812fx.service();
}

void set_busy() {
  Serial.println("busy");
  ws2812fx.setBrightness(100);
  ws2812fx.setColor(RED);
  ws2812fx.setSpeed(10000);
  ws2812fx.setMode(15);
}

void set_available() {
  Serial.println("available");
  ws2812fx.setBrightness(80);
  ws2812fx.setColor(GREEN);
  ws2812fx.setSpeed(5000);
  ws2812fx.setMode(FX_MODE_STATIC);
}

void set_connecting() {
  Serial.println("connecting");
  ws2812fx.setBrightness(50);
  ws2812fx.setColor(BLUE);
  ws2812fx.setMode(FX_MODE_STATIC);
}


void set_interuptible() {
  Serial.println("interuptible");
  ws2812fx.setBrightness(100);
  ws2812fx.setColor(ORANGE);
  ws2812fx.setSpeed(1200);
  ws2812fx.setMode(FX_MODE_RUNNING_LIGHTS);
}

void set_unknown() {
  Serial.println("unknown");
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(200);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
}

void set_off() {
  Serial.println("off");
  ws2812fx.setBrightness(0);
  ws2812fx.setMode(FX_MODE_STATIC);
}

void set_status(int new_status) {
  if (light_status == new_status) {
    return;
  }
  light_status = new_status;
  switch(light_status){
    case STATUS_AVAILABLE:
      set_available();
      break;
    case STATUS_BUSY:
      set_busy();
      break;
    case STATUS_INTERUPTIBLE:
      set_interuptible();
      break;
    case STATUS_OFF:
      set_off();
      break;
    case STATUS_UNKNOWN:
      set_unknown();
      break;
    default:
      set_unknown();
  }
}
