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

const char PINNUMBER[]     = SECRET_PINNUMBER;
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

int port = 80;
// Note: 3600000 is 1 hour.  Currently sleeping 1 hour-27 sec
int sleeping_ms = 3573000;
char server[] = "example.org";
char path[] = "/";

GSMClient client;
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
      // Serial.println(GPRS_READY);
      // Serial1.println(GPRS_READY);
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

  if (client.connect(server, port)) {
    Serial.println(F("connected to Rest API"));
    Serial1.println(F("connected to Rest API"));
    client.print(F("GET "));
    client.print(path);
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(server);
    client.println(F("Connection: close"));
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
    Serial1.println(F("connection failed"));
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

  Serial.println(F("Starting Arduino web client: GsmWebClient_hourly."));
  Serial1.println(F("Starting Arduino web client: GsmWebClient_hourly."));

  startModem();
  makeWebRequest();
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
    Serial1.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected()) {
    Serial.println();
    Serial.println(F("disconnecting."));
    Serial1.println();
    Serial1.println(F("disconnecting."));
    client.stop();


    // do nothing forevermore:
    //for (;;)
    //  ;
    Serial.println(getUptime());
    Serial1.println(getUptime());
    Serial.print(F("Sleeping for "));
    Serial.print(sleeping_ms);
    Serial.println(F(" ms Note: 3600000 is 1 hour"));
    Serial1.print(F("Sleeping for "));
    Serial1.print(sleeping_ms);
    Serial1.println(F(" ms Note: 3600000 is 1 hour"));
    // sleep 60 min minus 27 sec
    delay(sleeping_ms);

    startModem();
    makeWebRequest();
  }
}
