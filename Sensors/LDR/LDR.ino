#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int analogInPin = A0;  // Analog input pin that the LDR is attached to

//Wifi connection variables and setup
const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//MQTT variables and setup
const char* topic = "/home/ldr";
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
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883); 
}

int sensorValue = 0;  // value read from the LDR
long lastMsg = 0;     // time of last message sent
long now = 0;         // current time since startup
char outmessage[10];  // message to be sent

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

  //only update every 3 seconds
  now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    // read the analog in value:
    sensorValue = analogRead(analogInPin);
  
    // print the results to the serial monitor:
    Serial.print("sensor = ");
    Serial.print(sensorValue);
    Serial.println();
    
    //convert integer value to char*
    sprintf(outmessage,"%d",sensorValue);

    //publish value to topic
    client.publish(topic, outmessage, true);
    
    // wait 2 milliseconds before the next loop
    // for the analog-to-digital converter to settle
    // after the last reading:
    delay(2);
  }
}

