#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Wifi connection variables and setup
const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//MQTT variables and setup
const char* topic = "/home/huzzah2/led";
IPAddress mqtt_server(192, 168, 15, 106); 
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
    if (client.connect("LED_ESP")) {
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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(0, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(0, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

//Runs once upon startup
void setup() {  
  //set on-board LED to output, off by default (HIGH) 
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);

  //Set up serial console (rate in baud)
  Serial.begin(115200);
  
  //setup wifi connection
  setup_wifi();
  
  //set up mosquitto connection and callback function
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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
