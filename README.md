## Description
The arduino sketch, GsmWebClient_hourly_SSL_Post,
demonstrates how to send JSON data with an HTTP POST request using SSL/TLS.
The other two sketches were used for developing the first sketch.

### List of projects
* GsmWebClient_hourly: something simple, not a POST, not SSL, send a GET request every hour.
* GsmWebClient_hourly_SSL: something simple, not a POST, using SSL, sending a GET request every hour.
    * This sketch uses the SSL work-around described below in the MKRGSM library
    which disables cert validation
    * This sketch also uses the hack described below, in the sketch, to force the client to stop. 
* GsmWebClient_hourly_SSL_POST (currently deployed): sending JSON data with an HTTP POST using SSL.
    * This sketch uses the SSL work-around described below in the MKRGSM library
    which disables cert validation
    * This sketch **does not** use the hack described below for forcing the client to stop
    because the data is read differently.
* Note: these sketches also print to the second serial port on the MKR GSM 1400 board, pins 13, 14 and the ground
* Note: the GsmWebClient_hourly_SSL_Post sketch posts uptime and the IMEI string.

### Device details
* Board: Arduino MKR GSM 1400
* Power:
    * The USB port on the PC works
    * A Raspberry Pi power supply (either 5.1 Volts/3.1 Amps or 5.1 volts/2.1 amps)
    * The USB charger, [for example: Anker Quick Charge 3.0 39W Dual USB Wall Charger](https://www.amazon.com/gp/product/B01IUSYF8G/),
    that was used appeared to restart the device after the POST
    as if it's powering off, then on again: *do not use*. 
    * This board and sketch only use 0.130 amps at it's peak, i.e. momentarily.
* SIM: [Hologram](https://www.hologram.io/)

### Information on the SSL bug and writing the work-around
* [For SSL requests, disable cert validation](https://arduino.stackexchange.com/questions/60443/arduino-mkr-gsm-1400-ssl-client-example-fails-to-connect)
There appears to be a bug in the MKRGSM library that prevents
the sketch from connecting (consistently).
Even the example provided by the library fails to connect.
The workaround is to disable cert validation (see reference):
```
#file: libraries/MKRGSM/src/GSMClient.cpp
124     case CLIENT_STATE_MANAGE_SSL_PROFILE: {
125       // MODEM.sendf("AT+USECPRF=0,0,%d",_sslprofile);
126       MODEM.sendf("AT+USECPRF=0");
```

* This hack below has been added to the sketch GsmWebClient_hourly_SSL.
The issue was first noted in the GsmSSLWebClient example 
which fails to reach the end of the code, i.e. `disconnecting`. The
reason for hanging is that in the example sketch `client.connected()`
never returns false.  There appears to be unread data, i.e. one last byte, that is not
being read. *This hack is not needed for GsmWebClient_hourly_SSL_POST sketch
because the data is read differently (however the workaround in the MKRGSM
library is still needed*
```
128   // SSL hack: if there is just one byte left, force a stop
129   // For SSL client.connected() will not go to false - bug?
130   // "A client is considered connected if the connection
131   // has been closed but there is still unread data"
132   if (client.available() == 1 && client.connected()) {
133     force_client_stop = true;
134   }
135   if (force_client_stop && client.available() == 0) {
136     client.stop();
137   }
```

### References
Here are some references that were helpful along the way!
* [Pre-flight checklist](https://www.tigoe.com/pcomp/code/arduinowiring/1337/)
* [Background on writing json](https://forum.arduino.cc/t/tomcat-arduino-gsm-shield-post-json/387640)
* [JsonHttpClient.ino](https://arduinojson.org/v6/example/http-client/)
* [ArduinoJson 6 release notes](https://arduinojson.org/news/2018/06/07/version-6-0-0/)
* [JsonGeneratorExample.ino](https://arduinojson.org/v6/example/generator/)
* [The best way to use ArduinoJson](https://arduinojson.org/v6/how-to/reuse-a-json-document/)
* [How to send json data](https://arduinojson.org/v6/example/http-server/)
* [Lots of examples, but missing sending json data](https://arduinogetstarted.com/tutorials/arduino-http-request)

<!---
# vim: ai et ts=4 sw=4 sts=4 nu
-->
