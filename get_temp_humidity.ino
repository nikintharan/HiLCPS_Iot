#include <Adafruit_HDC1000.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

const char* led_topic = "/home/huzzah1/led";

const char* humidity_topic = "/home/huzzah1/humidity";
const char* temperature_topic = "/home/huzzah1/temp";

IPAddress mqtt_server(192, 168, 15, 102); 

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

int oldState = 0, newState = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
  // Loop until we're reconnected to client
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(led_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void reconnect_wifi() {
  Serial.print("Wifi disconnected, reconnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(0, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(0, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void setup() {
  // put your setup code here, to run once:
  
  //set on-board LED to output, off by default (HIGH) 
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  
  Serial.begin(115200);
  setup_wifi();
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Set SDA and SDL ports
  Wire.begin(2, 14);

  // Start sensor
  if (!hdc.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

void loop() {
  //reconnect to mosquitto server if disconnected
  if (!client.connected()) {
    reconnect_mqtt();
  }

  //reconnect to wifi if disconnected 
  if (WiFi.status() != WL_CONNECTED) {
    reconnect_wifi();
  }
  
  //loop that runs for mosquitto actions 
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    //get temp and humidity
    temp = hdc.readTemperature();
    hum = hdc.readHumidity();

    //print temp
    Serial.print("Temp: "); Serial.print(hdc.readTemperature());
    Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());

    //publish to mosquitto
    client.publish(temperature_topic, String(temp).c_str(), true);
    client.publish(humidity_topic, String(hum).c_str(), true);
  }
}
