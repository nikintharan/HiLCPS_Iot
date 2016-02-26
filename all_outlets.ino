/*
  Example for different sending methods
  
  http://code.google.com/p/rc-switch/
  
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//const char* topic = "/home/outlets/1";
IPAddress mqtt_server(192, 168, 15, 104); 

WiFiClient espClient;
PubSubClient client(espClient);

RCSwitch mySwitch = RCSwitch();

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

  client.subscribe("/home/outlets/1");
  client.subscribe("/home/outlets/2");
  client.subscribe("/home/outlets/3");
  client.subscribe("/home/outlets/4");
  client.subscribe("/home/outlets/5");
}

void reconnect_mqtt() {
  // Loop until we're reconnected to client
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("OutletHuzzah")) {
      Serial.println("connected");
      client.subscribe("/home/outlets/1");
      client.subscribe("/home/outlets/2");
      client.subscribe("/home/outlets/3");
      client.subscribe("/home/outlets/4");
      client.subscribe("/home/outlets/5");
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

  if (String(topic) == "/home/outlets/1") {
    if ((char)payload[0] == '1') {
        Serial.println("sending on command...");
        mySwitch.send(4527411, 24); //turn outlet 1 on  
    } else {
        Serial.println("sending off command...");
        mySwitch.send(4527420, 24);    //turn outlet 1 off
      }
  } //if for socket 1

  if (String(topic) == "/home/outlets/2") {
    if ((char)payload[0] == '1') {
        Serial.println("sending on command...");
        mySwitch.send(4527555, 24); //turn outlet 2 on  
    } else {
        Serial.println("sending off command...");
        mySwitch.send(4527564, 24);    //turn outlet 2 off
      }
  } //if for socket 2

  if (String(topic) == "/home/outlets/3") {
    if ((char)payload[0] == '1') {
        Serial.println("sending on command...");
        mySwitch.send(4527875, 24); //turn outlet 3 on  
    } else {
        Serial.println("sending off command...");
        mySwitch.send(4527884, 24);    //turn outlet 3 off
      }
  } //if for socket 3

  if (String(topic) == "/home/outlets/4") {
    if ((char)payload[0] == '1') {
        Serial.println("sending on command...");
        mySwitch.send(4529411, 24); //turn outlet 4 on  
    } else {
        Serial.println("sending off command...");
        mySwitch.send(4529420, 24);    //turn outlet 4 off
      }
  } //if for socket 4

  if (String(topic) == "/home/outlets/5") {
    if ((char)payload[0] == '1') {
        Serial.println("sending on command...");
        mySwitch.send(4535555, 24); //turn outlet 5 on  
    } else {
        Serial.println("sending off command...");
        mySwitch.send(4535564, 24);    //turn outlet 5 off
      }
  } //if for socket 5
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  
  //set up mosquitto connection 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Transmitter is connected to Arduino Pin #2  
  mySwitch.enableTransmit(2);

  // Optional set pulse length. 
  mySwitch.setPulseLength(185);

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
