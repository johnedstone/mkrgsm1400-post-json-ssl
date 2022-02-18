### Notes
* Description: Arduino sketch for the MKR GSM 1400 Arduino board for posting JSON data with SSL to a REST API
* Current two sketchs:
    * GsmWebClient_hourly_SSL_Post_GPS
    * GsmWebClient_hourly_SSL_Post_GPS_UTC
    * 
* For more history, and earlier scripts see [archive/Readme.md](archive)
* Can be powered with the PC USB or a 5 volt charger, e.g. Raspberry Pi power supply (either 5.1 Volts/3.5 Amps or 5.0 volts/2.5 amps)
* For related sketches see
    * [MKR NB 1500](https://github.com/johnedstone/MKR-NB-1500-sketches)
    * [GPy Pycom.io - tbd](./)
* Commented out SSL cert validation in the library as detailed below
```
#file: libraries/MKRGSM/src/GSMClient.cpp
124     case CLIENT_STATE_MANAGE_SSL_PROFILE: {
125       // MODEM.sendf("AT+USECPRF=0,0,%d",_sslprofile);
126       MODEM.sendf("AT+USECPRF=0");
```
* Perhaps equal to, or better than the sketch `GsmWebClient_hourly_SSL_Post`, will be [`archive/GsmWebClient_hourly_SSL_Post_with_Validation_Not`](archive), which has been running for 7d+

### Uptime issues
* MKR GSM 1400 `GsmWebClient_hourly_SSL_POST`: stopped at `"uptime": "uptime: 49d 16:23:13 or 4292593s",`
