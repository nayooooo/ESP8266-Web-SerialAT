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

At_Err_t at_user_AT_NULL(At_Param_t param);
At_Err_t at_user_AT(At_Param_t param);
At_Err_t at_user_AT_List(At_Param_t param);
At_Err_t at_user_AT_Reboot(At_Param_t param);
At_Err_t at_user_AT_LED(At_Param_t param);
At_Err_t at_user_AT_FS(At_Param_t param);
At_Err_t at_user_AT_WiFi_Connect(At_Param_t param);
At_Err_t at_user_AT_WiFi_Get_IP(At_Param_t param);
At_Err_t at_user_AT_Ping(At_Param_t param);

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

At at(atTable, Serial, Serial);

At_Err_t at_user_AT_NULL(At_Param_t param) {
  return AT_EOK;
}

At_Err_t at_user_AT(At_Param_t param) {
  at.sendInfor(AT_USER_OK);
  return AT_EOK;
}

/**
 * @brief 打印命令列表，同时另外发送指定位置(argv[i])的命令
 */
At_Err_t at_user_AT_List(At_Param_t param) {
  at.printSet("AT");
  at.sendInfor(AT_USER_OK);
  return AT_EOK;
}

#include <EspSaveCrash.h>  // 重启
At_Err_t at_user_AT_Reboot(At_Param_t param) {
  at.sendInfor(AT_USER_OK);
  ESP.reset();
  return AT_EOK;
}

#define AT_USER_LED_INITED (0X01)
static struct LED _led = {
  .pin = 2,
  .state = LED_OFF,
  .mode = OUTPUT,
  .flag = 0,
};

At_Err_t at_user_AT_LED(At_Param_t param) {
  String infor = "";
  if ((param->argc != 1) && (param->argc != 2)) {
    infor += "AT+LED\r\n";
    infor += "\tSTATE\r\n";
    infor += "\tNUM STATE\r\n";
    at.sendInfor(infor);
    return AT_ERROR;
  }
  LED_t led = &_led;
  uint8_t led_pin_ind = param->argc - 2;
  uint8_t led_state_ind = param->argc - 1;
  if (strcmp(param->argv[led_state_ind], "ON") && strcmp(param->argv[led_state_ind], "OFF")) {
    infor += AT_USER_ERROR_ARGC;
    at.sendInfor(infor);
    return AT_ERROR;
  }
  if (led_pin_ind == 0) {
    led->pin = (uint8_t)(String(param->argv[led_pin_ind]).toInt());
    led->flag &= ~AT_USER_LED_INITED;
  }
  if (!(led->flag & AT_USER_LED_INITED)) {  // 尚未初始化
    pinMode(led->pin, led->mode);
    led->flag |= AT_USER_LED_INITED;
  }
  if (!strcmp(param->argv[led_state_ind], "ON")) {
    led->state = LED_ON;
    digitalWrite(led->pin, led->state);
  } else if (!strcmp(param->argv[led_state_ind], "OFF")) {
    led->state = LED_OFF;
    digitalWrite(led->pin, led->state);
  }
  infor += AT_USER_OK;
  at.sendInfor(infor);

  return AT_EOK;
}

#include "fs_tools.h"
typedef struct At_State At_FS_State;

static At_Err_t _at_user_AT_FS_info(At_Param_t param) {
  if (fs_tools_FS_info()) return AT_ERROR;
  return AT_EOK;
}

static At_Err_t _at_user_AT_FS_print_directory(At_Param_t param) {
  // 没有参数，则是打印FS中的所有文件
  // 有参数，则是打印FS中，该参数目录下的文件
  if (param->argc > 1) return AT_ERROR;
  if (param->argc == 0) {
    if (fs_tools_print_directory("")) return AT_ERROR;
  } else if (param->argc == 1) {
    if (fs_tools_print_directory(param->argv[0])) return AT_ERROR;
  } else return AT_ERROR;
  return AT_EOK;
}

static At_Err_t _at_user_AT_FS_print_content(At_Param_t param) {
  if (param->argc == 0) return AT_ERROR;
  if (param->argv[0] == "") return AT_ERROR;

  String content = "";
  int ret = fs_tools_readFile(param->argv[0], content);
  if (ret) {
    Serial.println(String("The file(") + param->argv[0] + ") has something error!");
    return AT_ERROR;
  }
  if (content != "") {
    Serial.println(String("The file(") + param->argv[0] + ") content: ");
    Serial.println(content);
  } else {
    Serial.println(String("The file(") + param->argv[0] + ") is empty!");
  }

  return AT_EOK;
}

static At_FS_State atFSTable[] = {
  { "info", AT_TYPE_CMD, _at_user_AT_FS_info },
  { "prdir", AT_TYPE_CMD, _at_user_AT_FS_print_directory },
  { "prcon", AT_TYPE_CMD, _at_user_AT_FS_print_content },
  { AT_LABLE_TAIL, AT_TYPE_NULL, at_user_AT_NULL },
};
static At _at_fs(atFSTable, Serial, Serial);
At_Err_t at_user_AT_FS(At_Param_t param) {
  At_Err_t err;
  String str = "";
  
  if (param->argc == 0) {
    goto err_out;
  }

  for (size_t i = 0; i < param->argc; i++) {
    str += String(param->argv[i]) + " ";
  }
  err = _at_fs.handle(str);
  if (err != AT_EOK) goto err_out;
  _at_fs.sendInfor(AT_USER_OK);
  return AT_EOK;

  if (0) {
err_out:
    _at_fs.printSet(param->cmd);
    at.sendInfor(String("commond(") + param->cmd + ")'s param is error");
    return AT_ERROR;
  }
}

At_Err_t at_user_AT_WiFi_Connect(At_Param_t param) {
  String infor = "";
  if (param->argc != 2) {
    infor += "AT+WIFI\r\n";
    infor += "\tSSID PSK\r\n";
    at.sendInfor(infor);
    return AT_ERROR;
  }
  WiFi.disconnect();
  int ret = ESP8266_WiFi_STA_Init(param->argv[0], param->argv[1]);
  if (ret) {
    infor += AT_USER_ERROR_FUNCTION;
    at.sendInfor(infor);
    return AT_ERROR;
  }
  infor += AT_USER_OK;
  at.sendInfor(infor);

  return AT_EOK;
}

At_Err_t at_user_AT_WiFi_Get_IP(At_Param_t param) {
  String infor = "";
  infor = WiFi.localIP().toString();
  at.sendInfor(infor);

  return AT_EOK;
}

#include <Pinger.h>
extern "C" {
#include <lwip/icmp.h>
}
static Pinger pinger;
static bool pinger_set_ok = false;
At_Err_t at_user_AT_Ping(At_Param_t param) {
  if (!WiFi.isConnected()) {
    Serial.println("Wi-Fi is not connect!");
    return AT_ERROR;
  }

  if (!pinger_set_ok) {
    pinger.OnReceive([](const PingerResponse &response) {
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

    pinger.OnEnd([](const PingerResponse &response) {
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

  if (param->argc == 0) {
    if (pinger.Ping(WiFi.gatewayIP()) == false) {
      Serial.println("Error during last ping command.");
    }
  } else if (param->argc == 1) {
    if (pinger.Ping(param->argv[0]) == false) {
      Serial.println("Error during last ping command.");
    }
  } else {
    Serial.println("arg number error!");
    return AT_ERROR;
  }

  return AT_EOK;
}
