#include <dirent.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "color.h"
#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"

int handle_list(int sock_data) {
    Message msg;
    recv_message(sock_data, &msg);
    if (msg.type == MSG_DATA_LS) {
        printf("%s", msg.payload);
    }
    return 0;
}

// Custom comparison function
int compare(const struct dirent **a, const struct dirent **b) {
    struct stat sa, sb;
    stat((*a)->d_name, &sa);
    stat((*b)->d_name, &sb);

    // Directory entries first
    if (S_ISDIR(sa.st_mode) && !S_ISDIR(sb.st_mode))
        return -1;
    if (!S_ISDIR(sa.st_mode) && S_ISDIR(sb.st_mode))
        return 1;

    // Then sort alphabetically
    return alphasort(a, b);
}

int server_list(int sockfd) {
    struct dirent **output = NULL;
    Message msg;
    char curr_dir[SIZE], payload[SIZE];
    memset(curr_dir, 0, SIZE);
    memset(payload, 0, SIZE);

    getcwd(curr_dir, sizeof(curr_dir));
    std::string str_curr_dir(curr_dir);
    std::string str_log_message = "List directory " + str_curr_dir;
    server_log('i', str_log_message.c_str());

    int n = scandir(curr_dir, &output, NULL, compare);
    for (int i = 0; i < n; i++) {
        if (strcmp(output[i]->d_name, ".") != 0 && strcmp(output[i]->d_name, "..") != 0 &&
            strcmp(output[i]->d_name, ".share") != 0) {
            struct stat s;
            if (stat(output[i]->d_name, &s) == 0) {
                if (s.st_mode & S_IFDIR) {
                    if (strcmp(output[i]->d_name, "share") == 0) {
                        strcat(payload, BOLD);
                        strcat(payload, COLOR_MAGNETA);
                        strcat(payload, output[i]->d_name);
                        strcat(payload, RESET);
                    } else {
                        strcat(payload, BOLD);
                        strcat(payload, COLOR_CYAN);
                        strcat(payload, output[i]->d_name);
                        strcat(payload, RESET);
                    }
                } else if (s.st_mode & S_IFREG) {
                    strcat(payload, output[i]->d_name);
                }
            }
            strcat(payload, "  ");
        }
    }
    strcat(payload, "\n");
    msg = create_message(MSG_DATA_LS, payload);
    if (send_message(sockfd, msg) < 0) {
        perror("error");
    }

    return 0;
}