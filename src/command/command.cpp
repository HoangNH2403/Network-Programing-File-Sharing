#include "command.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <sstream>
#include <vector>

#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "utils.h"
#include "validate.h"

#define INITIAL_SIZE 1024

/**
 * wrapper output of command to a string
 */
char *get_cmd_output(char *cmd) {
    FILE *fp;
    char buffer[256];
    char *output = (char *) malloc(INITIAL_SIZE);
    if (output == NULL) {
        perror("Malloc failed");
        return NULL;
    }
    output[0] = '\0';
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    fp = popen(full_cmd, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        free(output);
        return NULL;
    }

    size_t currentSize = INITIAL_SIZE;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Check if we need to expand our output buffer
        if (strlen(output) + strlen(buffer) + 1 > currentSize) {
            currentSize *= 2;
            output = (char *) realloc(output, currentSize);
            if (output == NULL) {
                perror("Realloc failed");
                return NULL;
            }
        }
        strcat(output, buffer);
    }

    pclose(fp);
    return output;
}

int _process_command(int sockfd, char *base_cmd) {
    char cmd[256];
    char *cmd_output = get_cmd_output(base_cmd);

    snprintf(cmd, sizeof(cmd), "%s > /dev/null 2>&1", base_cmd);
    server_log('i', cmd);
    int res = system(cmd);
    char *msg_log = (char *) malloc(SIZE);
    if (res == -1) {
        sprintf(msg_log, "Error: %s", base_cmd);
        server_log('e', msg_log);
        char *errorString = strerror(errno);
        send_message(sockfd, create_message(MSG_TYPE_ERROR, errorString));
        return -1;
    }

    int n = (int) strlen(cmd_output);
    int bytes_sent = 0;
    Message msg;
    bool download_success = true;

    while (bytes_sent < n) {
        int chunk_size = std::min(PAYLOAD_SIZE, n - bytes_sent);

        strncpy(msg.payload, cmd_output + bytes_sent, chunk_size);
        msg.length = chunk_size;
        msg.type = MSG_DATA_CMD;
        if (send_message(sockfd, msg) < 0) {
            download_success = false;
            break;
        }
        memset(msg.payload, 0, PAYLOAD_SIZE);
        bytes_sent += chunk_size;
    }
    if (download_success) {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    }
    sprintf(msg_log, "Success: %s", base_cmd);
    server_log('i', msg_log);
    free(msg_log);
    return 0;
}

int process_command(int sockfd, char *base_cmd, char *cur_dir) {
    // if (strncmp(base_cmd, "cat", 3) == 0) {
    //     send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    //     _process_command(sockfd, base_cmd);
    //     return 0;
    // }
    char *share_folder_path = (char *) malloc(SIZE);
    int share_mode = is_current_share_folder(cur_dir, share_folder_path);
    if (share_mode == -1) {
        char err_msg[] = "You shouldn't modify files in the share folder.";
        server_log('w', err_msg);
        send_message(sockfd, create_message(MSG_TYPE_ERROR, err_msg));
        return -1;
    } else if (share_mode == 0) {
        char err_msg[] = "You do not have permission to modify this folder.";
        server_log('e', err_msg);
        send_message(sockfd, create_message(MSG_TYPE_ERROR, err_msg));
        return -1;
    } else if (share_mode == 1) {
        // send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
        // _process_command(sockfd, base_cmd);
        if (strncmp(base_cmd, "cat", 3) == 0) {
            send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
            _process_command(sockfd, base_cmd);
        } else {
            char err_msg[] = "You do not have permission to modify this folder.";
            server_log('e', err_msg);
            send_message(sockfd, create_message(MSG_TYPE_ERROR, err_msg));
            return -1;
        }
    } else if (share_mode == 2) {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
        _process_command(sockfd, base_cmd);
    } else {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
        _process_command(sockfd, base_cmd);
        std::string user_path(cur_dir);
        std::string share_path(share_folder_path);
        std::string rm_cmd = "rm -rf " + share_path + "/*";
        system(rm_cmd.c_str());
        server_log('i', rm_cmd.c_str());
        std::string cp_cmd = "cp -r " + user_path + "/* " + share_path;
        system(cp_cmd.c_str());
        server_log('i', cp_cmd.c_str());
    }
    return 0;
}

