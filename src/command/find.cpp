#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "utils.h"

int server_find(int sockfd, char *arg) {
    FILE *fp;
    char buffer[256];
    char *output = (char *) malloc(SIZE);
    if (output == NULL) {
        perror("Malloc failed");
        return -1;
    }
    output[0] = '\0';
    char full_cmd[SIZE];
    snprintf(full_cmd, sizeof(full_cmd), "fd %s", arg);
    std::string cmd(full_cmd);
    std::string left_cmd, right_cmd;
    server_log('i', cmd.c_str());

    size_t pos = cmd.find('|');
    if (pos != std::string::npos) {
        // '|' found, split the string
        left_cmd = cmd.substr(0, pos);
        left_cmd += " --type f";
        right_cmd = cmd.substr(pos + 1);

        left_cmd.erase(left_cmd.find_last_not_of(" \n\r\t") + 1);
        right_cmd.erase(0, right_cmd.find_first_not_of(" \n\r\t"));
        std::string str_log_message = "Pipe command: " + left_cmd + " | " + right_cmd;
        server_log('i', str_log_message.c_str());
    } else {
        left_cmd = cmd;
    }

    fp = popen(left_cmd.c_str(), "r");
    if (fp == NULL) {
        perror("Failed to run command");
        free(output);
        return -1;
    }

    size_t currentSize = SIZE;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Check if we need to expand our output buffer
        if (strlen(output) + strlen(buffer) + 1 > currentSize) {
            currentSize *= 2;
            output = (char *) realloc(output, currentSize);
            if (output == NULL) {
                perror("Realloc failed");
                return -1;
            }
        }
        strcat(output, buffer);
    }
    pclose(fp);
    int n = (int) strlen(output);
    int bytes_sent = 0;
    Message msg;

    while (bytes_sent < n) {
        int chunk_size = std::min(PAYLOAD_SIZE, n - bytes_sent);

        strncpy(msg.payload, output + bytes_sent, chunk_size);
        msg.length = chunk_size;
        msg.type = MSG_DATA_FIND;
        if (send_message(sockfd, msg) < 0) {
            break;
        }
        memset(msg.payload, 0, PAYLOAD_SIZE);
        bytes_sent += chunk_size;
    }

    send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));

    if (!right_cmd.empty()) {
        send_message(sockfd, create_status_message(MSG_TYPE_PIPE, NO));
        server_pipe_download(sockfd, output);
    } else {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    }
    return 0;
}
