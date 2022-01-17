/*
 * Sketch: NBWebClient_hourly_SSL_Post
 * MKR NB 1500 board
 *
 * Based on: 
 *  - libraries/MKRNB/examples/NBSSLWebClient
 *  - GSMWebClient_hourly_SSL_Post (https://github.com/johnedstone/mkrgsm1400-post-json-ssl)
 *
 * Several library modification needed
 *   First: remove libraries/MKRNB and
 *     replace with libraries/MKRNB-master from https://github.com/arduino-libraries/MKRNB
 *   Second: update libraries/MKRNB-master/src/Modem.cpp as
 *     as noted at https://forum.arduino.cc/t/mkr-1500-nb-hangs-on-nbaccess-begin/636736
 *     proposed by hakondahle.
 *     Namely update int ModemClass::begin(bool restart) and
 *                   void ModemClass::end()
 *   Third: while waiting on modem firmware updating and/or getting Root certs loaded,
 *     disable cert validation:
 *     See this file: libraries/MKRNB-master/src/NBClient.cpp
 *      125 //MODEM.send("AT+USECPRF=0,0,1");
 *      126  MODEM.sendf("AT+USECPRF=0");
 */

#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime
#include <MKRNB.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"

const char PINNUMBER[]     = SECRET_PINNUMBER;
int port = 443;

// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
int sleeping_ms = 3561000;
// int sleeping_ms = 180000; // 3 min

char server[] = "your.rest.api";
char path[] = "/your/endpoint/to/post/";

NBSSLClient client;
GPRS gprs;
NB nbAccess;
NBModem modemTest;

void setup() {
  Serial.begin(9600);
  delay(2000);

  Serial.println(F("Sketch: NBWebClient_hourly_SSL_Post"));
}

void loop() {
  postData();

  Serial.print(F("Sleeping for "));
  Serial.print(sleeping_ms);
  Serial.println(F(" ms Note: 3600000 is 1 hour"));
  delay(sleeping_ms);

  //while (true);
}

void postData() {

  Serial.println(F("Starting Arduino SSL web client."));
  // connection state
  boolean connected = false;

  // After starting the modem with NB.begin()
  // attach to the GPRS network with the APN, login and password
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println(F("Modem/GPRS not connected"));
      delay(1000);
    }
  }

  // https://arduinojson.org/v6/example/http-server/ (for sending json)
  // Allocate a temporary JsonDocument
  // One could use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<200> doc;
  doc["imei_string"] = modemTest.getIMEI();
  doc["uptime"] = getUptime();
  Serial.println(F("connecting..."));

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println(F("Connected to REST API"));
    client.print(F("POST "));
    client.print(path);
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(server);

    // https://forum.arduino.cc/t/how-to-make-a-post-request-with-mkrgsm-gsmsslclient-library/501250/13
    // https://forum.arduino.cc/t/tomcat-arduino-gsm-shield-post-json/387640
    client.println(F("Content-Type: application/json;charset=UTF-8"));
    client.println(F("Connection: close"));
    client.print(F("Content-Length: "));
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
    char status[96] = {0};
    client.readBytesUntil('\r', status, sizeof(status));

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
    client.stop();

  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("REST API connection failed"));
  }

  serializeJsonPretty(doc, Serial);
  Serial.println(F(""));

  Serial.print(F("ICCID: "));
  Serial.println(modemTest.getICCID());
  Serial.println(F("... done for now"));
}

String getUptime() {
  return "uptime: "+uptime_as_string()+" or "+uptime()+"s";
}

/*
# vim: ai et ts=2 sw=2 sts=2 nu
*/
