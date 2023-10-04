#include "HardwareSerial.h"
#include "at_user.h"
#include <ESP8266WiFi.h>

#include <string.h>
using namespace std;
#include "led.h"
#include "wifi.h"

/*============================================================
    AT
============================================================*/

At_Err_t at_user_AT_NULL(int argc, char *argv[]);
At_Err_t at_user_AT(int argc, char *argv[]);
At_Err_t at_user_AT_List(int argc, char *argv[]);
At_Err_t at_user_AT_Reboot(int argc, char *argv[]);
At_Err_t at_user_AT_LED(int argc, char *argv[]);
At_Err_t at_user_AT_FS(int argc, char *argv[]);
At_Err_t at_user_AT_WiFi_Connect(int argc, char *argv[]);
At_Err_t at_user_AT_WiFi_Get_IP(int argc, char *argv[]);
At_Err_t at_user_AT_Ping(int argc, char *argv[]);

struct At_State atTable[] = {
    { "AT", AT_TYPE_CMD, at_user_AT },
    { "AT+LS", AT_TYPE_CMD, at_user_AT_List },
    { "AT+REBOOT", AT_TYPE_CMD, at_user_AT_Reboot },
    { "AT+LED", AT_TYPE_CMD, at_user_AT_LED },
    { "AT+FS", AT_TYPE_CMD, at_user_AT_FS },
    { "AT+WIFI", AT_TYPE_CMD, at_user_AT_WiFi_Connect },
    { "AT+IP", AT_TYPE_CMD, at_user_AT_WiFi_Get_IP },
    { "AT+PING", AT_TYPE_CMD, at_user_AT_Ping },
    { AT_LABLE_TAIL, AT_TYPE_NULL, at_user_AT_NULL },
};

At_Class at_user(atTable);

At_Err_t at_user_AT_NULL(int argc, char *argv[]) {
  return AT_OK;
}

At_Err_t at_user_AT(int argc, char *argv[]) {
  at_user.At_Send_Infor(AT_USER_OK);

  return AT_OK;
}

/**
 * @brief 打印命令列表，同时另外发送指定位置(argv[i])的命令
 */
At_Err_t at_user_AT_List(int argc, char *argv[]) {
  String infor = "";
  const At_State_t pAtState = at_user.At_Get_State_Table();
  for (int i = 0; pAtState[i].atLable != AT_LABLE_TAIL; i++) {
    infor += (String)("[") + String(i) + "]: " + pAtState[i].atLable + "\r\n";
  }
  at_user.At_Send_Infor(infor);

  return AT_OK;
}

#include <EspSaveCrash.h>  // 重启
At_Err_t at_user_AT_Reboot(int argc, char *argv[])
{
    ESP.reset();
    return AT_OK;
}

#define AT_USER_LED_INITED    (0X01)
static struct LED _led = {
  .pin = 2,
  .state = LED_OFF,
  .mode = OUTPUT,
  .flag = 0,
};

At_Err_t at_user_AT_LED(int argc, char *argv[])
{
  String infor = "";
  if ((argc != 1) && (argc != 2)) {
    infor += "AT+LED\r\n";
    infor += "\tSTATE\r\n";
    infor += "\tNUM STATE\r\n";
    at_user.At_Send_Infor(infor);
    return AT_ERROR;
  }
  LED_t led = &_led;
  uint8_t led_pin_ind = argc - 2;
  uint8_t led_state_ind = argc - 1;
  if (strcmp(argv[led_state_ind], "ON") && strcmp(argv[led_state_ind], "OFF")) {
    infor += AT_USER_ERROR_ARGC;
    at_user.At_Send_Infor(infor);
    return AT_ERROR;
  }
  if (led_pin_ind == 0) {
    led->pin = (uint8_t)(String(argv[led_pin_ind]).toInt());
    led->flag &= ~AT_USER_LED_INITED;
  }
  if (!(led->flag & AT_USER_LED_INITED)) {  // 尚未初始化
    pinMode(led->pin, led->mode);
    led->flag |= AT_USER_LED_INITED;
  }
  if (!strcmp(argv[led_state_ind], "ON")) {
    led->state = LED_ON;
    digitalWrite(led->pin, led->state);
  } else if (!strcmp(argv[led_state_ind], "OFF")) {
    led->state = LED_OFF;
    digitalWrite(led->pin, led->state);
  }
  infor += AT_USER_OK;
  at_user.At_Send_Infor(infor);

  return AT_OK;
}

#include "fs_tools.h"
typedef struct At_State At_FS_State;

static At_Err_t _at_user_AT_FS_info(int argc, char* argv[])
{
    if (fs_tools_FS_info()) return AT_ERROR;
    return AT_OK;
}

static At_Err_t _at_user_AT_FS_print_directory(int argc, char* argv[])
{
    // 没有参数，则是打印FS中的所有文件
    // 有参数，则是打印FS中，该参数目录下的文件
    if (argc > 1) return AT_ERROR;
    if (argc == 0) {
        if (fs_tools_print_directory("")) return AT_ERROR;
    } else if (argc == 1) {
        if (fs_tools_print_directory(argv[0])) return AT_ERROR;
    } else return AT_ERROR;
    return AT_OK;
}

