#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>           //https://github.com/winlinvip/SimpleDHT
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define ssid          "FAST_AD00F0"
#define password      "1357924680"

//adafruit stuff
#define SERVER          "io.adafruit.com"
#define SERVERPORT      1883
#define MQTT_USERNAME   "luigipizzolito"
#define MQTT_KEY        "6e123dc9bf924dffb8fa7b4eb1408e76"

#define USERNAME          "luigipizzolito/"
#define PREAMBLE          "feeds/"
#define T_LUMINOSITY      "luminosity"
#define T_TEMPERATURE     "temperature"
#define T_HUMIDITY        "humidity"

//IFTTT E-Mail Stuff
const String privateKey = "hEPx88jPBFo8rhXT9Aav-8xzp18Nrd9y3Y4oMOr-FMk";
ADC_MODE(ADC_VCC);

unsigned long entry;
byte clientStatus, prevClientStatus = 99;
float luminosity = -1;
char valueStr[5];
byte temp = 0;
byte humi = 0;

const int pinDHT11 = 2;
SimpleDHT11 dht11;

WiFiClient WiFiClient;
// create MQTT object
PubSubClient client(WiFiClient);


//
void setup() {
  Serial.begin(115200);
  manageWiFi();
  delay(100);
  Serial.println("alive");
  Serial.println();
  Serial.println();
  Serial.println();
  /*
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
  WiFi.printDiag(Serial);
*/
  client.setServer(SERVER, SERVERPORT);
  //client.setCallback(callback);
}

//
void loop() {
  //Serial.println("loop");
  yield();

  if (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("", MQTT_USERNAME, MQTT_KEY)) {
      Serial.println("connected");
      // ... and resubscribe
      //client.subscribe(USERNAME PREAMBLE T_COMMAND, 1);
      //client.subscribe(USERNAME PREAMBLE "test", 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("reseting..");
      ESP.reset();
    }
  }

  if (millis() - entry > 1200) {
    Serial.println("Measure");
    entry = millis();
    //luminosity = analogRead(A0);                    //use digital pin
    luminosity = 58;
    if (dht11.read(pinDHT11, &temp, &humi, NULL)) {
      Serial.print("Read DHT11 failed.");
      ESP.restart();
      return;
    }
  }

  if (client.connected()) {
    Serial.println("Publish Data");
    String hi = (String)map(luminosity, 0, 1024, 0, 100);
    hi.toCharArray(valueStr, 5);
    client.publish(USERNAME PREAMBLE T_LUMINOSITY, valueStr);
    hi = (String)temp;
    hi.toCharArray(valueStr, 5);
    client.publish(USERNAME PREAMBLE T_TEMPERATURE, valueStr);
    hi = (String)humi;
    hi.toCharArray(valueStr, 5);
    client.publish(USERNAME PREAMBLE T_HUMIDITY, valueStr);
    delay(500);
  }

  client.loop();
  delay(5000);
  Serial.println("sleeping");
  ESP.deepSleep(20 * 1000000);
}


//----------------------------------------------------------------------
/*

  void callback(char* topic, byte * data, unsigned int length) {
  // handle message arrived {

  Serial.print(topic);
  Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();
  if (data[1] == 'F')  {
    clientStatus = 0;
    digitalWrite(LEDBLUE, LOW);
  } else {
    clientStatus = 1;
    digitalWrite(LEDBLUE, HIGH);
  }
  }
*/

void manageWiFi() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect("ESP8266 Weather Station");


  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  digitalWrite(BUILTIN_LED, LOW);
}

void lowbatteryEMAIL() {
  Serial.println("sending low battery e-mail");
  Serial.println();
  Serial.print("connecting to ");
  Serial.println("maker.ifttt.com");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("maker.ifttt.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }


  // This will send the request to the server
  client.print("POST /trigger/");
  client.print("station_low_battery");
  client.print("/with/key/");
  client.print(privateKey);
  client.println(" HTTP/1.1");
  client.println("Host: maker.ifttt.com");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");

  String data = "{\"value1\":";
  data += ESP.getVcc();
  data += "}";

  client.println(data.length());
  client.println();
  client.println(data);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}


void startupEMAIL() {
  Serial.println("sending startup e-mail");
  Serial.println();
   Serial.print("connecting to ");
  Serial.println("maker.ifttt.com");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("maker.ifttt.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }


  // This will send the request to the server
  client.print("POST /trigger/");
  client.print("station_on");
  client.print("/with/key/");
  client.print(privateKey);
  client.println(" HTTP/1.1");
  client.println("Host: maker.ifttt.com");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void okEMAIL() {
  Serial.println("sending ok e-mail");
  Serial.println();
   Serial.print("connecting to ");
  Serial.println("maker.ifttt.com");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("maker.ifttt.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }


  // This will send the request to the server
  client.print("POST /trigger/");
  client.print("station_ok");
  client.print("/with/key/");
  client.print(privateKey);
  client.println(" HTTP/1.1");
  client.println("Host: maker.ifttt.com");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

