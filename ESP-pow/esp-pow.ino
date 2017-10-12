#include <HLW8012.h> //https://bitbucket.org/xoseperez/hlw8012
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
//for LED status
#include <Ticker.h>
#include <BlynkSimpleEsp8266.h>

Ticker ticker;

#define PIN_LED 3
#define PIN_BUTTON 0
#define PIN_RELAY 12
#define SEL_PIN  16
#define CF1_PIN  14
#define CF_PIN   13

#define LED_ON() digitalWrite(PIN_LED, HIGH)
#define LED_OFF() digitalWrite(PIN_LED, LOW)
#define LED_TOGGLE() digitalWrite(PIN_LED, digitalRead(PIN_LED) ^ 0x01)
#define RELAY_ON() digitalWrite(PIN_RELAY, HIGH)
#define RELAY_OFF() digitalWrite(PIN_RELAY, LOW)

char auth[] = "your-auth-in-blynk-app";
#define OTA_PASS "your-OTA-app"
#define OTA_PORT (8266)

// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where a
// the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define CURRENT_MODE                    HIGH

// These are the nominal values for the resistors in the circuit
#define CURRENT_RESISTOR                0.001
#define VOLTAGE_RESISTOR_UPSTREAM       ( 5 * 470000 ) // Real: 2280k
#define VOLTAGE_RESISTOR_DOWNSTREAM     ( 950 ) // Real 1.009k

HLW8012 hlw8012;

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void hlw8012_cf1_interrupt() {
    hlw8012.cf1_interrupt();
}
void hlw8012_cf_interrupt() {
    hlw8012.cf_interrupt();
}

// Library expects an interrupt on both edges
void setInterrupts() {
    attachInterrupt(CF1_PIN, hlw8012_cf1_interrupt, CHANGE);
    attachInterrupt(CF_PIN, hlw8012_cf_interrupt, CHANGE);
}


bool longPress()
{
  static int lastPress = 0;
  if (millis() - lastPress > 3000 && digitalRead(PIN_BUTTON) == 0) {
    return true;
  } else if (digitalRead(PIN_BUTTON) == 1) {
    lastPress = millis();
  }
  return false;
}

void tick()
{
  //toggle state
  int state = digitalRead(PIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(PIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //set led pin as output
  pinMode(PIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.2, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);


  if (!wifiManager.autoConnect(("TUANPM_" + String(ESP.getChipId())).c_str())) {
    ESP.reset();
    delay(1000);
  }

  ticker.detach();


  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.begin();
  LED_ON();
  RELAY_ON();
  Blynk.config(auth);
  hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);
  hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
  setInterrupts();
}


bool in_smartconfig = false;
void enter_smartconfig()
{
  if (in_smartconfig == false) {
    in_smartconfig = true;
    ticker.attach(0.5, tick);
    WiFi.beginSmartConfig();
  }
}

void exit_smart()
{
  ticker.detach();
  LED_ON();
  in_smartconfig = false;
}
bool relay_on = false;
int last_check = 0, vol = 0;
void loop() {
  // put your main code here, to run repeatedly:

  if (longPress()) {
    enter_smartconfig();
  }
  if (WiFi.status() == WL_CONNECTED && in_smartconfig && WiFi.smartConfigDone()) {
    exit_smart();
  }

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }
  if (millis() - last_check > 1000) {
    last_check = millis();
    Blynk.virtualWrite(V0, hlw8012.getVoltage());
    Blynk.virtualWrite(V1, hlw8012.getCurrent());
    Blynk.virtualWrite(V2, hlw8012.getActivePower());
    Blynk.virtualWrite(V3, hlw8012.getApparentPower());
    Blynk.virtualWrite(V4, 100 * hlw8012.getPowerFactor());
  }
  Blynk.run();

}
