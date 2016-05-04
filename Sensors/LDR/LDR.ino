#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";
const char* topic = "/home/ldr";
const int analogInPin = A0;  // Analog input pin that the LDR is attached to

IPAddress mqtt_server(192, 168, 15, 104); //CHANGE IP ADDRESS TO CURRENT ADDRESS OF BEAGLEBONE

WiFiClient espClient;
PubSubClient client(espClient);

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

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
}

int sensorValue = 0;        // value read from the pot
long lastMsg = 0;
char* outmessage;
String pubStr;

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

  //only update every second
  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    // read the analog in value:
    sensorValue = analogRead(analogInPin);
  
    // print the results to the serial monitor:
    Serial.print("sensor = ");
    Serial.print(sensorValue);
    Serial.println();
    
    //convert integer value to string
    pubStr = String(sensorValue);
    pubStr.toCharArray(outmessage, pubStr.length()+1);

    //publish value to topic
    client.publish(topic, outmessage, true);
    
    // wait 2 milliseconds before the next loop
    // for the analog-to-digital converter to settle
    // after the last reading:
    delay(2);
  }
}

