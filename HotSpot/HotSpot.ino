#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define PHANT_URL "http://iot.poul.org"

#define API_URL "api.telegram.org"
#define API_AUTH "bot238708260:AAHTgqFIn0o4tMDRktEqFa3TwPWf27XJINg"

#define SSID "hackathon-iot"
#define PASSWORD "polifactory2016"

#define TELEGRAM_SHA1 "C65BFA5BF7570C6A0285C16FA7196C3632B42821"
#define PHANT_SHA1 "9F6B3C62037BBF99BCEDA0CB96CFF38A01024584"

#define API_URL_AUTH "https://" API_URL "/" API_AUTH "/"

#define PHANT_PUBKEY "VXGz42p42GiyAdDLWgKgCzKE2Np"
#define PHANT_PRIVKEY "7nAlZOgZOASAaVqjxgMgT9rD8pw"

#define KEY1_PIN D1
#define KEY2_PIN D2
#define KEY3_PIN D3
#define LOCK_PIN D4

#define TEMP_PIN D5
#define RLED_PIN D6
#define GLED_PIN D7
#define SERVO_PIN D8

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

void sendRequest(const char *request, const char *fingerprint)
{
  HTTPClient http;
  int httpCode;

  Serial.print("HTTP GET ");
  Serial.println(request);

  if (fingerprint[0] == '\0')
    http.begin(request);
  else
    http.begin(request, fingerprint);
  httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print(httpCode);
    Serial.print(": ");
    Serial.println(http.getString());  
  }
  else {
    Serial.print("HTTP GET... failed: ");
    Serial.println(http.errorToString(httpCode));
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(KEY1_PIN, INPUT_PULLUP);
  pinMode(KEY2_PIN, INPUT_PULLUP);
  pinMode(KEY3_PIN, INPUT); //On-board pullup
  pinMode(LOCK_PIN, INPUT); //On-board pullup
  pinMode(TEMP_PIN, INPUT);
  pinMode(RLED_PIN, INPUT_PULLUP);
  pinMode(GLED_PIN, INPUT_PULLUP);
  pinMode(SERVO_PIN, OUTPUT); //On-board pulldown

  digitalWrite(LED_BUILTIN, HIGH);

  sensors.begin();

  Serial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connessione in corso");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" Connesso!");
}

void loop() {
  char request[200];
  const unsigned int chat_id = 33972702;
  const unsigned int foodCode = random(0, 4) * 1000 + random(0, 4) * 100 + random(0, 4) * 10 + random(0, 4);  //FIXME
  const char *latitude = "45.506221";
  const char *longitude = "9.166277";
  bool hotDish = true;
  double temperature;
  char temperatureStr[6];

  sensors.requestTemperatures();  //Blocking!!
  temperature = sensors.getTempCByIndex(0);
  dtostrf(temperature, 0, 1, temperatureStr);

  sprintf(request, "%s/input/%s?private_key=%s&temperature=%s", PHANT_URL, PHANT_PUBKEY, PHANT_PRIVKEY, temperatureStr);
  sendRequest(request, "");

  if (temperature >= 30)  //It could have been done in better ways...
    hotDish = true;
  else
    hotDish = false;

  sprintf(request, "%ssendMessage?chat_id=%d&text=Your+food+is+available+here.+The+code+for+accessing+the+food+is+%d.%s", API_URL_AUTH, chat_id, foodCode, hotDish ? "+Hurry+up%2C+hot+dish%21" : "");
  sendRequest(request, TELEGRAM_SHA1);

  sprintf(request, "%ssendLocation?chat_id=%d&latitude=%s&longitude=%s", API_URL_AUTH, chat_id, latitude, longitude);
  sendRequest(request, TELEGRAM_SHA1);

  delay(30000);
}
