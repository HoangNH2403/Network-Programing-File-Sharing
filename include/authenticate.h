#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

#include <cstdio>

#include "message.h"

//void sha256(const char *input, char *output);

/**
 * Login to minidrive
 * @param sockfd socket for control connection
 * @param cur_user current user logged in
 * @return 1 if authenticated, 0 if not
 */
int handle_login(int sockfd, char *cur_user);

/**
 * Register new account
 * @param sockfd socket for control connection
 * @return 1 if success, 0 if not
 */
int register_acc(int sockfd);

/**
 * Authenticate a user's credentials
 * @param user username
 * @param pass password
 * @param user_dir user directory
 * @return 1 if authenticated, 0 if not
 */
int check_user_acc(char *user, char *pass, char *user_dir);

/**
 * Check if db has existing username
 * @param user username
 * @return 1 if exist, 0 if not
 */
int check_username(char *user);

/**
 * server handle login
 * @param msg message to receive
 * @param user_dir user directory
 * @return 1 if authenticated, 0 if not
 */
int server_login(Message msg, char *user_dir);

/**
 * server handle register new account
 * @param msg message to receive
 * @return 1 if success, 0 if not
 */
int server_register(Message msg);

#endif