int load_shared_file(char *user_dir) {
    std::string user_path(user_dir);
    std::string share_path = user_path + "/share/.share";
    system(("rm -rf " + user_path + "/share/*").c_str());
    std::ifstream file(share_path);

    if (!file.is_open()) {
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string path;
        int number;

        if (!(iss >> path >> number)) {
            continue;
        }

        int fl = validate_file_or_dir(path.c_str());
        if (fl < 0) {
            continue;
        } else if (fl == 0) {
            system(("cp -r " + path + " " + user_path + "/share").c_str());
        } else if (fl == 1) {
            system(("cp " + path + " " + user_path + "/share").c_str());
        }
    }

    file.close();
    return 0;
}

int check_permision(const char *c_dir, const char *share_path, char *share_folder_path) {
    std::ifstream file(share_path);
    if (!file.is_open()) {
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string path;
        int number;

        if (!(iss >> path >> number)) {
            continue;
        }

        char *c_base = basename((char *) path.c_str());
        std::string base(c_base);
        std::string dir(c_dir);
        if (base == dir) {
            strcpy(share_folder_path, path.c_str());
            return number;
        }
    }

    file.close();
    return 0;
}

int is_current_share_folder(char *dir, char *share_folder_path) {
    std::string cur_dir(dir);
    std::vector<std::string> tokens = split(cur_dir, '/');
    int index = -1;
    for (int i = 0; i < (int) tokens.size(); i++) {
        if (tokens[i] == "share") {
            index = i;
            break;
        }
    }
    if (index == (int) tokens.size() - 1)
        return -1;
    if (index == -1) {
        return 3;
    } else {
        std::string share_folder = "/";
        for (int i = 0; i <= index; i++) {
            share_folder += tokens[i];
            share_folder += "/";
        }
        share_folder += ".share";
        std::string folder = tokens[index + 1];
        strcpy(share_folder_path, share_folder.c_str());
        return check_permision(folder.c_str(), share_folder.c_str(), share_folder_path);
    }
}

// int is_current_share_folder(char *dir, char *share_folder_path) {
//     std::string cur_dir(dir);
//     std::vector<std::string> tokens = split(cur_dir, '/');
//     int index = -1;
//     for (int i = 0; i < (int) tokens.size(); i++) {
//         if (tokens[i] == "share") {
//             index = i;
//             break;
//         }
//     }
//     if (index == (int) tokens.size() - 1)
//         return -1;
//     if (index == -1) {
//         return 2;
//     } else {
//         std::string share_folder = "/";
//         for (int i = 0; i <= index; i++) {
//             share_folder += tokens[i];
//             share_folder += "/";
//         }
//         share_folder += ".share";
//         std::string folder = tokens[index + 1];
//         strcpy(share_folder_path, share_folder.c_str());
//         return check_permision(folder.c_str(), share_folder.c_str(), share_folder_path);
//     }
// }

// int download_from_share_folder(char *dir, char *share_folder_path) {
//     std::string cur_dir(dir);
//     std::vector<std::string> tokens = split(cur_dir, '/');
//     int index = -1;
//     for (int i = 0; i < (int) tokens.size(); i++) {
//         if (tokens[i] == "share") {
//             index = i;
//             break;
//         }
//     }
//     if (index == (int) tokens.size() - 1) {
//         std::string share_folder = "/";
//         for (int i = 0; i <= index; i++) {
//             share_folder += tokens[i];
//             share_folder += "/";
//         }
//         share_folder += ".share";
//         std::string folder = d;
//         strcpy(share_folder_path, share_folder.c_str());
//         return check_permision(dir.c_str(), share_folder.c_str(), share_folder_path);
//     }
//     return -1;
// }