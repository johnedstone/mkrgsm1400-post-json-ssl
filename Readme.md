### Notes
* Description: Arduino sketch for the MKR GSM 1400 (plus 2 SIM900 sketches) Arduino board for posting JSON data with SSL to a REST API
* Currently using
    * GsmWebClient_hourly_SSL_Post_GPS_UTC with Arduino MKR GPS
    * GsmWebClient_hourly_SSL_Post_ENV_UTC with Arduino MKR ENV Shield rev2 
    * *Bonus*: Adding an SSL/https GET sketch for a SIM900 `sim900_https_get_version_04`
    * *Bonus*: Adding an SSL/https POST sketch for a SIM900 `sim900_https_post_json_v01`
      Note that this sketch posts the IMEI, and the timestamp when the device boots,
      i.e. uptime: `timestamp`.  The json key "start_time" has no meaning for this sketch, its required by the REST API.  To do: figure out how to keep time on the SIM900 so that uptime and start_time have the actual values.

* For more history, and earlier scripts see [archive/Readme.md](archive)
* Can be powered with the PC USB or a 5 volt charger, e.g. Raspberry Pi power supply (either 5.1 Volts/3.5 Amps or 5.0 volts/2.5 amps)
* For related sketches see
    * [MKR NB 1500](https://github.com/johnedstone/MKR-NB-1500-sketches)
    * [GPy Pycom.io](https://github.com/johnedstone/pycom-gpy)

* __Comment out SSL cert validation in the library as detailed below__
Either of the following changes can be made:

#### Method #1
```
#file: libraries/MKRGSM/src/GSMClient.cpp
124     case CLIENT_STATE_MANAGE_SSL_PROFILE: {
125       // MODEM.sendf("AT+USECPRF=0,0,%d",_sslprofile);
126       MODEM.sendf("AT+USECPRF=0");
```

#### Method #2
```
#file: libraries/MKRGSM/src/GSMClient.cpp
54   //_sslprofile(1),
55   _sslprofile(0),
```

