#include <Ticker.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define API_URL "api.telegram.org"
#define API_AUTH "bot238708260:AAHTgqFIn0o4tMDRktEqFa3TwPWf27XJINg"

#define SSID "hackathon-iot"
#define PASSWORD "polifactory2016"

#define SHA1FINGERPRINT "C65BFA5BF7570C6A0285C16FA7196C3632B42821"

#define API_URL_AUTH "https://" API_URL "/" API_AUTH "/"

const int KEYPIN[3] = {D1,D2,D3};

#define LOCK_PIN D4
#define SERVO_PIN D5
#define RLED_PIN D6
#define GLED_PIN D7

bool locked = false;
int currdigit = 0;
int secret[4] = {1,2,3,1};
int combination[4] = {0,0,0,0};

Ticker callAfterSecs;

void sendRequest(const char *request)
{
  HTTPClient http;
  int httpCode;

  Serial.print("HTTP GET ");
  Serial.println(request);

  http.begin(request, SHA1FINGERPRINT);
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
  pinMode(KEYPIN[0], INPUT_PULLUP);
  pinMode(KEYPIN[1], INPUT_PULLUP);
  pinMode(KEYPIN[2], INPUT); //On-board pullup
  pinMode(LOCK_PIN, INPUT); //On-board pullup
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(RLED_PIN, INPUT_PULLUP);
  pinMode(GLED_PIN, INPUT_PULLUP);

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

  sprintf(request, "%ssendMessage?chat_id=%d&text=Your+food+is+available+here.+The+code+for+accessing+the+food+is+%d.%s", API_URL_AUTH, chat_id, foodCode, hotDish ? "+Hurry+up%2C+hot+dish%21" : "");
  sendRequest(request);

  sprintf(request, "%ssendLocation?chat_id=%d&latitude=%s&longitude=%s", API_URL_AUTH, chat_id, latitude, longitude);
  sendRequest(request);


  enableInterrupts();
  locked = true;

  delay(30000);
}
