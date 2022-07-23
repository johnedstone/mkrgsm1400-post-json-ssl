/* References from previous sim900_https_get_version_04
 *  http://www.raviyp.com/sim900-gprs-http-at-commands/
 *  https://simcom.ee/documents/SIM900/SIM900_HTTPS%20AT%20Command%20Set_V1%2000.pdf
 *  https://simcom.ee/documents/?sort_by=name&sort_as=desc&dir=SIM900/
 *  https://forum.arduino.cc/t/sim900-http-request-not-allowed/524505
 *  https://simcom.ee/documents/SIM900/SIM900_AT%20Command%20Manual_V1.11.pdf
 *  This sketch is also published on https://github.com/johnedstone/mkrgsm1400-post-json-ssl
 *  
 *  Refereneces for json post
 *  https://arduino.stackexchange.com/questions/75599/how-send-method-post-with-json-arduino-uno-r3-sim900 - useful
 *  https://forum.arduino.cc/t/read-and-save-imei-from-serial-port/517925/15 - did not needed
 *  https://forum.arduino.cc/t/arduino-serial-to-variable/37458 - did not use
 *  https://elementztechblog.wordpress.com/2016/12/28/getting-time-and-date-from-gsm-modem/ - writing permanently to modem
 *
 * To do: In this sketch the json key "uptime" is really the UTC timestamp when the device boots up.
 *        The json key "start_time" has no meaning - it is a required field for the rest-api.
 */
 
#include <SoftwareSerial.h>
#include "arduino_secrets.h"

char sketch_name[] = "sim900_https_post_json_v01";
char url_address[] = URL_ADDRESS;
String IMEI;
String UPTIME;
 
/*
 *
   60000:  1 min
  120000:  2 min
  300000:  5 min
  900000: 15 min
 3600000: 60 min
 */

unsigned long sleeping_ms = 3600000;

SoftwareSerial SIM900(7, 8);

void setup() {
  Serial.begin(9600);
  delay(4000);
  ShowSerialData(10);

  Serial.println(F("Goodnight moon!"));
  Serial.print(F("Sketch: "));
  Serial.println(sketch_name);
  Serial.print(F("The loop will sleep (ms): "));
  Serial.print(sleeping_ms);
  Serial.println(F(" when it is finished."));
  Serial.println(F(""));
  Serial.println(F("Sleeping 10 sec now, waiting for things to settle down"));
  Serial.println(F(""));
  delay(10000);


  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  Serial.println(F("Power should be on now, waiting 10 sec, again, for things to settle down"));
  Serial.println(F(""));
  ShowSerialData(10000);

  Serial.println(F("Software Serial has now beginning, waiting 10 sec, again, for things to settle down"));
  Serial.println(F(""));

  SIM900.begin(9600);

  ShowSerialData(10000);

  SIM900.println("ATE0");
  SIM900.flush();
  ShowSerialData(2000);

  Serial.println(F("Getting IMEI"));
  SIM900.println("AT+GSN");
  SIM900.flush();
  getIMEI(5000);
  Serial.print(F("IMEI: "));
  Serial.println(IMEI);
  ShowSerialData(4000);

  Serial.println(F("Getting UPTIME (TZ is in 1/4 hr increments)"));
  SIM900.println("AT+CLTS=1;&W"); // enable getting network time and store permanently in modem profile
  SIM900.flush();
  Serial.println(F("AT+CLTS=1;&W"));
  ShowSerialData(2000);
  Serial.println(F(""));

  SIM900.println("AT+CCLK?");
  SIM900.flush();
  Serial.println(F("AT+CCLK?"));
  getUPTIME(5000);
  Serial.print(F("uptime: "));
  Serial.println(UPTIME);
  ShowSerialData(4000);

  SIM900.println("ATE1");
  SIM900.flush();
  ShowSerialData(2000);
  
  delay(2000);
  Serial.println(F("Setup is finished. Entering the loop."));
}

void loop() {

  Serial.print(F("Sketch: "));
  Serial.println(sketch_name);

  connectGPRS();
  delay(4000);
  postData();

  delay(1000);
  Serial.print(F("Sleeping (ms): "));
  Serial.println(sleeping_ms);
  
  delay(sleeping_ms); // 60 min

}

void ShowSerialData(int delay_ms) {
  delay(delay_ms);
  while (SIM900.available() != 0)
    Serial.write(SIM900.read());
}

void getIMEI(int delay_ms) {
  delay(delay_ms);
  while(SIM900.available()!= 0) {
    IMEI = SIM900.readString();
    IMEI.replace("OK","");
    IMEI.trim();
  }
}

