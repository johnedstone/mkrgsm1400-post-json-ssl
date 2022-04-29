### Notes
* Description: Arduino sketch for the MKR GSM 1400 Arduino board for posting JSON data with SSL to a REST API
* Currently using
    * GsmWebClient_hourly_SSL_Post_GPS_UTC with Arduino MKR GPS
    * GsmWebClient_hourly_SSL_Post_ENV_UTC with Arduino MKR ENV Shield rev2 

* For more history, and earlier scripts see [archive/Readme.md](archive)
* Can be powered with the PC USB or a 5 volt charger, e.g. Raspberry Pi power supply (either 5.1 Volts/3.5 Amps or 5.0 volts/2.5 amps)
* For related sketches see
    * [MKR NB 1500](https://github.com/johnedstone/MKR-NB-1500-sketches)
    * [GPy Pycom.io](https://github.com/johnedstone/pycom-gpy)
* __Comment out SSL cert validation in the library as detailed below__
```
#file: libraries/MKRGSM/src/GSMClient.cpp
124     case CLIENT_STATE_MANAGE_SSL_PROFILE: {
125       // MODEM.sendf("AT+USECPRF=0,0,%d",_sslprofile);
126       MODEM.sendf("AT+USECPRF=0");
```
