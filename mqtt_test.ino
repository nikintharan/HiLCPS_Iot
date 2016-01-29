#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";
const char* topic = "/home/esp/com";
const char* outTopic = "/home/esp/state";
IPAddress mqtt_server(192, 168, 15, 102); 
//const char* mqtt_server = "beaglebone.local";

WiFiClient espClient;
PubSubClient client(espClient);

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
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  if (digitalRead(0) == HIGH){ //If LED is off, state is 0
    newState = 0;
  }
  else if (digitalRead(0) == LOW){ //If LED is on, state is 1
    newState = 1;
  }

  if (newState != oldState){
    if(newState)
      client.publish(outTopic, "1");
    else
      client.publish(outTopic, "0");
    oldState = newState;
  }
}
