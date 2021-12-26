/*
 Circuit:
 * MKR GSM 1400 board
 * Antenna
 * SIM card with a data plan
*/
#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime
#include <MKRGSM.h>
#include "arduino_secrets.h" 
#include <ArduinoJson.h>

const char PINNUMBER[]     = SECRET_PINNUMBER;
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

int port = 443;

// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
int sleeping_ms = 3573000;
// int sleeping_ms = 180000; // 3 min

char server[] = "your.server.net";
char path[] = "/your/endpoint/"; // whatever your endpoint might be

GSMSSLClient client;
GPRS gprs;
GSM gsmAccess;
GSMModem modem;

String IMEI = "";

void startModem() {

  bool connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
      Serial.println(F("Connected to network"));
      Serial1.println(F("Connected to network"));

      //Get IP.
      IPAddress LocalIP = gprs.getIPAddress();
      Serial.print(F("Local IP address = "));
      Serial.println(LocalIP);
      Serial1.print(F("Local IP address = "));
      Serial1.println(LocalIP);

      IMEI = modem.getIMEI();
      Serial.println("Modem's IMEI: " + IMEI);
      Serial1.println("Modem's IMEI: " + IMEI);

    } else {
      Serial.println(F("Not connected to network"));
      Serial1.println(F("Not connected to network"));
      delay(1000);
    }
  }
}

void makeWebRequest() {
  Serial.println(F("connecting to REST API ..."));
  Serial1.println(F("connecting to REST API ..."));

  // https://arduinojson.org/v6/example/http-server/ (for sending json)
  // Allocate a temporary JsonDocument
  // One could use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<200> doc;
  doc["imei_string"] = IMEI;
  doc["uptime"] = getUptime();

  // Serial.println(getUptime());
  // Serial1.println(getUptime());
  // Serial.println("Modem's IMEI: " + IMEI);


  if (client.connect(server, port)) {
    Serial.println(F("connected to REST API"));
    Serial1.println(F("connected to REST API"));
    client.print(F("POST "));
    client.print(path);
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(server);

    // https://forum.arduino.cc/t/how-to-make-a-post-request-with-mkrgsm-gsmsslclient-library/501250/13
    // https://forum.arduino.cc/t/tomcat-arduino-gsm-shield-post-json/387640
    client.println(F("Content-Type: application/json;charset=UTF-8"));
    client.println(F("Connection: close"));

    client.print("Content-Length: ");
    client.println(measureJsonPretty(doc));

    /*
    Serial.print(F("Content-Length: "));
    Serial1.print(F("Content-Length: "));
    Serial.println(measureJsonPretty(doc));
    Serial1.println(measureJsonPretty(doc));
    */


    // Terminate headers
    client.println();
    Serial.println();
    Serial1.println();

    // Send body
    serializeJsonPretty(doc, client);

    // pause waiting for response
    delay(750);

    // https://arduinojson.org/v6/example/http-client/
    // Check HTTP status
    // char status[32] = {0}; // Good for "HTTP/1.1 200 OK"
    char status[96] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    // Serial.println(status);
    // Serial.println(sizeof(status));

    if (strcmp(status, "HTTP/1.1 201 Created") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      Serial1.print(F("Unexpected response: "));
      Serial1.println(status);
      client.stop();
      return;
    } else {
      Serial.print(F("Status: "));
      Serial.println(status);
      Serial1.print(F("Status: "));
      Serial1.println(status);
    }
  
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      Serial1.println(F("Invalid response"));
      client.stop();
      return;
    }
  
    // Allocate the JSON document - couldn't get this to work :(
    // Use arduinojson.org/v6/assistant to compute the capacity.
    // Using ngrok to debug this;
    // const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    // Serial.print("capacity: ");
    // Serial.println(capacity);
    // DynamicJsonDocument doc(capacity);
    DynamicJsonDocument doc(512);
  
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      Serial1.print(F("deserializeJson() failed: "));
      Serial1.println(error.f_str());
      client.stop();
      return;
    }
  
    // Extract values
    Serial.print(F("Response: "));
    Serial1.print(F("Response: "));
    Serial.println(doc["url"].as<char*>());
    Serial1.println(doc["url"].as<char*>());

    /* Examples
    Serial.print(F("locations: "));
    Serial.println(doc["locations"].as<char*>());
    Serial.print(F("arduinos: "));
    Serial.println(doc["arduinos"].as<char*>());
    Serial.print(F("reports: "));
    Serial.println(doc["reports"].as<char*>());
    Serial.println(doc["sensor"].as<char*>());
    Serial.println(doc["time"].as<long>());
    Serial.println(doc["data"][0].as<float>(), 6);
    Serial.println(doc["data"][1].as<float>(), 6);
    */
  
    // Disconnect
    client.stop();

  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed - check Readme: has cert validation been disabled?"));
    Serial1.println(F("connection failed - check Readme: has cert validation been disabled?"));
  }

  Serial.println(F("Blinking  5 sec ... web request finished"));
  Serial1.println(F("Blinking 5 sec  ... web request finished"));
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second

}

String getUptime() {
  return "uptime: "+uptime_as_string()+" or "+uptime()+"s";
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */
  delay(2000);

  Serial.println(F("Sketch: GsmWebClient_hourly_SSL_POST."));
  Serial1.println(F("Sketch: GsmWebClient_hourly_SSL_POST."));
}

void loop() {
  startModem();
  makeWebRequest();

  Serial.println(getUptime());
  Serial1.println(getUptime());
  Serial.print(F("Sleeping for "));
  Serial.print(sleeping_ms);
  Serial.println(F(" ms Note: 3600000 is 1 hour"));
  Serial1.print(F("Sleeping for "));
  Serial1.print(sleeping_ms);
  Serial1.println(F(" ms Note: 3600000 is 1 hour"));
  delay(sleeping_ms);

  // do nothing:
  // while (true);
}

/*
# vim: ai et ts=2 sw=2 sts=2 nu
*/
