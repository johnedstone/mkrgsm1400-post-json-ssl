/*
 * Sketch: NBWebClient_hourly_SSL_Post
 * Device: MKR NB 1500
 * SIM: hologram
 * Firmware:
 *
 * Purpose: POST JSON over SSL to a REST API server
 *
 * Development based on these sketches: 
 *  - libraries/MKRNB/examples/NBSSLWebClient
 *  - GSMWebClient_hourly_SSL_Post (https://github.com/johnedstone/mkrgsm1400-post-json-ssl)
 *
 * Several library modification needed
 *   First: remove libraries/MKRNB and
 *     replace with libraries/MKRNB-master from https://github.com/arduino-libraries/MKRNB
 *   Second: update libraries/MKRNB-master/src/Modem.cpp and
 *     as noted at https://forum.arduino.cc/t/mkr-1500-nb-hangs-on-nbaccess-begin/636736
 *     proposed by hakondahle,
 *     update int ModemClass::begin(bool restart) and
 *                   void ModemClass::end()
 *   Third (optionally): while waiting on modem firmware updating and/or getting Root certs loaded,
 *     disable cert validation:
 *     See this file: libraries/MKRNB-master/src/NBClient.cpp
 *       125 //MODEM.send("AT+USECPRF=0,0,1");
 *       126  MODEM.sendf("AT+USECPRF=0");
 *
 * Previous problems have been (which may now have been fixed):
 *   Test: previously this halted after 9x 1 hour iterations, and again 9x the second attempt
 *         changing to 3min to see if this can repeat
 *         Result: halted after 9 iterations
 *
 *   Problem: This code stops after 9x iterations whether it's 1 hour or 3 min
 *            Returns status but then fails to send the next command: AT+USOCL
 *            which is close socket. Argggg! : (
 *            This is what should happen, but the last two lines never appear
 *                +UUSORD: 0,195
 *                Status: HTTP/1.1 201 Created
 *                AT+USOCL=0
 *                OK
 * Changes that perhaps is enabling this to work are
 *   -- not sure
 *   -- carefully placed client stop, as if it weren't disconnected
 *   -- put initiating classes back inside the loop, instead of global to prevent corruption - a guesss
 * Worked:
 *    Powered by laptop USB
 *    Hard power off/on, i.e. not just uploaded sketch
 *    Adding modem.begin()
 *    Carefully placing client.stop()
 *    10sec, 20 iterations ok
 * Worked:
 *    Powered by laptop USB
 *    Hard power off/on, i.e. not just uploaded sketch
 *    3 min iterations
 *    60x no problems
 * Worked:
 *    Powered by laptop USB
 *    Hard power off/on, i.e. not just uploaded sketch
 *    30 min iterations
 *    44x no problems
 * Next:
 *    Powered with 5V 2.5A Pi Power Supply from Canakit
 *    Hard power off/on, i.e. not just uploaded sketch
 *    60 min iterations
 *    Install Let's Encrypt SSL Root Cert
 *
 */

#include <Arduino.h>
#include <UpTime.h>     // https://github.com/jozef/Arduino-UpTime
#include <MKRNB.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"

const char PINNUMBER[]     = SECRET_PINNUMBER;
int port = 443;

// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
//int sleeping_ms = 3561000;
//int sleeping_ms = 180000; // 3 min
int sleeping_ms = 1800000; // 30 min

char server[] = "your.rest.api";
char path[] = "/your/endpoint/to/post/";


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
  NBSSLClient client;
  GPRS gprs;
  NB nbAccess;
  NBModem modemTest;

  Serial.println(F("Starting NB/GPRS/NBSSLClient."));
  // connection state
  boolean connected = false;

  while (!connected) {
    Serial.println(F("one"));
    if ((nbAccess.begin(PINNUMBER) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
      Serial.println(F("two"));
    } else {
      // Untested: this is a "guess" as to what to do if we get here
      Serial.println(F("########## Modem/GPRS not connected: restarting modem ##############"));
      Serial.println(F("##########                   three                    ##############"));
      modemTest.begin();
      Serial.println(F("##########             Modem restarted!               ##############"));
      delay(1000);
    }
  }
  Serial.println(F("four"));

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

    // Question: where should this stop be?
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
    // if you didn't get a connection to the server:
    Serial.println(F("REST API connection failed"));
    client.stop();
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
