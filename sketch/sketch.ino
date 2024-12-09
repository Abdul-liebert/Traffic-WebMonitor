#include <WiFi.h>
#include <MQTT.h>
#include <NusabotSimpleTimer.h>

// Pin sensor ultrasonik
#define TRIG_PIN 12
#define ECHO_PIN 13

// Informasi jaringan WiFi
const char* ssid = "SMK TI BAZMA";
const char* password = "bazma123";

// Informasi broker MQTT
const char* mqttServer = "abdulvision.cloud.shiftr.io";
const char* mqttUser = "abdulvision";
const char* mqttPassword = "KucQcYwsKtml14zj";
const int mqttPort = 1883;

// Objek WiFi, MQTT, dan timer
WiFiClient net;
MQTTClient client;
NusabotSimpleTimer timer;

// Variabel untuk perhitungan kendaraan
int vehicleCount = 0;
int lastPublishedCount = 0;
long lastVehicleDetectedTime = 0;
const long debounceDelay = 800;      // Waktu debounce (0.8 detik)
const long detectionThreshold = 30; // Threshold jarak deteksi (30 cm)

// Fungsi prototipe
void onMessageReceived(String &topic, String &payload);
void connectToMQTT();
void publishTrafficData();
long measureDistance();

void setup() {
  Serial.begin(115200); // Inisialisasi Serial Monitor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Sambungkan ke WiFi
  Serial.print("Menyambungkan ke WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke WiFi");

  // Konfigurasi MQTT client
  client.begin(mqttServer, mqttPort, net);
  client.onMessage(onMessageReceived);
  connectToMQTT(); // Sambungkan ke broker MQTT

  // Jadwalkan pengiriman data setiap 5 detik
  timer.setInterval(5000, publishTrafficData);
}

void loop() {
  client.loop(); // Jalankan client MQTT
  timer.run();   // Jalankan timer

  // Cek koneksi MQTT
  if (!client.connected()) {
    connectToMQTT();
  }

  // Hitung jarak dan deteksi kendaraan
  long distance = measureDistance();
  if (distance > 0 && distance < detectionThreshold && millis() - lastVehicleDetectedTime > debounceDelay) {
    vehicleCount++;
    lastVehicleDetectedTime = millis();
    Serial.print("Kendaraan terdeteksi. Jarak: ");
    Serial.print(distance);
    Serial.print(" cm. Total kendaraan: ");
    Serial.println(vehicleCount);
  }
}

// Fungsi untuk menghubungkan ke broker MQTT
void connectToMQTT() {
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke WiFi");

  Serial.print("Menghubungkan ke broker MQTT");
  while (!client.connect("TrafficMonitorDevice", mqttUser, mqttPassword)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke broker MQTT");
}

void publishTrafficData() {
  if (client.connected()) {
    String message = String(vehicleCount);
    client.publish("trf/index", message.c_str());
    Serial.print("Data lalu lintas diterbitkan: ");
    Serial.println(vehicleCount);
  }
}

long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void onMessageReceived(String &topic, String &payload) {
  Serial.println("Pesan diterima di topik: " + topic);
  Serial.println("Isi pesan: " + payload);
}
