#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>

#include "led.h"
#include "wifi.h"
#include "fs_tools.h"

#define USE_SERIAL_AT     (1)  // 使用串口作为AT的指令获取设备
#define USE_WEB_SERVER    (1)

#if USE_SERIAL_AT

#include "at_user.h"

#endif  // USE_SERIAL_AT

#if USE_WEB_SERVER

#include <ESP8266WebServer.h>

#define WEB_SERVER_PORT         (80)
ESP8266WebServer web_server(WEB_SERVER_PORT);

#define WEB_PAGE_FOLDER_PATH                ((String)("/page"))
#define WEB_IMG_FOLDER_PATH                 ((String)("/img"))
#define WEB_PAGE_NOTFOUND_FILE_NAME         ((String)("notFound.html"))
#define WEB_PAGE_NOTFOUND_FILE_PATH         (WEB_PAGE_FOLDER_PATH + "/" + WEB_PAGE_NOTFOUND_FILE_NAME)
#define WEB_PAGE_HOMEPAGE_FILE_NAME         ((String)("homePage.html"))
#define WEB_PAGE_HOMEPAGE_FILE_PATH         (WEB_PAGE_FOLDER_PATH + "/" + WEB_PAGE_HOMEPAGE_FILE_NAME)

void web_notFound(void)
{
    String message;
    fs_tools_readFile(WEB_PAGE_NOTFOUND_FILE_PATH, message);
    web_server.send(404, "text/html", message);
}

void web_homePage(void)
{
    web_server.sendHeader("Content-Type", "text/html; charset=utf-8");
    String message;
    fs_tools_readFile(WEB_PAGE_HOMEPAGE_FILE_PATH, message);
    web_server.send(200, "text/html", message.c_str());
    
    Serial.println("用户访问了主页。");
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

    if (WiFi.isConnected()) {
        if(fs_tools_FS_begin()) {
            web_server.serveStatic("/download", SPIFFS, "/");
            fs_tools_FS_end();
        }
        web_server.on("/", web_homePage);
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

    at.handleAuto();

    #endif  // USE_SERIAL_AT

    #if USE_WEB_SERVER

    if (WiFi.isConnected()) {
        web_server.handleClient();
    }

    #endif  // USE_WEB_SERVER
}
