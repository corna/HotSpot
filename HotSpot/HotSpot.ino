#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define API_URL "api.telegram.org"
#define API_AUTH "bot238708260:AAHTgqFIn0o4tMDRktEqFa3TwPWf27XJINg"

#define SSID "hackathon-iot"
#define PASSWORD "polifactory2016"

#define SHA1FINGERPRINT "C65BFA5BF7570C6A0285C16FA7196C3632B42821"

#define API_URL_AUTH "https://" API_URL "/" API_AUTH "/"

#define KEY_1_PIN D1
#define KEY_2_PIN D2
#define KEY_3_PIN D3

#define LOCK_PIN D4
#define SERVO_PIN D5
#define RLED_PIN D6
#define GLED_PIN D7

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

void setup() {
  pinMode(KEY1_PIN, INPUT_PULLUP);
  pinMode(KEY2_PIN, INPUT_PULLUP);
  pinMode(KEY3_PIN, INPUT); //On-board pullup
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

  delay(30000);
}