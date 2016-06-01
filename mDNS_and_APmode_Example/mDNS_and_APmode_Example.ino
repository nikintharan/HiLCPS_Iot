/*  This sketch checks in EEPROM memory for a 
 *  stored WiFi SSID and password. If connection is
 *  unsuccessful, the Arduino will go into AP mode and
 *  host a page with an HTML form to get WiFi credentials.
 *  When successful, the ESP will then use mDNS to look 
 *  for openHAB services on the network and will attempt 
 *  to connect to the MQTT server at the first location 
 *  it finds.
 */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

//data structure to hold wifi name and password
//for EEPROM storage
struct wifiConfig {
  char ssid[32];
  char password[32];
};
wifiConfig configs;

// address to access/store data in EEPROM
int eeAddress = 0;

// HTML form that gets SSID and password info from user
String form = "<h2> Please enter your WiFi information here: </h2>"
              "<form action='/submit' method='post'>"
              "SSID: <input type='text' name='ssid'> <br />"
              "Password: <input type='password' name='password'>"
              "<input type='submit' value='Submit'>"
              "</form>";
  
// HTTP server will listen at port 80
ESP8266WebServer server(80);

// MQTT variables and setup
const char* topic = "/home/huzzah2/led";
WiFiClient espClient;
PubSubClient client(espClient);

// used to keep track of time
long now = 0, lastTime = 0;

// used to track if user has submmitted wifi info
bool isSubmit = false;

// to store unique ESP hostname
char hostString[16] = {0};


//Reconnects to MQTT client if disconnected
void reconnect_mqtt() {
  // Loop until we're reconnected to client
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //Every Huzzah needs to be named differently, or they will alternate
    //trying to connect, and will never execute any code. 
    if (client.connect(hostString)) {
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

// puts the ESP in AP mode and hosts a webpage with 
// an HTML form that asks for the user's WiFi info.
// It then attemps to connect using that info
void setup_AP() {
  
  //set up endpoints for HTTP server
  server.on("/", [](){
    server.send(200, "text/html", form);
  });

  // handle action for submission
  server.on("/submit", [](){
    //get user info
    strcpy(configs.ssid, server.arg("ssid").c_str());
    strcpy(configs.password, server.arg("password").c_str());   
 
    // prepare confirmation message and send
    char message[50];
    strcpy(message, "Now connecting to ");
    strcat(message, configs.ssid);
    server.send(200, "text/plain", message);
    isSubmit = true;
  });

  // Start the server 
  server.begin();
  Serial.println("HTTP server started");

  // loop runs until successful wifi connection
  while (WiFi.status() != WL_CONNECTED) {

    //clear config arrays to make way for new info
    memset(configs.ssid, 0, sizeof(configs.ssid));
    memset(configs.password, 0, sizeof(configs.password));

    // put ESP in AP mode
    // HTML page at 192.168.4.1
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESPAP", "password");

    //loop will exit when user has entered info
    while (!isSubmit) {
      server.handleClient();
    }
  
    // change back to station mode and try to connect
    // with new ssid and password
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(configs.ssid, configs.password);
    lastTime = millis();
    now = lastTime;
    while (WiFi.status() != WL_CONNECTED) {
      now = millis();
      if ( now - lastTime > 8000) {
        Serial.println("Connection unsucessful, please try again.");
        isSubmit = false;
        break;
      }
      delay(2); //needed to avoid wdt reset
    }
  }
  
  // if new ssid and password work, write to EEPROM
  if (WiFi.status() == WL_CONNECTED) {    
    EEPROM.put(eeAddress, configs);
  }
}

// use EEPROM to connect to WiFi. If first time, user can enter
// SSID and password information
void setup_wifi() {
  //clear config arrays to make way for new info
  memset(configs.ssid, 0, sizeof(configs.ssid));
  memset(configs.password, 0, sizeof(configs.password));
  
  // get values for struct (will return garbage for
  // first bootup) and attempt wifi connection
  EEPROM.get(eeAddress, configs);
  WiFi.begin(configs.ssid, configs.password);
  
  //wait 5 seconds for wifi to attempt to connect
  lastTime = millis();
  now = lastTime;
  while (WiFi.status() != WL_CONNECTED) {
    now = millis();
    if ( now - lastTime > 5000) {
      break;
    }
    delay(2); //needed to avoid wdt reset
  }

  while (WiFi.status() != WL_CONNECTED) {
    setup_AP();
  }

  Serial.println("WiFi connected.");
}

// uses mDNS to look for openHAB service, then
// tries to get IP address to connect to MQTT
// server on same device
void setup_mqtt() {

  // set up hostname for ESP
  sprintf(hostString, "ESP_%06X", ESP.getChipId());
  WiFi.hostname(hostString);

  //check to make sure mDNS is working
  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  }
  
  //look for openHAB service
  Serial.println("Sending mDNS query");
  int n = MDNS.queryService("openhab-server", "tcp"); // Send out query for esp tcp services
  Serial.println("mDNS query done");
  if (n == 0) {
    Serial.println("no services found");
  }
  else {
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
    
    // set up mosquitto connection and callback function
    // using first service found
    client.setServer(MDNS.IP(0), 1883);
    client.setCallback(callback);
  }
}

//Runs once upon startup
void setup() {  
  //Set up serial console (rate in baud)
  Serial.begin(115200);
  Serial.println("");
  Serial.setDebugOutput(true);

  setup_wifi();
  setup_mqtt();
    
  //set on-board LED to output, off by default (HIGH) 
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
}

//Continuously runs while Huzzah remains on
void loop() {
  
  //reconnect to mosquitto server if disconnected
  if (!client.connected()) {
    reconnect_mqtt();
  }
  
  //loop that runs for mosquitto actions 
  client.loop();
}
