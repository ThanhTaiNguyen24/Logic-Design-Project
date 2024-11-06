#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/****************************** Define MQTT ***************************************/

#define WLAN_SSID "RD-SEAI_2.4G"
#define WLAN_PASS ""

#define AIO_USERNAME  "ThanhTaiNguyen"
#define AIO_KEY       "aio_GAzm50zeE6u2WheOGlKSFLkml5EW"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

#define OHS_SERVER      "mqtt.ohstem.vn"
#define OHS_SERVERPORT  1883
#define OHS_USERNAME    "tainguyen24082004"
#define OHS_KEY         "helloworld"

/****************************** Define PIN ***************************************/
#define D5 8
#define D3 6
#define A1 2
#define A0 1
#define LED_BUILTIN 48
#define SDA 11
#define SCL 12
#define ANALOG_INPUT_PIN 1

WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Client mqtt(&client, OHS_SERVER, OHS_SERVERPORT, OHS_USERNAME, OHS_USERNAME, OHS_KEY);
/****************************** ADAFRUIT Feeds ***************************************/
//Cambiennhietdo
Adafruit_MQTT_Publish cambiennhietdo = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambiennhietdo");

//Cambiendoam
Adafruit_MQTT_Publish cambiendoam = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambiendoam");

//Cambienanhsang
Adafruit_MQTT_Publish cambienanhsang = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/cambienanhsang");

//nutnhan
Adafruit_MQTT_Subscribe nutnhan1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/nutnhan1", MQTT_QOS_1);

//Battatden
Adafruit_MQTT_Publish battatden = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battatden");

//Doamdat
Adafruit_MQTT_Publish doamdat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/doamdat");

/****************************** OhStem Feeds ***************************************/
//Cambiendoam
Adafruit_MQTT_Publish ohstemdoam = Adafruit_MQTT_Publish(&mqtt, OHS_USERNAME "/feeds/V1");

//Cambiennhietdo
Adafruit_MQTT_Publish ohstemnhietdo = Adafruit_MQTT_Publish(&mqtt, OHS_USERNAME "/feeds/V2");

//Cambienanhsang
Adafruit_MQTT_Publish ohstemanhsang = Adafruit_MQTT_Publish(&mqtt, OHS_USERNAME "/feeds/V3");

//Doamdat
Adafruit_MQTT_Publish ohstemdoamdat = Adafruit_MQTT_Publish(&mqtt, OHS_USERNAME "/feeds/V4");



/****************************** Define Tasks ***************************************/
void TaskBlink(void *pvParameters);
void TaskTemperatureHumidity(void *pvParameters);
void TaskSoilMoistureAndRelay(void *pvParameters);
void TaskLightAndLED(void *pvParameters);


/****************************** Define Components ***************************************/
Adafruit_NeoPixel pixels3(4, D5, NEO_GRB + NEO_KHZ800);
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);


void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000);  // wait 10 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}


// The setup function runs once when you press reset or power on the board.
void setup() {

  Serial.begin(115200);
  Wire.begin(SDA,SCL);
  dht20.begin();
  lcd.begin();
  pixels3.begin();



WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(2000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



  uint32_t blink_delay = 1000;  
xTaskCreate(TaskBlink, "Task Blink", 2048, (void *)&blink_delay, 2, nullptr);
xTaskCreate( TaskTemperatureHumidity, "Task Temperature" ,2048  ,NULL  ,2 , NULL);
xTaskCreate( TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
xTaskCreate( TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);

}

void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {
      mqtt.disconnect();
  }
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
uint32_t x = 0;
void TaskBlink(void *pvParameters) { 
  uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {                          
    digitalWrite(LED_BUILTIN, HIGH);  
    delay(blink_delay);
    digitalWrite(LED_BUILTIN, LOW);  
    delay(blink_delay);
    if (battatden.publish(x++)){
      Serial.println(F("Published count of LED successfully!!"));
    }
  }
}


void TaskTemperatureHumidity(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    dht20.read();
    float Temperature = dht20.getTemperature();
    float Humidity = dht20.getHumidity();
    Serial.println(F("Temperature"));
    Serial.println(Temperature);
    Serial.println(F("Humidity"));
    Serial.println(Humidity);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(Temperature);
    lcd.setCursor(0, 1);
    lcd.print(Humidity);
    if (cambiennhietdo.publish(Temperature)){
      Serial.println(F("Published Temperature Sensor successfully!!"));
    }
    if (cambiendoam.publish(Humidity)){
      Serial.println(F("Published Humidity Sensor successfully!!"));
    }
    delay(5000);
  }
}

void TaskSoilMoistureAndRelay(void *pvParameters) {  
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(D3, OUTPUT);

  while(1) { 
    uint32_t moisture = analogRead(A0);                        
    if(moisture > 300){
      digitalWrite(D3, LOW);
    }
    if(moisture < 50){
      digitalWrite(D3, HIGH);
    }
    Serial.println(F("Moisture:"));
    Serial.println(moisture);
    if (doamdat.publish(moisture)){
      Serial.println(F("Published Moisture Sensor successfully!!"));
    }
    delay(2000);
  }
}

void TaskLightAndLED(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) { 
    uint32_t light = analogRead(A1);
    Serial.println(F("Light: "));
    Serial.println(light);                      
    if(light < 350){
      pixels3.setPixelColor(0, pixels3.Color(0,0,255));
      pixels3.setPixelColor(1, pixels3.Color(0,255,0));
      pixels3.setPixelColor(2, pixels3.Color(255,0,0));
      pixels3.setPixelColor(3, pixels3.Color(128, 0, 128));
      pixels3.show();
    }
    if(light > 550){
      pixels3.setPixelColor(0, pixels3.Color(0,0,0));
      pixels3.setPixelColor(1, pixels3.Color(0,0,0));
      pixels3.setPixelColor(2, pixels3.Color(0,0,0));
      pixels3.setPixelColor(3, pixels3.Color(0,0,0));
      pixels3.show();
    }
    
    if (cambienanhsang.publish(light)){
      Serial.println(F("Published Light Sensor successfully!!"));
    }
    delay(3000);
  }
}