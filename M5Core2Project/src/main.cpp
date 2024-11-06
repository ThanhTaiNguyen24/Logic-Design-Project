#include <M5Core2.h>
#include <DHT20.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <WiFi.h>


#define WLAN_SSID "RD-SEAI_2.4G"
#define WLAN_PASS ""
#define AIO_USERNAME  "ThanhTaiNguyen"
#define AIO_KEY       "aio_GAzm50zeE6u2WheOGlKSFLkml5EW"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
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



void TaskBlink(void *pvParameters);
void TaskTemperatureHumidity(void *pvParameters);
void TaskSoilMoistureAndRelay(void *pvParameters);
void TaskLightAndLED(void *pvParameters);

DHT20 dht20;
Adafruit_NeoPixel pixels3(4, 13, NEO_GRB + NEO_KHZ800);

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

void setup() {

    M5.begin(true, false, true, true, kMBusModeOutput, false); 
    dht20.begin();
    pixels3.begin();

    M5.Lcd.setTextSize(4);  
    M5.Lcd.fillScreen(BLACK);  


    WiFi.begin(WLAN_SSID, WLAN_PASS);
    delay(2000);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


   xTaskCreate(TaskTemperatureHumidity, "Task Temperature" ,2048  ,NULL  ,2 , NULL);
   xTaskCreate(TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);
   xTaskCreate( TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
}
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {
      mqtt.disconnect();
  }
}
void TaskTemperatureHumidity(void *pvParameters) {  
  while(1) {
    M5.Lcd.clear();                          
    dht20.read();
    float Temperature = dht20.getTemperature();
    float Humidity = dht20.getHumidity();

    // Display Temperature
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("Temperature:");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf("%.2f C", Temperature); 

    // Display Humidity
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("Humidity:");
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.printf("%.2f %%", Humidity); 

    Serial.print("Temperature: ");
    Serial.print(Temperature);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(Humidity);
    Serial.println(" %");
    if (cambiennhietdo.publish(Temperature)){
      Serial.println(F("Published Temperature Sensor successfully!!"));
    }
    if (cambiendoam.publish(Humidity)){
      Serial.println(F("Published Humidity Sensor successfully!!"));
    }
    vTaskDelay(5000);
    
  }
}
void TaskLightAndLED(void *pvParameters) { 

  while(1) { 
    uint32_t light = analogRead(34);
    Serial.println(F("Light: "));
    Serial.println(light);                      
    if(light < 400){
      pixels3.setPixelColor(0, pixels3.Color(0,0,255));
      pixels3.setPixelColor(1, pixels3.Color(0,255,0));
      pixels3.setPixelColor(2, pixels3.Color(255,0,0));
      pixels3.setPixelColor(3, pixels3.Color(128, 0, 128));
      pixels3.show();
    }
    if(light > 600){
      pixels3.setPixelColor(0, pixels3.Color(0,0,0));
      pixels3.setPixelColor(1, pixels3.Color(0,0,0));
      pixels3.setPixelColor(2, pixels3.Color(0,0,0));
      pixels3.setPixelColor(3, pixels3.Color(0,0,0));
      pixels3.show();
    }
    if (cambienanhsang.publish(light)){
      Serial.println(F("Published Light Sensor successfully!!"));
    }
    vTaskDelay(7000);
  }
}
void TaskSoilMoistureAndRelay(void *pvParameters) {  

  pinMode(2, OUTPUT);

  while(1) { 
    uint32_t moisture = analogRead(36);                        
    if(moisture > 300){
      digitalWrite(2, LOW);
    }
    if(moisture < 50){
      digitalWrite(2, HIGH);
    }
    Serial.println(F("Moisture:"));
    Serial.println(moisture);
    if (doamdat.publish(moisture)){
      Serial.println(F("Published Moisture Sensor successfully!!"));
    }
    vTaskDelay(9000);
  }
}