void getUPTIME(int delay_ms) {
  delay(delay_ms);
  while(SIM900.available()!= 0) {
    UPTIME = SIM900.readString();
    UPTIME.replace("OK","");
    UPTIME.replace("+CCLK:","");
    UPTIME.replace("\"","");
    UPTIME.trim();
  }
}

void connectGPRS() {
  SIM900.println("AT+CMEE=2"); // Turns on error reporting verbose
  SIM900.flush();
  ShowSerialData(1000);
  
  SIM900.println("AT+CSQ");  // Signal Quality Report
  SIM900.flush();
  ShowSerialData(1000);
  
  SIM900.println("AT+CGATT?");  // Attached to GPRS Service
  SIM900.flush();
  ShowSerialData(1000);
  
  SIM900.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); // -> OK
  SIM900.flush();
  ShowSerialData(1000);
  
  SIM900.println("AT+SAPBR=3,1,\"APN\",\"hologram\""); // -> OK
  SIM900.flush();
  ShowSerialData(1000);

  Serial.println(F(""));
  Serial.println(F("If the following returns an error, do not panic!"));
  Serial.println(F("It may simply mean that GPRS is already enabled."));
  Serial.println(F(""));

  SIM900.println("AT+SAPBR=1,1"); // Enable GPRS, Occassionally OK, but mostly "+CME ERROR: operation not allowed"
  SIM900.flush();
  ShowSerialData(6000);
  
  delay(2000);
  Serial.println(F("If the following does not return an IP address, yikes! Resert the device!"));
  SIM900.println("AT+SAPBR=2,1"); // Query the connection, if IP, then proceed :)
  SIM900.flush();
  ShowSerialData(4000);
}

void postData(){
  String json_data="{\"imei_string\": \"" + IMEI + "\"," + "\"uptime\": \"" + UPTIME + "\", \"start_time\": 100}";

  Serial.println(F(""));
  Serial.println(F("If the following returns an error, do not panic.!"));
  Serial.println(F("It may simply mean that the HTTP Service is initialized already."));
  Serial.println(F(""));

  SIM900.println("AT+HTTPINIT"); // OK
  SIM900.flush();
  ShowSerialData(2000);

  SIM900.println("AT+HTTPSSL=1"); // 
  SIM900.flush();
  ShowSerialData(2000);

  SIM900.println("AT+HTTPPARA=\"CID\",1"); // 
  SIM900.flush();
  ShowSerialData(2000);

  SIM900.print("AT+HTTPPARA=\"URL\",\"");
  SIM900.print(url_address);
  SIM900.println("\"");
  SIM900.flush();
  ShowSerialData(2000);
  Serial.println(F(""));


  SIM900.println("AT+HTTPPARA=\"CONTENT\", \"application/json\"");
  SIM900.flush();
  ShowSerialData(2000);

  Serial.println(F("Data (json)"));
  Serial.println(json_data);
  Serial.println(F(""));

  SIM900.println("AT+HTTPDATA=" + String(json_data.length()) + ",100000"); // may not need the 10000 (try someday w/o)
  SIM900.flush();
  ShowSerialData(2000);

  SIM900.println(json_data);
  SIM900.flush();
  ShowSerialData(4000);

  SIM900.println("AT+HTTPACTION=1"); // Start the POST -> OK +HTTPACTION:1,201,442 (POST,STATUS,Length)
  SIM900.flush();
  Serial.println(F("POST request completed, waiting 12 sec before reading the response."));
  Serial.println(F(""));
  ShowSerialData(12000);

  SIM900.println("AT+HTTPREAD"); // +HTTPREAD:442
  SIM900.flush();
  ShowSerialData(1000);

  Serial.println(F(""));
  SIM900.println("AT"); // OK (clear line ending)
  SIM900.flush();
  ShowSerialData(1000);

  SIM900.println("AT+HTTPSTATUS?"); // POST,0,0,0 - POST, idle, data transmitted, data remaining to be sent or received
  SIM900.flush();
  ShowSerialData(1000);

  SIM900.println("AT+HTTPSCONT?"); // Reads Status of HTTP Application Context
  SIM900.flush();
  ShowSerialData(2000);

  /*
  SIM900.println("AT+HTTPCONT"); // Saves the of HTTP Application Context to NVRAM, so will be save on next reboot
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  */

  Serial.println(F(""));

  SIM900.println("AT+HTTPTERM"); // Terminate any HTTP Service
  SIM900.flush();
  ShowSerialData(2000);

  SIM900.println("AT+SAPBR=0,1"); // Close Bearer
  SIM900.flush();
  ShowSerialData(1000);

}