static At_Err_t _at_user_AT_FS_print_content(int argc, char* argv[])
{
    if (argc == 0) return AT_ERROR;
    if (argv[0] == "") return AT_ERROR;

    String content = "";
    int ret = fs_tools_readFile(argv[0], content);
    if (ret) {
        Serial.println(String("The file(") + argv[0] + ") has something error!");
        return AT_ERROR;
    }
    if (content != "") {
        Serial.println(String("The file(") + argv[0] + ") content: ");
        Serial.println(content);
    } else {
        Serial.println(String("The file(") + argv[0] + ") is empty!");
    }

    return AT_OK;
}

static At_FS_State atFSTable[] = {
    { "info", AT_TYPE_CMD, _at_user_AT_FS_info },
    { "prdir", AT_TYPE_CMD, _at_user_AT_FS_print_directory },
    { "prcon", AT_TYPE_CMD, _at_user_AT_FS_print_content },
    { AT_LABLE_TAIL, AT_TYPE_NULL, at_user_AT_NULL },
};
At_Err_t at_user_AT_FS(int argc, char *argv[])
{
    if (argc == 0) {
    AT_FS_set_not_found:
        Serial.println();
        Serial.println("please typing like the below: ");
        Serial.println("AT+FS");
        uint32_t i = 0;
        while (atFSTable[i].atLable != AT_LABLE_TAIL) {
            Serial.println(String("--") + atFSTable[i].atLable);
            i++;
        }
        return AT_ERROR;
    }
    
    String cmd = String(argv[0]);
    uint32_t i = 0;
    while (atFSTable[i].atLable != AT_LABLE_TAIL) {
        if (atFSTable[i].atLable == cmd) {
            if (atFSTable[i].act == nullptr) return AT_ERROR;
            return atFSTable[i].act(argc - 1, argv + 1);
        }
        i++;
    }

    goto AT_FS_set_not_found;
}

At_Err_t at_user_AT_WiFi_Connect(int argc, char *argv[]) {
  String infor = "";
  if (argc != 2) {
    infor += "AT+WIFI\r\n";
    infor += "\tSSID PSK\r\n";
    at_user.At_Send_Infor(infor);
    return AT_ERROR;
  }
  WiFi.disconnect();
  int ret = ESP8266_WiFi_STA_Init(argv[0], argv[1]);
  if (ret) {
    infor += AT_USER_ERROR_FUNCTION;
    at_user.At_Send_Infor(infor);
    return AT_ERROR;
  }
  infor += AT_USER_OK;
  at_user.At_Send_Infor(infor);

  return AT_OK;
}

At_Err_t at_user_AT_WiFi_Get_IP(int argc, char *argv[]) {
  String infor = "";
  infor = WiFi.localIP().toString();
  at_user.At_Send_Infor(infor);

  return AT_OK;
}

#include <Pinger.h>
extern "C"
{
    #include <lwip/icmp.h>
}
static Pinger pinger;
static bool pinger_set_ok = false;
At_Err_t at_user_AT_Ping(int argc, char *argv[])
{
    if (!WiFi.isConnected()) {
        Serial.println("Wi-Fi is not connect!");
        return AT_ERROR;
    }

    if (!pinger_set_ok) {
        pinger.OnReceive([](const PingerResponse& response){
            if (response.ReceivedResponse) {
                Serial.printf("Reply from %s: bytes=%d time=%lums TTL=%d\r\n",
                    response.DestIPAddress.toString().c_str(),
                    response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
                    response.ResponseTime,
                    response.TimeToLive);
            } else {
                Serial.println("Request timed out.");
            }
            return true;
        });

        pinger.OnEnd([](const PingerResponse& response){
            float loss = 100;  // 丢失百分比
            if (response.TotalReceivedResponses > 0) {
                loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
            }
            Serial.printf("Ping statistics for %s:\r\n",
                response.DestIPAddress.toString().c_str());
            Serial.printf("\tPackets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\r\n",
                response.TotalSentRequests,
                response.TotalReceivedResponses,
                response.TotalSentRequests - response.TotalReceivedResponses,
                loss);
            if (response.TotalReceivedResponses > 0) {
                Serial.println("Approximate round trip times in milli-seconds:");
                Serial.printf("\tMinimum = %lums, Maximum = %lums, Average = %.2fms\r\n",
                    response.MinResponseTime,
                    response.MaxResponseTime,
                    response.AvgResponseTime);
            }
            Serial.println("Destination host data:");
            Serial.printf("\tIP address: %s\r\n",
                response.DestIPAddress.toString().c_str());
            if (response.DestMacAddress != nullptr) {
                Serial.printf("\tMAC address: " MACSTR "\r\n",
                    MAC2STR(response.DestMacAddress->addr));
            }
            if (response.DestHostname != "") {
                Serial.printf("\tDNS name: %s\r\n",
                    response.DestHostname.c_str());
            }
            return true;
        });

        pinger_set_ok = true;
    }

    if (argc == 0) {
        if (pinger.Ping(WiFi.gatewayIP()) == false) {
            Serial.println("Error during last ping command.");
        }
    } else if (argc == 1) {
        if (pinger.Ping(argv[0]) == false) {
            Serial.println("Error during last ping command.");
        }
    } else {
        Serial.println("arg number error!");
        return AT_ERROR;
    }

    return AT_OK;
}
