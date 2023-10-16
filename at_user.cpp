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

// 建立AT指令集下的LED子指令集
#define _AT_USER_LED_ON      LOW
#define _AT_USER_LED_OFF     HIGH
static At_Err_t _at_user_led_on(At_Param_t param);
static At_Err_t _at_user_led_off(At_Param_t param);
struct At_State atLEDTable[] = {
    { "on", AT_TYPE_CMD, _at_user_led_on },
    { "off", AT_TYPE_CMD, _at_user_led_off },
    { AT_LABLE_TAIL, AT_TYPE_NULL, at_user_AT_NULL },
};
static At _at_led(atLEDTable, Serial, Serial);

static At_Err_t _at_user_led_on(At_Param_t param)
{
    pinMode(2, OUTPUT);
    digitalWrite(2, _AT_USER_LED_ON);
    _at_led.sendInfor(AT_USER_OK);
    return AT_EOK;
}

static At_Err_t _at_user_led_off(At_Param_t param)
{
    pinMode(2, OUTPUT);
    digitalWrite(2, _AT_USER_LED_OFF);
    _at_led.sendInfor(AT_USER_OK);
    return AT_EOK;
}

At_Err_t at_user_AT_LED(At_Param_t param)
{
    At_Err_t err;
    String str = "";

    // if ((param->argc < 1) || (param->argc > 2)) {
    if (param->argc != 1) {
        goto err_out;
    }
    
    for (size_t i = 0; i < param->argc; i++) {
        str += String(param->argv[i]) + " ";
    }
    err = _at_led.handle(str);
    if (err != AT_EOK) goto err_out;
    at.sendInfor(AT_USER_OK);
    return err;

    if (0) {
    err_out:
        _at_led.printSet(param->cmd);
        at.sendInfor(String("commond(") + param->cmd + ")'s param is error");
        return AT_ERROR;
    }
}

#include "fs_tools.h"
static At_Err_t _at_user_AT_FS_info(At_Param_t param);
static At_Err_t _at_user_AT_FS_format(At_Param_t param);
static At_Err_t _at_user_AT_FS_print_directory(At_Param_t param);
static At_Err_t _at_user_AT_FS_print_content(At_Param_t param);
static struct At_State atFSTable[] = {
  { "info", AT_TYPE_CMD, _at_user_AT_FS_info },
  { "format", AT_TYPE_CMD, _at_user_AT_FS_format },
  { "prdir", AT_TYPE_CMD, _at_user_AT_FS_print_directory },
  { "prcon", AT_TYPE_CMD, _at_user_AT_FS_print_content },
  { AT_LABLE_TAIL, AT_TYPE_NULL, at_user_AT_NULL },
};
static At _at_fs(atFSTable, Serial, Serial);

static At_Err_t _at_user_AT_FS_info(At_Param_t param) {
  if (fs_tools_FS_info()) return AT_ERROR;
  return AT_EOK;
}

static At_Err_t _at_user_AT_FS_format(At_Param_t param)
{
    if (fs_tools_FS_format() != true) return AT_ERROR;
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
    _at_fs.println(String("The file(") + param->argv[0] + ") has something error!");
    return AT_ERROR;
  }
  if (content != "") {
    _at_fs.println(String("The file(") + param->argv[0] + ") content: ");
    _at_fs.println(content);
  } else {
    _at_fs.println(String("The file(") + param->argv[0] + ") is empty!");
  }

  return AT_EOK;
}

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
  at.sendInfor(AT_USER_OK);
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
    at.println("Wi-Fi is not connect!");
    return AT_ERROR;
  }

  if (!pinger_set_ok) {
    pinger.OnReceive([](const PingerResponse &response) {
      if (response.ReceivedResponse) {
        at.printf("Reply from %s: bytes=%d time=%lums TTL=%d\r\n",
                      response.DestIPAddress.toString().c_str(),
                      response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
                      response.ResponseTime,
                      response.TimeToLive);
      } else {
        at.println("Request timed out.");
      }
      return true;
    });

    pinger.OnEnd([](const PingerResponse &response) {
      float loss = 100;  // 丢失百分比
      if (response.TotalReceivedResponses > 0) {
        loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
      }
      at.printf("Ping statistics for %s:\r\n",
                    response.DestIPAddress.toString().c_str());
      at.printf("\tPackets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\r\n",
                    response.TotalSentRequests,
                    response.TotalReceivedResponses,
                    response.TotalSentRequests - response.TotalReceivedResponses,
                    loss);
      if (response.TotalReceivedResponses > 0) {
        at.println("Approximate round trip times in milli-seconds:");
        at.printf("\tMinimum = %lums, Maximum = %lums, Average = %.2fms\r\n",
                      response.MinResponseTime,
                      response.MaxResponseTime,
                      response.AvgResponseTime);
      }
      at.println("Destination host data:");
      at.printf("\tIP address: %s\r\n",
                    response.DestIPAddress.toString().c_str());
      if (response.DestMacAddress != nullptr) {
        at.printf("\tMAC address: " MACSTR "\r\n",
                      MAC2STR(response.DestMacAddress->addr));
      }
      if (response.DestHostname != "") {
        at.printf("\tDNS name: %s\r\n",
                      response.DestHostname.c_str());
      }
      return true;
    });

    pinger_set_ok = true;
  }

  if (param->argc == 0) {
    if (pinger.Ping(WiFi.gatewayIP()) == false) {
      at.println("Error during last ping command.");
    }
  } else if (param->argc == 1) {
    if (pinger.Ping(param->argv[0]) == false) {
      at.println("Error during last ping command.");
    }
  } else {
    at.println("arg number error!");
    return AT_ERROR;
  }

  return AT_EOK;
}
