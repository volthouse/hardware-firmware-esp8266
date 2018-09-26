

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h> // settimeofday_cb()

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

#define TZ              1       // (utc+) TZ in hours
#define DST_MN 60 // use 60mn for summer time in some countries
#define RTC_TEST 1510592820 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

#define MAGIC_NO 0xABCDEFAB
#define SLEEP_SEC 15


const int led = 2;
timeval cbtime;      // time set in callback
bool cbtime_set = false;

timeval tv;
timespec tp;
time_t now;
uint32_t now_ms, now_us;

struct {
  int magic;
  int count;
  timeval tv;
} rtc_time_desc;


void handleRoot();              // function prototypes for HTTP handlers
void handleLED();
void handleNotFound();

void time_is_set(void) {
  gettimeofday(&cbtime, NULL);
  cbtime_set = true;
  Serial.println("------------------ settimeofday() was called ------------------");
}

// for testing purpose:
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

#define PTM(w) \
  Serial.print(":" #w "="); \
  Serial.print(tm->tm_##w);

void printTm(const char* what, const tm* tm) {
  Serial.print(what);
  PTM(isdst); PTM(yday); PTM(wday);
  PTM(year);  PTM(mon);  PTM(mday);
  PTM(hour);  PTM(min);  PTM(sec);
}


void add(const timeval& a, const timeval& b, timeval& result)

{

    result.tv_sec = a.tv_sec + b.tv_sec;

    result.tv_usec = a.tv_usec + b.tv_usec;

    if (result.tv_usec >= 1000000)

    {

        result.tv_sec++;

        result.tv_usec -= 1000000;

    }

}

void setup(void){

  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));
  
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  if(rtc_time_desc.magic != MAGIC_NO) {
    ESP.eraseConfig();
    time_t rtc = RTC_TEST;
    timeval tv = { rtc, 0 };
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&tv, &tz);

    
    memset(&rtc_time_desc, 0, sizeof(rtc_time_desc));
    rtc_time_desc.magic = MAGIC_NO;
    Serial.println("First Boot");
  } else {    
    timeval tv;
    timeval tv_add;
    memset(&tv_add, 0, sizeof(tv_add));
    tv_add.tv_sec = SLEEP_SEC;
    add(rtc_time_desc.tv, tv_add, tv);
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&tv, &tz);
  }

  Serial.println(String("RTC Count") + String(rtc_time_desc.count));

  pinMode(led, OUTPUT);


  WiFi.softAP("ESP-LED");
  
  Serial.println("Connecting ...");
  
  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", HTTP_GET, handleRoot);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/LED", HTTP_POST, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

}

void loop(void){
  server.handleClient();                    // Listen for HTTP requests from clients

  gettimeofday(&tv, nullptr);
  clock_gettime(0, &tp);
  now = time(nullptr);
  now_ms = millis();
  now_us = micros();



  // localtime / gmtime every second change
  static time_t lastv = 0;
  static int cnt = 0;
  
  if (lastv != tv.tv_sec) {
    lastv = tv.tv_sec;
    //Serial.println();
    //printTm("localtime", localtime(&now));    
    //Serial.println();
    char b[200];

    ctime_r(&now, b);

    Serial.println(b);
    cnt++;
  }

  if(cnt > 60) {
    cnt = 0;
    rtc_time_desc.count++;
    memcpy(&rtc_time_desc.tv, &tv, sizeof(tv));
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));
    ESP.deepSleep(SLEEP_SEC * 1e6, WAKE_RFCAL);    
  }

}

void handleRoot() {

  now = time(nullptr);

  char b[200];

  ctime_r(&now, b);


  
  // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\">" + String(b) + "</form>");
}

void handleLED() {                          // If a POST request is made to URI /LED
  digitalWrite(led,!digitalRead(led));      // Change the state of the LED
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect


}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
