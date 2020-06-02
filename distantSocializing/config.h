/******************************* WIFI **************************************/
// Fill in your wifi SSID and password below
const char* wifi_ssid = "Mirada672";  // network name -- put your WiFi SSID here
const char* wifi_password = "672261612";     // network password -- enter your WiFi password (if one is needed)

/************************ Server Config *******************************/

const String io_key = "aio_BewW73swXNllUAfHpcKR607dQXAD";  // YOUR ADAFRUIT IO KEY - YOU MUST REPLACE
const String io_host = "io.adafruit.com";
const int io_port = 443;
const String io_username = "kimanh";          
const String io_feedkey = "activity";         
const String post_io_feed_path = "/api/v2/" + io_username + "/feeds/" + io_feedkey + "/data";
const String get_io_feed_path = "/api/v2/" + io_username + "/feeds/" + io_feedkey + "/data/retain";

/************************ Your Name *******************************/
const String your_name = "Cesar";
