/*
 * Sketch:
 * Arduino MKR GSM 1400
 * Antenna
 * SIM: Hologram
 * Arduino MKR GPS (optional: set gps_timeout = 0 if not using)
 * Based on GsmWebClient_hourly_SSL_Post_GPS
*/

#include <Arduino.h>
#include <MKRGSM.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

#include <Arduino_MKRGPS.h>

const char PINNUMBER[]     = SECRET_PINNUMBER;
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;
const char REST_SERVER[]   = SECRET_REST_SERVER;
const char REST_ENDPOINT[] = SECRET_REST_ENDPOINT;

int port = 443;

// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
int sleeping_ms = 3561000;
//int sleeping_ms = 180000; // 3 min

char server[] = "your.server.net";
char path[] = "/your/endpoint/"; // whatever your endpoint might be

/*
 * 6000000 is approx 8.5 sec
 * 90000000 is approx 2 min
 * Set to 0 if no GPS shield
 */
int gps_timeout = 90000000;

String IMEI = "";
int start_time = 0;
String msg = "Start time (unixtime / utc_datetime): ";

struct GPSInfo {
  float latitude;
  float longitude;
  float altitude;
  int   satellites;
};

GPSInfo getGPSInfo() {
  bool connected = false;
  GPSInfo g = {0.0, 0.0, 0.0, 0};

  // Let's make this "optional"
  //while (!connected) {
    if (GPS.begin(GPS_MODE_SHIELD)) {
      Serial.println(F("GPS initialized"));
      connected = true;
      Serial.print("wait location ... ");
    
      bool gps_available = false;
      int counter = 0;
      unsigned long startMillis = millis();

      while (!gps_available) {
        if (GPS.available()) {
          gps_available = true;
          unsigned long endMillis = millis();
          Serial.print(endMillis - startMillis);
          Serial.println(F(" ms to get GPS data"));
          g.latitude   = GPS.latitude();
          g.longitude  = GPS.longitude();
          g.altitude   = GPS.altitude();
          g.satellites = GPS.satellites();
        } else {
          if (counter == gps_timeout) {
            gps_available = true;
            Serial.println(F("GPS is unavailable"));
            unsigned long endMillis = millis();
            Serial.print(endMillis - startMillis);
            Serial.println(F(" ms spent checking for GPS data"));
            Serial.print(F("Counter: "));
            Serial.println(counter);
          }  
          counter += 1;
        }
      }
    } else {
      Serial.println("Failed to initialize GPS!");
    }
  //}

  GPS.end();
  return g;
}

void startModem() {
  GPRS gprs;
  //GSM gsmAccess(true);
  GSM gsmAccess;
  GSMModem modem;

  bool connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
      Serial.println(F("Connected to network"));

      //Get IP.
      IPAddress LocalIP = gprs.getIPAddress();
      Serial.print(F("Local IP address = "));
      Serial.println(LocalIP);

      IMEI = modem.getIMEI();
      Serial.println("Modem's IMEI: " + IMEI);

    } else {
      Serial.println(F("Not connected to network"));
      delay(1000);
    }
  }
}

void getUTC () {
  // http://worldtimeapi.org/pages/examples
  // http://worldtimeapi.org/api/timezone/ETC/UTC
  // https://arduinojson.org/v6/example/http-client/
  GSMClient client;
  if (client.connect("worldtimeapi.org", 80)) {
    Serial.println(F("connected to World Time API"));
    client.print(F("GET "));
    client.print(F("/api/timezone/ETC/UTC"));
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(F("worldtimeapi.org"));
    client.println(F("Connection: close"));
    // Terminate headers
    client.println();

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    Serial.println(status);
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      client.stop();
      Serial.println(F("Not 200 OK"));
      return;
    }
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      client.stop();
      Serial.println(F("Somethings amiss with the HTTP headers"));
      return;
    }

    StaticJsonDocument<128> doc;
    StaticJsonDocument<128> filter;
    filter["unixtime"] = true;
    filter["utc_datetime"] = true;
    DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));


    /*
    DeserializationError error = deserializeJson(doc, client);
    */
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      client.stop();
      return;
    }
  
    // Extract values
    // Serial.println(F("Response:"));
    //Serial.println(doc["unixtime"].as<int>());
    // Serial.println(doc["utc_datetime"].as<String>());

  
    // Disconnect
    client.stop();
    start_time = doc["unixtime"].as<int>();
    msg += start_time;
    msg += " / ";
    msg += doc["utc_datetime"].as<String>();
    //Serial.println(msg);
    return;
  } else {
    Serial.println(F("Failed to connect to World Time API"));
  }
}

void makeWebRequest(GPSInfo gps_info) {
  GSMSSLClient client;

  Serial.println(F("connecting to REST API ..."));

  // https://arduinojson.org/v6/example/http-server/ (for sending json)
  // Allocate a temporary JsonDocument
  // One could use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<200> doc;
  doc["imei_string"] = IMEI;
  doc["uptime"] = msg;
  doc["latitude"] = gps_info.latitude;
  doc["longitude"] = gps_info.longitude;
  doc["altitude"] = gps_info.altitude;
  doc["start_time"] = start_time;


  if (client.connect(server, port)) {
    Serial.println(F("connected to REST API"));
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

    // Terminate headers
    client.println();

    // Send body
    serializeJsonPretty(doc, client);
    // serializeJsonPretty(doc, Serial);

    // pause waiting for response
    // delay(750);
    delay(2000);

    char status[96] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    client.stop();

    if (strcmp(status, "HTTP/1.1 201 Created") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    } else {
      Serial.print(F("Status: "));
      Serial.println(status);
    }
  } else {
    Serial.println(F("REST API connection failed"));
    client.stop();
    return;
  }

  Serial.println(F("Blinking  5 sec ... web request finished"));
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second

}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  delay(2000);

  Serial.println(F("Sketch: GsmWebClient_hourly_SSL_Post_GPS_UTC"));
}

void loop() {
  startModem();
  if (start_time == 0) {
      getUTC();
  }
  GPSInfo g = getGPSInfo();
  makeWebRequest(g);

  Serial.print(F("Sleeping for "));
  Serial.print(sleeping_ms);
  Serial.println(F(" ms Note: 3600000 is 1 hour"));
  Serial.println(F(""));
  delay(sleeping_ms);

}

/*
# vim: ai et ts=2 sw=2 sts=2 nu
*/
