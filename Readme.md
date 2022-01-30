### Notes
* Description: Arduino sketch for the MKR GSM 1400 Arduino board for posting JSON data with SSL to a REST API
* Currently, the most robust sketch (running now for 30d+) is `GsmWebClient_hourly_SSL_Post`
* For more history, and earlier scripts see [archive/Readme.md](archive)
* Can be powered with the PC USB or a 5 volt charger, e.g. Raspberry Pi power supply (either 5.1 Volts/3.5 Amps or 5.0 volts/2.5 amps)
* For related sketches see
    * [MKR NB 1500](https://github.com/johnedstone/MKR-NB-1500-sketches)
    * [GPy Pycom.io - tbd](./)
* Commenting out SSL cert validation here:
```
#file: libraries/MKRGSM/src/GSMClient.cpp
124     case CLIENT_STATE_MANAGE_SSL_PROFILE: {
125       // MODEM.sendf("AT+USECPRF=0,0,%d",_sslprofile);
126       MODEM.sendf("AT+USECPRF=0");
```
* Perhaps equal to, or better than the sketch GsmWebClient_hourly_SSL_Post, will be `GsmWebClient_hourly_SSL_Post_with_Validation_Not`, which has been running for 7d+
