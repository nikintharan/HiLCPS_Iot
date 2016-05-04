/*
  Example for different sending methods
  
  http://code.google.com/p/rc-switch/
  
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

const char* topic = "/home/TX";
IPAddress mqtt_server(192, 168, 15, 104); 

WiFiClient espClient;
PubSubClient client(espClient);

RCSwitch mySwitch = RCSwitch();

int code = 0, bitLen = 0, pulseLen = 0;

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
    if (client.connect("TX Huzzah")) {
      Serial.println("connected");
      client.subscribe(topic);
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

  //parse message
  String message = String((char*)payload);
  code = message.substring(0,message.indexOf(',')).toInt();
  bitLen = message.substring(message.indexOf(',')+1,message.lastIndexOf(',')).toInt();
  pulseLen = message.substring(message.lastIndexOf(',')+1).toInt();
  Serial.println(pulseLen);

  //Set correct pulse length from message, transmit code at specified bitlength
  mySwitch.setPulseLength(pulseLen);
  mySwitch.send(code, bitLen);

  //Clear payload 
  int n=0;
  while (payload[n] != 0) {
    payload[n] = 0;
    n++;
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Transmitter is connected to Arduino Pin #2  
  mySwitch.enableTransmit(2);

  // Optional set number of transmission repetitions. 
  mySwitch.setRepeatTransmit(9);
}

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
  /* Same switch as above, but using decimal code */
}
