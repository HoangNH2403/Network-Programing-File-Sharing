#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "status.h"

int is_subdir(const char *baseDir, const char *dir) {
    while (*baseDir) {
        if (*baseDir++ != *dir++)
            return 0;   // It is not a subdirectory
    }

    return (*dir == '\0' || *dir == '/') ? 1 : 0;   // Check if it is a subdirectory
}

int server_cd(int sockfd, char *dir, char *user_dir, char *cur_user_dir) {
    char cur_dir[SIZE];
    if (chdir(dir) == 0) {
        getcwd(cur_dir, sizeof(cur_dir));
        if (!is_subdir(user_dir, cur_dir)) {
            server_log('i', "No such file or directory!");
            send_message(sockfd, create_status_message(MSG_TYPE_ERROR, DIRECTORY_NOT_FOUND));
        } else {
            strcpy(cur_user_dir, cur_dir);
            std::string str_cur_dir(cur_dir);
            std::string str_log_message = "Change directory to " + str_cur_dir;
            server_log('i', str_log_message.c_str());
            send_message(sockfd, create_message(MSG_DATA_CD, cur_dir));
        }
    } else {
        send_message(sockfd, create_status_message(MSG_TYPE_ERROR, DIRECTORY_NOT_FOUND));
    }
    return 0;
}