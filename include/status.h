#ifndef __STATUS_H__
#define __STATUS_H__

#include <string>

typedef enum {
    NO = 0,
    LOGIN_SUCCESS = 201,
    LOGIN_FAIL = 430,
    USER_NOT_FOUND = 101,
    USER_IS_BLOCKED = 102,
    USERNAME_EXIST = 109,
    BLOCKED_USER = 103,
    PASSWORD_INVALID = 104,
    FILE_NOT_FOUND = 108,
    USER_IS_ONLINE = 105,
    DIRECTORY_NOT_FOUND = 110,
    ACCOUNT_IS_EXIST = 106,
    REGISTER_SUCCESS = 202,
    LOGOUT_SUCCESS = 203,
    USERNAME_OR_PASSWORD_INVALID = 107,
    COMMAND_INVALID = 301,
    STATUS_MV_ERROR = 302,
    STATUS_SHARECMD_ERROR = 303,
    STATUS_MESSAGE_TOO_LARGE = 304,
    SERVER_ERROR = 500
} Status;

/**
 * Convert status code to string
 * @param code status code
 * @return string of status code
 */
std::string status_str(Status code);

#endif   // !__STATUS_H__
