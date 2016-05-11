#include <Adafruit_HDC1000.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Wifi connection variables and setup
const char* ssid     = "EEGu_2_4GHz";
const char* password = "";

//MQTT variables and setup
const char* humidity_topic = "/home/huzzah1/humidity";
const char* temperature_topic = "/home/huzzah1/temp";
IPAddress mqtt_server(192, 168, 15, 104); //CHANGE IP ADDRESS TO CURRENT ADDRESS OF BEAGLEBONE
WiFiClient espClient;
PubSubClient client(espClient);

//create object for sensor
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

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
    if (client.connect("Temp_ESP")) {
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

  // Set SDA and SDL ports for HDC
  Wire.begin(2, 14);

  // Start sensor
  if (!hdc.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
}

long lastMsg = 0;   //Time of last message
long now = 0;       //Current time
float temp = 0.0;   //Measured temperature in celcius
float hum = 0.0;    //Measured humidity in %

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

  //get current time since startup
  now = millis();

  //only execute every second
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
