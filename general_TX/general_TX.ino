#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

//Wifi connection variables and setup
const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//MQTT variables and setup
const char* topic = "/home/TX";
IPAddress mqtt_server(192, 168, 15, 106); 
WiFiClient espClient;
PubSubClient client(espClient);

//setup object for RF transmission
RCSwitch mySwitch = RCSwitch();

//variables used to characterize RF signals
int code = 0, bitLen = 0, pulseLen = 0;

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
    if (client.connect("TX_Huzzah")) {
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

//Handles incoming messages and what to do with them
void callback(char* topic, byte* payload, unsigned int length) {
  //Print message
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //parse message and store in separate variables
  String message = String((char*)payload);
  code = message.substring(0,message.indexOf(',')).toInt();
  bitLen = message.substring(message.indexOf(',')+1,message.lastIndexOf(',')).toInt();
  pulseLen = message.substring(message.lastIndexOf(',')+1).toInt();
  Serial.println(pulseLen);

  //Set correct pulse length from message, transmit code at specified bitlength
  mySwitch.setPulseLength(pulseLen);
  mySwitch.send(code, bitLen);

  //Clear payload (for some reason this library does not remove
  //old messages when a new one comes in, meaning that characters
  //from the end of old messages can be appended to new ones)
  int n=0;
  while (payload[n] != 0) {
    payload[n] = 0;
    n++;
  }
}

//Runs once upon startup
void setup() {
  //Set up serial console (rate in baud)
  Serial.begin(115200);
  
  //setup wifi connection
  setup_wifi();
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);

  //setup callback function
  client.setCallback(callback);

  // Transmitter is connected to Arduino Pin #2  
  mySwitch.enableTransmit(2);

  // Optional set number of transmission repetitions 
  // (9 seems to work well) 
  mySwitch.setRepeatTransmit(9);
}

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
}
