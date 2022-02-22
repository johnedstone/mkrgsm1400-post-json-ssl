/*
 * Sketch:
 * Arduino MKR GSM 1400
 * Antenna
 * SIM: Hologram
 * Arduino MKR GPS
 * Based on GsmWebClient_hourly_SSL_Post_with_Validation_Not sketch
*/

#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime
#include <MKRGSM.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

#include <Arduino_MKRGPS.h>

const char PINNUMBER[]     = SECRET_PINNUMBER;
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

int port = 443;

// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
int sleeping_ms = 3561000;
// int sleeping_ms = 180000; // 3 min

char server[] = "your.server.net";
char path[] = "/your/endpoint/"; // whatever your endpoint might be

String IMEI = "";

struct GPSInfo {
  float latitude;
  float longitude;
  float altitude;
  int   satellites;
};

GPSInfo getGPSInfo() {
  bool connected = false;

  // Let's make this "optional"
  //while (!connected) {
    if (GPS.begin(GPS_MODE_SHIELD)) {
      Serial.println(F("GPS initialized"));
      connected = true;
    } else {
      Serial.println("Failed to initialize GPS!");
    }
  //}

  GPSInfo g = {0.0, 0.0, 0.0, 0};
  Serial.print("wait location ... ");

  // wait for new GPS data to become available
  unsigned long startMillis = millis();
  while (!GPS.available());
  unsigned long endMillis = millis();

  Serial.print(endMillis - startMillis);
  Serial.println(" ms");

  // read GPS values
  /*
  float latitude   = GPS.latitude();
  float longitude  = GPS.longitude();
  float altitude   = GPS.altitude();
  int   satellites = GPS.satellites();
  */
  g.latitude   = GPS.latitude();
  g.longitude  = GPS.longitude();
  g.altitude   = GPS.altitude();
  g.satellites = GPS.satellites();

  // print GPS values
  /*
  Serial.println();
  Serial.print("Location: ");
  Serial.print(latitude, 7);
  Serial.print(", ");
  Serial.println(longitude, 7);

  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println("m");

  Serial.print("Number of satellites: ");
  Serial.println(satellites);

  Serial.println();
  */

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

void makeWebRequest(GPSInfo gps_info) {
  GSMSSLClient client;

  Serial.println(F("connecting to REST API ..."));

  // https://arduinojson.org/v6/example/http-server/ (for sending json)
  // Allocate a temporary JsonDocument
  // One could use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<200> doc;
  doc["imei_string"] = IMEI;
  doc["uptime"] = getUptime();
  doc["latitude"] = gps_info.latitude;
  doc["longitude"] = gps_info.longitude;
  doc["altitude"] = gps_info.altitude;


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

    // pause waiting for response
    // delay(750);
    delay(2000);

    // https://arduinojson.org/v6/example/http-client/
    // Check HTTP status
    // char status[32] = {0}; // Good for "HTTP/1.1 200 OK"

    /*
     * Debugging

    if (client.find("201")) {
      Serial.println("Found 201");
    } else {
      Serial.println("Not found 201");
    }
    Serial.println("Start");
    Serial.println("peek: " + client.peek());
    Serial.println(client.peek());
    Serial.println("End");
    while (true) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    
      // SSL hack: if there is just one byte left, force a stop
      // For SSL client.connected() will not go to false - bug?
      // "A client is considered connected if the connection
      // has been closed but there is still unread data"
      static bool force_client_stop = false;
      if (client.available() == 1 && client.connected()) {
        force_client_stop = true;
      }
      if (force_client_stop && client.available() == 0) {
        client.stop();
      }
    
      // if the server's disconnected, stop the client:
      if (!client.available() && !client.connected()) {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        return;
    
      }
    }
    // End Debugging
     */

    char status[96] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

    //Serial.println(status);
    //Serial.println(sizeof(status));

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

String getUptime() {
  return "uptime: "+uptime_as_string()+" or "+uptime()+"s";
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */
  delay(2000);

  Serial.println(F("Sketch: GsmWebClient_hourly_SSL_Post_GPS."));
}

void loop() {
  startModem();
  GPSInfo g = getGPSInfo();

  /*
  Serial.print("Location: ");
  Serial.print(g.latitude, 7);
  Serial.print(", ");
  Serial.println(g.longitude, 7);

  Serial.print("Altitude: ");
  Serial.print(g.altitude);
  Serial.println("m");

  Serial.print("Number of satellites: ");
  Serial.println(g.satellites);
  */

  makeWebRequest(g);

  Serial.println(getUptime());
  Serial.print(F("Sleeping for "));
  Serial.print(sleeping_ms);
  Serial.println(F(" ms Note: 3600000 is 1 hour"));
  Serial.println(F(""));
  delay(sleeping_ms);

  // do nothing:
  // while (true);
}

/*
# vim: ai et ts=2 sw=2 sts=2 nu
*/
