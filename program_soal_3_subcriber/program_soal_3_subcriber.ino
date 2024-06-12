#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Konfigurasi WiFi dan MQTT Broker
const char* ssid            = "Wokwi-GUEST";
const char* password        = "";
const char* mqtt_ip_broker  = "Isi dengan IP-Address Mqtt Broker";
const int   port_broker     = 1883;
const char* topic_sensor    = "test1/datasensor";
const char* id              = "ESP32(2) Sebagai Subscriber";
WiFiClient espclient;
PubSubClient client(espclient);

void setup_wifi(){
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Berhasil Terkoneksi dengan WiFi");
  Serial.print("Ip-Address : ");
  Serial.println(WiFi.localIP());
}

void callback (char* topic, byte* payload, unsigned int length){
  Serial.print("Telah menerima pesan dari topic");
  Serial.print(topic);
  Serial.print("]");
  String message;
  for (unsigned int i = 0; i < length; i++){
    message += (char)payload[i];
  } 
  Serial.println(message);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error){
    Serial.print("Gagal melakukan parsing JSON!");
    Serial.println(error.c_str());
    return;
  }
  float t         = doc["Temperature(*C)"];
  float h         = doc["Humidity"];
  float f         = doc["Temperature(*F)"];
  float heatindex = doc["HeatIndex(f,h)"];
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("Temperature: ");
  Serial.print(f);
  Serial.print(" *F ");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" % ");
  Serial.print("Heat Index (f dan H): ");
  Serial.print(heatindex);
  Serial.println(" *F ");

  lcd.setCursor(0,0);
  lcd.print("T :");
  lcd.print(t);
  lcd.print("   *C");

  lcd.setCursor(0,1);
  lcd.print("H :");
  lcd.print(h);
  lcd.print("   %");

  lcd.setCursor(0,2);
  lcd.print("T :");
  lcd.print(f);
  lcd.print("   *F");

  lcd.setCursor(0,3);
  lcd.print("HI:");
  lcd.print(heatindex);
  lcd.print("   *F");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_ip_broker, port_broker);
  client.setCallback(callback);
  lcd.init();
  lcd.backlight(); 
}
void reconnect(){
  while (!client.connected()) {
    if (client.connect(id)) {
      client.subscribe(topic_sensor);
    } else {
      Serial.print("gagal!, rc=");
      Serial.print(client.state());
      Serial.println("Coba lagi dalam 5 detik.");
      delay(5000);
    }
  }
}
void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
}
