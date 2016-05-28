#include <Ticker.h>
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

const int KEYPIN[3] = {KEY1_PIN,KEY2_PIN,KEY3_PIN};

const unsigned int chat_id = 33972702;
const char *latitude = "45.506221";
const char *longitude = "9.166277";

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

bool locked = false;
bool justLocked = false;
int currdigit = 0;
int secret[4] = {1,2,3,1};
int combination[4] = {0,0,0,0};

Ticker callAfterSecs;


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

void enableInterrupts()
{
  attachInterrupt(digitalPinToInterrupt(KEYPIN[0]), key1handler, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEYPIN[1]), key2handler, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEYPIN[2]), key3handler, FALLING);
}

void handleLock()
{
  locked = true;
  justLocked = true;
}

void handleButtons(int key)
{
  //Disable interrupts to avoid input bouncing
  detachInterrupt(digitalPinToInterrupt(KEYPIN[key]));
  //Complete debouncing by re-enabling interrupts
  callAfterSecs.once(0.5, enableInterrupts);

  Serial.printf("Digit %d = %d", currdigit, key+1);

  combination[currdigit] = key+1;
  Serial.printf("Saved key %d", combination[currdigit]);

  if(currdigit < 3) {
    currdigit++;
  }
  else {
    currdigit=0;
    if(checkSequence()) {
      locked = false;
      Serial.println("Unlocked!");
    }
  }
}

bool checkSequence()
{
  bool match = true;
  for(int digit = 0;digit < 4; digit++) {
    if(combination[digit] != secret[digit]) {
      match = false;
      Serial.println("Wrong combination");
      break;
    }
  }
  return match;
}

void key1handler(){ handleButtons(0); }
void key2handler(){ handleButtons(1); }
void key3handler(){ handleButtons(2); }

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

  attachInterrupt(digitalPinToInterrupt(LOCK_PIN), handleLock, FALLING);

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
  bool hotDish = true;
  double temperature;
  char temperatureStr[6];

  if (justLocked) {
    justLocked = false;
    
    sensors.requestTemperatures();  //Blocking!!
    temperature = sensors.getTempCByIndex(0);
    dtostrf(temperature, 0, 1, temperatureStr);

    sprintf(request, "%s/input/%s?private_key=%s&temperature=%s", PHANT_URL, PHANT_PUBKEY, PHANT_PRIVKEY, temperatureStr);
    sendRequest(request, "");

    if (temperature >= 30)  //It could have been done in better ways...
      hotDish = true;
    else
      hotDish = false;

    for (unsigned int i = 0; i < 4; i++)
      secret[i] = random(0, 3);

    sprintf(request, "%ssendMessage?chat_id=%d&text=Your+food+is+available+here.+The+code+for+accessing+the+food+is+%c%c%c%c.%s", API_URL_AUTH, chat_id, secret[0] + '0', secret[1] + '0', secret[2] + '0', secret[3] + '0', hotDish ? "+Hurry+up%2C+hot+dish%21" : "");
    sendRequest(request, TELEGRAM_SHA1);

    sprintf(request, "%ssendLocation?chat_id=%d&latitude=%s&longitude=%s", API_URL_AUTH, chat_id, latitude, longitude);
    sendRequest(request, TELEGRAM_SHA1);
  }

  if (!locked) {
    enableInterrupts();
    locked = true;
  }
}
