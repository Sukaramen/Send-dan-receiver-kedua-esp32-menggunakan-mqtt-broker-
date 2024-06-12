#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//Konfigurasi Koneksi WiFi dan MQTT Broker
const char* ssid            = "Wokwi-GUEST";
const char* password        = "";
const char* mqtt_ip_broker  = "Isi dengan IP-Address Mqtt Broker";
const int   port_broker     = 1883;
const char* topic           = "test1/datasensor";
const char* id              = "ESP32(1) Sebagai Publisher";
char charArray[256];
WiFiClient espclient;
PubSubClient client(espclient);

//Konfigurasi Sensor DHT11
#define DHTTYPE DHT22
#define DHTPIN  15
DHT dht(DHTPIN, DHTTYPE);
float h;
float t;
float f;
float heatindex;

//Konfigurasi Pembacaan Sensor Menggunakan Millis Function
unsigned long       oldTime2               = 0;
const unsigned long intervalSendDataSensor = 1000;

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
void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_ip_broker, port_broker);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(id)) {
      client.publish(topic, id);
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
  t = dht.readTemperature();
  h = dht.readHumidity();
  f = dht.readTemperature(true); //Pembacaan Temperature dalam farenheit
  heatindex = dht.computeHeatIndex(f,h);
  if(isnan(t)||isnan(h)||isnan(f)){
    Serial.println("Pembacaan Sensor Suhu Gagal!, Coba Beberapa Saat.");
    return;
  }
  StaticJsonDocument<200> doc;
  doc["Temperature(*C)"] = t;
  doc["Humidity"] = h;
  doc["Temperature(*F)"] = f;
  doc["HeatIndex(f,h)"] = heatindex;
  serializeJson(doc, charArray);
  if(millis() - oldTime2 > intervalSendDataSensor){
    client.publish(topic, charArray);
    oldTime2 = millis();
  }
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
  delay(1000);
}
