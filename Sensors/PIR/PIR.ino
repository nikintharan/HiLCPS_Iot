#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";
const char* pir_topic = "/home/pir";

IPAddress mqtt_server(192, 168, 15, 104); //CHANGE IP ADDRESS TO CURRENT ADDRESS OF BEAGLEBONE

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
    if (client.connect("PIR_ESP")) {
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
  Serial.begin(115200);
  setup_wifi();

  //Data from PIR read on pin 2
  pinMode(2, INPUT);
  
  //TESTING 
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}

int val=0;
int pirState = LOW;

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
  
  // Motion Sensor
  val = digitalRead(2);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
        digitalWrite(0, LOW);

      // Publish to Mosquitto 
       client.publish(pir_topic, "1", true);
    
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
        digitalWrite(0, HIGH);
      
      //Publish to Mosquitto 
       client.publish(pir_topic, "0", true);
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}


