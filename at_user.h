#ifndef __AT_USER_H__
#define __AT_USER_H__

#include <Arduino.h>

#include "at.h"

#define AT_USER_OK "OK"
#define AT_USER_ERROR_ARGC "error1"
#define AT_USER_ERROR_TIMEOUT "error2"
#define AT_USER_ERROR_NOTFIND "error3"
#define AT_USER_ERROR_FUNCTION "error4"

extern At_Class at_user;

#endif /* __AT_USER_H__ */
