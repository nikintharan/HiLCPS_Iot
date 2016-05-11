#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Wifi connection variables and setup
const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//MQTT variables and setup
const char* pir_topic = "/home/pir";
IPAddress mqtt_server(192, 168, 15, 106); //CHANGE IP ADDRESS TO CURRENT ADDRESS OF BEAGLEBONE
WiFiClient espClient;
PubSubClient client(espClient);

//Sets up the initial Wifi connection
void setup_wifi() {
  delay(10);
  // Print info to Serial console
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //attempt connection
  WiFi.begin(ssid, password);

  //print "." for each half second while waiting to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //print success to Serial console
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Reconnects to MQTT client if disconnected
void reconnect_mqtt() {
  // Loop until we're reconnected to client
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //Every Huzzah needs to be named differently, or they will alternate
    //trying to connect, and will never execute any code. 
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

//If wifi connection is disrupted, attempt to reconnect
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

//Runs once upon startup
void setup() {    
  //Set up serial console (rate in baud)
  Serial.begin(115200);

  //setup wifi connection
  setup_wifi();

  //Data from PIR read on pin 2
  pinMode(2, INPUT);
  
  //Allows control of on-board LED
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);  //Turn off
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
}

int val=0;            //Output from PIR
int pirState = LOW;   //keeps track of last state

//Continuously runs while Huzzah remains on
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
  if (val == HIGH) {
    if (pirState == LOW) {
      // we have just turned on, turn LED on
      Serial.println("Motion detected!");
      digitalWrite(0, LOW);

      // Publish to Mosquitto 
      client.publish(pir_topic, "1", true);
    
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned off
      Serial.println("Motion ended!");
      digitalWrite(0, HIGH);
      
      //Publish to Mosquitto 
      client.publish(pir_topic, "0", true);
      
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}

