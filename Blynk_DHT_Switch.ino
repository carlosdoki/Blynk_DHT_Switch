#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""

// Projeto medir a temperatura e a umidade com DHT11
#include <DHT.h> //Inclui a biblioteca DHT Sensor Library
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define DHTPIN D2     // Pino digital 2 conectado ao DHT11
#define DHTTYPE DHT11 // DHT 11

char ssid[] = "";
char pass[] = "";

bool fetch_blynk_state = true; // true or false

#define RelayPin1 D5 // D1

#define SwitchPin1 D1 // SD3

#define wifiLed D0 // D0

// Change the virtual pins according the rooms
#define VPIN_BUTTON_1 V1

#define VPIN_BUTTON_C V5
#define VPIN_TEMPERATURE V6
#define VPIN_HUMIDITY V7

// Relay State
bool toggleState_1 = LOW; // Define integer to remember the toggle state for relay 1

// Switch State
bool SwitchState_1 = LOW;

int wifiFlag = 0;
float temperature1 = 0;
float humidity1 = 0;

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;

DHT dht(DHTPIN, DHTTYPE); // Inicializando o objeto dht do tipo DHT passando como parâmetro o pino (DHTPIN) e o tipo do sensor (DHTTYPE)

// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1)
{
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, !toggleState_1);
}

BLYNK_WRITE(VPIN_BUTTON_C)
{
  all_SwitchOff();
}

void all_SwitchOff()
{
  toggleState_1 = 0;
  digitalWrite(RelayPin1, HIGH);
  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  delay(100);

  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

void checkBlynkStatus()
{ // called every 2 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false)
  {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, HIGH);
  }
  if (isconnected == true)
  {
    wifiFlag = 0;
    if (!fetch_blynk_state)
    {
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    }
    digitalWrite(wifiLed, LOW);
    // Serial.println("Blynk Connected");
  }
}

BLYNK_CONNECTED()
{
  // Request the latest state from the server
  if (fetch_blynk_state)
  {
    Blynk.syncVirtual(VPIN_BUTTON_1);
  }
  Blynk.syncVirtual(VPIN_TEMPERATURE);
  Blynk.syncVirtual(VPIN_HUMIDITY);
}

void readSensor()
{

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else
  {
    humidity1 = h;
    temperature1 = t;
    // Serial.println(temperature1);
    // Serial.println(humidity1);
  }
}

void sendSensor()
{
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity1);
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature1);
}

void manual_control()
{
  if (digitalRead(SwitchPin1) == LOW && SwitchState_1 == LOW)
  {
    digitalWrite(RelayPin1, LOW);
    toggleState_1 = HIGH;
    SwitchState_1 = HIGH;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 on");
  }
  if (digitalRead(SwitchPin1) == HIGH && SwitchState_1 == HIGH)
  {
    digitalWrite(RelayPin1, HIGH);
    toggleState_1 = LOW;
    SwitchState_1 = LOW;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 off");
  }
}

void setup()
{
  Serial.begin(115200); // Inicializa a comunicação serial

  pinMode(RelayPin1, OUTPUT);

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);

  // During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !toggleState_1);

  dht.begin(); // Inicializa o sensor DHT11

  digitalWrite(wifiLed, HIGH);

  // Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensor);       // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);

  if (!fetch_blynk_state)
  {
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  }
}

void loop()
{
  manual_control();
  Blynk.run();
  timer.run();

  // float h = dht.readHumidity();//lê o valor da umidade e armazena na variável h do tipo float (aceita números com casas decimais)
  // float t = dht.readTemperature();//lê o valor da temperatura e armazena na variável t do tipo float (aceita números com casas decimais)

  // // if (isnan(h) || isnan(t)) {//Verifica se a umidade ou temperatura são ou não um número
  // //   return;//Caso não seja um número retorna
  // // }

  // Serial.print("Umidade: ");//Imprime no monitor serial a mensagem "Umidade: "
  // Serial.print(h);//Imprime na serial o valor da umidade
  // Serial.println("%");//Imprime na serial o caractere "%" e salta para a próxima linha
  // Serial.print("Temperatura: ");//Imprime no monitor serial a mensagem "Temperatura: "
  // Serial.print(t);//Imprime na serial o valor da temperatura
  // Serial.println("°C ");//Imprime no monitor serial "ºC" e salta para a próxima linha
}