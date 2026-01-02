/* The source Code from : https://github.com/riyadhasan24
 * By Md. Riyad Hasan
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const uint8_t STATION_ID = 2;   // <-- set 1 for Receiver-1, set 2 for Receiver-2

const char* ssid = "Power Note";
const char* pass = "12345678";

// ESP32 AP default IP
const char* serverHost = "192.168.4.1";

LiquidCrystal_I2C lcd(0x25, 16, 2);

const uint8_t buzzerPin = D6;
const uint8_t buttonPin = D5;

unsigned long lastPollMs = 0;
const unsigned long pollIntervalMs = 1200;

uint32_t lastVersion = 0;

bool alarmOn = false;

bool lastButton = HIGH;
unsigned long pressStartMs = 0;
const unsigned long longPressMs = 900;

void buzzerOn() 
{
  digitalWrite(buzzerPin, HIGH);
  alarmOn = true;
}

void buzzerOff() 
{
  digitalWrite(buzzerPin, LOW);
  alarmOn = false;
}

bool httpGet(String url, String &out) 
{
  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, url)) return false;
  int code = http.GET();
  if (code > 0) out = http.getString();
  http.end();

  return (code == 200);
}

bool parseMwAndV(const String &json, int &mw, uint32_t &v) 
{
  int mwIdx = json.indexOf("\"mw\":");
  int vIdx  = json.indexOf("\"v\":");
  if (mwIdx < 0 || vIdx < 0) return false;

  mwIdx += 5; 
  vIdx  += 4;

  int mwEnd = json.indexOf(",", mwIdx);
  if (mwEnd < 0) mwEnd = json.indexOf("}", mwIdx);
  mw = json.substring(mwIdx, mwEnd).toInt();

  int vEnd = json.indexOf(",", vIdx);
  if (vEnd < 0) vEnd = json.indexOf("}", vIdx);
  v = (uint32_t) json.substring(vIdx, vEnd).toInt();

  return true;
}

void showData(int mw, uint32_t v) 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Station_");
  lcd.print(STATION_ID);

  lcd.setCursor(0, 1);
  lcd.print("Load: ");
  lcd.print(mw);
  lcd.print("MW");
}

void sendAck() 
{
  String resp;
  String url = String("http://") + serverHost + "/ack?st=" + String(STATION_ID);
  httpGet(url, resp); // ignore response; it's ok
}

void connectWifi() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  lcd.setCursor(0, 1);
  lcd.print("Power Note");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) 
  {
    delay(200);
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) 
  {
    lcd.setCursor(0, 0);
    lcd.print("Connected");
    lcd.setCursor(0, 1);
    lcd.print("Polling server");
  } 
  else 
  {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Retrying...");
  }
}

void setup() 
{
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  buzzerOff();

  Wire.begin();
  lcd.init();
  lcd.backlight();

  connectWifi();
}

void loop() 
{
  // If WiFi drops, reconnect
  if (WiFi.status() != WL_CONNECTED) 
  {
    connectWifi();
    delay(500);
    return;
  }

  // Poll for new data
  if (millis() - lastPollMs >= pollIntervalMs) 
  {
    lastPollMs = millis();

    String resp;
    String url = String("http://") + serverHost + "/get?st=" + String(STATION_ID);
    if (httpGet(url, resp)) 
    {
      int mw = 0;
      uint32_t v = 0;
      if (parseMwAndV(resp, mw, v)) 
      {
        if (v != lastVersion) 
        {
          lastVersion = v;
          showData(mw, v);
          buzzerOn(); // new update alarm
        }
      }
    }
  }

  bool nowButton = digitalRead(buttonPin); // LOW = pressed
  if (lastButton == HIGH && nowButton == LOW) 
  {
    pressStartMs = millis();
  }

  if (lastButton == LOW && nowButton == HIGH) 
  {
    pressStartMs = 0;
  }

  if (nowButton == LOW && pressStartMs > 0 && (millis() - pressStartMs >= longPressMs)) 
  {
    pressStartMs = 0;

    buzzerOff();
    sendAck();

    lcd.setCursor(0, 0);
    lcd.print("Station_");
    lcd.print(STATION_ID);
    lcd.print(" ACK  ");
  }

  lastButton = nowButton;
}
