#ifndef CONFIG_H
#define CONFIG_H

#ifdef WIFI_SSID
const char *SSID = WIFI_SSID;
#else
const char *SSID = "NETGEAR29";
#endif

#ifdef WIFI_PWD
const char *WIFI_PWD = WIFI_PWD;
#else
const char *WIFI_PWD = "purpleplum557";
#endif

// timezone Europe/Zurich as per https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIMEZONE "EET-2EEST,M3.5.0/3,M10.5.0/4"

#define UPDATE_INTERVAL_MINUTES 10

#endif