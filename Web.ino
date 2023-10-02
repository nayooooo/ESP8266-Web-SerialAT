#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <StreamString.h>
#include <FS.h>

#include "led.h"
#include "wifi.h"

#define USE_SERIAL_AT     (1)
#define USE_WEB_SERVER    (1)

#if USE_SERIAL_AT

#include "at_user.h"
String serialInput = "";
bool serialInputComplete = false;

void serialEvent(void)
{
    while (Serial.available()) {
        char chr = (char)Serial.read();
        if (chr == '\n') {
            serialInputComplete = true;
            continue;
        }
        serialInput += chr;
    }
}

#endif  // USE_SERIAL_AT

#if USE_WEB_SERVER

#include <ESP8266WebServer.h>
#include "fs_tools.h"

#define WEB_SERVER_PORT         (80)
ESP8266WebServer web_server(WEB_SERVER_PORT);

static void webHomePage_Refresh(void)
{
    struct { int hr; int min; int sec; } time;
    time.sec = millis() / 1000;
    time.min = time.sec / 60 % 60;
    time.hr = time.sec / (60 * 60);
    time.sec %= 60;

    StreamString message;
    message.clear();
    message.printf(""\
        "<html>"\
            "<head>"\
                "<meta http-equiv='refresh' content='5'/>"\
                "<title>ESP8266 Demo</title>"\
                "<style>"\
                    "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }"\
                "</style>"\
            "</head>"\
            "<body>"\
                "<h1>Hello from ESP8266!</h1>"\
                "<font size=\"5\">Uptime: %02d:%02d:%02d</font><br><br>"\
            "</body>"\
            "<button onclick=\"webHomePage_Refresh()\">"\
                "Click Me!"\
            "</button>"\
        "</html>", time.hr, time.min, time.sec);

    web_server.send(200, "text/html", message.c_str());
}

void webHomepage(void)
{
    webHomePage_Refresh();
    Serial.println("用户访问了主页。");
}

void webNotFound(void)
{
    String message = "Not Found\n\n";
    message += "URI: " + WiFi.localIP().toString() + web_server.uri() + "\n";
    message += "Method: ";
    message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\n";
    message += "Arguments: ";
    message += web_server.args();
    message += "\n";
    for (uint8_t i = 0; i < web_server.args(); i++) {
        message += "\t" + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
    }
    web_server.send(404, "text/plain", message);
}

#endif  // USE_WEB_SERVER

/*============================================================
    Wi-Fi
============================================================*/

String wifi_ssid = "MM67987";
String wifi_psk = "XXYJXZ316";

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200UL);

    if (ESP8266_WiFi_STA_Init(wifi_ssid, wifi_psk)) {
        Serial.println();
        Serial.println("Connect failed!");
    }

    #if USE_WEB_SERVER

    Serial.println("SPIFFS format start");
    SPIFFS.format();
    Serial.println("SPIFFS format finish");

    if (0) {
        StreamString message;
        message.clear();
        message.print(""\
            "<html>"\
                "<head>"\
                    "<meta http-equiv='refresh' content='5'/>"\
                    "<title>ESP8266 Demo</title>"\
                    "<style>"\
                        "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }"\
                    "</style>"\
                "</head>"\
                "<body>"\
                    "<h1>Hello from ESP8266!</h1>"\
                "</body>"\
                "<button onclick=\"webHomePage_Refresh()\">"\
                    "Click Me!"\
                "</button>"\
            "</html>");
        fs_tools_writeFile("/Web/001/page/home/page.txt", message.c_str());
    }
    File f = SPIFFS.open("/Web/001/page/home/page.txt", "r");
    Serial.printf("file size: %d\r\n", (int)f.count());
    f.close();

    if (WiFi.isConnected()) {
        web_server.on("/", webHomepage);
        web_server.onNotFound(webNotFound);
        web_server.begin();
        Serial.println("HTTP server started!");
    } else {
        Serial.println("Can't start HTTP server!");
    }

    #endif  // USE_WEB_SERVER
}

void loop() {
    // put your main code here, to run repeatedly:
    #if USE_SERIAL_AT

    if (serialInputComplete) {
        serialInputComplete = false;
        At_Err_t ret = at_user.At_Handle(serialInput);
        if (ret != AT_OK) {
            Serial.println();
            Serial.println("[AT] error: " + at_user.At_ErrorToString(ret));
        }
        serialInput = "";
    }

    #endif  // USE_SERIAL_AT

    #if USE_WEB_SERVER

    if (WiFi.isConnected()) {
        web_server.handleClient();
    }

    #endif  // USE_WEB_SERVER
}
