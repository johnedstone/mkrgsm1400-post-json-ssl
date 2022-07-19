/* References
 *  http://www.raviyp.com/sim900-gprs-http-at-commands/
 *  https://simcom.ee/documents/SIM900/SIM900_HTTPS%20AT%20Command%20Set_V1%2000.pdf
 *  https://simcom.ee/documents/?sort_by=name&sort_as=desc&dir=SIM900/
 *  https://forum.arduino.cc/t/sim900-http-request-not-allowed/524505
 *  This sketch is also published on https://github.com/johnedstone/mkrgsm1400-post-json-ssl
 */
 
#include <SoftwareSerial.h>
#include "arduino_secrets.h" 

char url_address[] = URL_ADDRESS;
  
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
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!
  
  SIM900.begin(9600);
  delay(6000);

  ShowSerialData(); // hoping to catch TZ information, to avoid garbage output (?)
  
  Serial.println(F("Goodnight moon!"));
  Serial.println(F("Sketch: sim900_https_get_version_04"));
  Serial.print(F("Sleeping (ms): "));
  Serial.println(sleeping_ms);

  ShowSerialData(); // hoping to catch TZ information, to avoid garbage output (?)

  SIM900.println("ATE");
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  
  delay(2000);
}

void loop() {

  Serial.println(F("Sketch: sim900_https_get_version_04"));

  connectGPRS();
  delay(2000);
  getRequest();

  delay(1000);
  Serial.print(F("Sleeping (ms): "));
  Serial.println(sleeping_ms);
  
  //delay( 60000); // 1 min
  //delay(120000); // 2 min
  //delay(300000); // 5 min
  //delay(900000); // 15 min
  delay(sleeping_ms); // 60 min

}

void ShowSerialData() {
  while(SIM900.available()!=0)
    Serial.write(SIM900.read());
}


void connectGPRS() {
  SIM900.println("AT+CMEE=2"); // Turns on error reporting verbose
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  
  SIM900.println("AT+CSQ");  // Signal Quality Report
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  
  SIM900.println("AT+CGATT?");  // Attached to GPRS Service
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  /*
   * Cleaned up at the end
  SIM900.println("AT+SAPBR=0,1"); // Close any previous bearer
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  */
  
  SIM900.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); // -> OK
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  
  SIM900.println("AT+SAPBR=3,1,\"APN\",\"hologram\""); // -> OK
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  Serial.println(F(""));
  Serial.println(F("If the following returns an error, do not panic!"));
  Serial.println(F("It may simply mean that GPRS is already enabled."));
  Serial.println(F(""));

  SIM900.println("AT+SAPBR=1,1"); // Enable GPRS, Occassionally OK, but mostly "+CME ERROR: operation not allowed"
  SIM900.flush();
  delay(4000);
  ShowSerialData();
  
  delay(2000);
  SIM900.println("AT+SAPBR=2,1"); // Query the connection, if IP, then proceed :)
  SIM900.flush();
  delay(4000);
  ShowSerialData();
}

void getRequest(){

  /*
   * Cleaned up at the end
  SIM900.println("AT+HTTPTERM"); // Terminate any preexisting HTTP Service
  SIM900.flush();
  delay(2000);
  ShowSerialData();
  */

  Serial.println(F(""));
  Serial.println(F("If the following returns an error, do not panic.!"));
  Serial.println(F("It may simply mean that the HTTP Service is initialized already."));
  Serial.println(F(""));

  SIM900.println("AT+HTTPINIT"); // OK
  SIM900.flush();
  delay(2000);
  ShowSerialData();

  SIM900.println("AT+HTTPSSL=1"); // 
  SIM900.flush();
  delay(2000);
  ShowSerialData();

  SIM900.println("AT+HTTPPARA=\"CID\",1"); // 
  SIM900.flush();
  delay(2000);
  ShowSerialData();

  SIM900.print("AT+HTTPPARA=\"URL\",\"");
  SIM900.print(url_address);
  SIM900.println("\"");
  SIM900.flush();
  delay(2000);
  ShowSerialData();
  Serial.println(F(""));

  SIM900.println("AT+HTTPACTION=0"); // Start the GET session -> OK +HTTPACTION:0,200,262 (GET,STATUS,Length)
  SIM900.flush();
  Serial.println(F("Get requests completed, waiting 10 sec before reading the response."));
  Serial.println(F(""));
  delay(10000);
  ShowSerialData();

  SIM900.println("AT+HTTPREAD"); // +HTTPREAD:362 
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  SIM900.println("AT"); // OK (clear line ending)
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  SIM900.println("AT+HTTPSTATUS?"); // GET,0,0,0 - GET, idle, data transmitted, data remaining to be sent or received
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  SIM900.println("AT+HTTPSCONT?"); // Reads Status of HTTP Application Context
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  /*
  SIM900.println("AT+HTTPCONT"); // Saves the of HTTP Application Context to NVRAM, so will be save on next reboot
  SIM900.flush();
  delay(1000);
  ShowSerialData();
  */

  SIM900.println("AT"); // OK (clear line ending)
  SIM900.flush();
  delay(1000);
  ShowSerialData();

  SIM900.println("AT+HTTPTERM"); // Terminate any HTTP Service
  SIM900.flush();
  delay(2000);
  ShowSerialData();

  SIM900.println("AT+SAPBR=0,1"); // Close Bearer
  SIM900.flush();
  delay(1000);
  ShowSerialData();

}
