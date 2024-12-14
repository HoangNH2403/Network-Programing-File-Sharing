#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "validate.h"

int parse_string(std::string& input, int& mode, std::vector<std::string>& users,
                 std::vector<std::string>& files) {
    std::istringstream iss(input);
    std::string token;
    bool parsing_users = false;
    bool parsing_files = false;
    bool have_users = false;
    bool have_files = false;

    if (!(iss >> mode)) {
        return -1;
    }

    while (iss >> token) {
        if (token == "-u") {
            have_users = parsing_users = true;
            parsing_files = false;
            continue;
        } else if (token == "-f") {
            have_files = parsing_files = true;
            parsing_users = false;
            continue;
        }

        if (parsing_users) {
            users.push_back(token);
        } else if (parsing_files) {
            files.push_back(token);
        }
    }
    if (!have_users || !have_files)
        return -1;
    return 0;
}

int server_share(int sockfd, char* arg, char* user_dir) {
    std::string args(arg);
    std::string current_dir(user_dir);
    int mode;
    std::vector<std::string> users, files;
    if (parse_string(args, mode, users, files) < 0) {
        std::string error_msg = "Parser share command error!";
        server_log('e', error_msg.c_str());
        send_message(sockfd, create_status_message(MSG_TYPE_ERROR, STATUS_SHARECMD_ERROR));
        return -1;
    } else {
        std::string success_msg = "Parser share command success!";
        server_log('i', success_msg.c_str());
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    }
    bool is_file_valid = true;
    for (const auto& file : files) {
        std::string path = current_dir + "/" + file;
        if (validate_file_or_dir(path.c_str()) < 0) {
            is_file_valid = false;
            break;
        }
    }
    if (!is_file_valid) {
        char* error_msg = (char*) malloc(sizeof(char) * SIZE);
        strcpy(error_msg, "Some files or directories are not valid, please check again!");
        server_log('e', error_msg);
        send_message(sockfd, create_message(MSG_TYPE_ERROR, error_msg));
        return -1;
    } else {
        send_message(sockfd, create_status_message(MSG_TYPE_OK, NO));
    }

    std::string root_path(root_dir);
    for (const auto& file : files) {
        std::string path = current_dir + "/" + file;
        for (const auto& user : users) {
            std::string user_dir = root_path + "/" + APP_STORAGE + user;
            std::string share_path = user_dir + "/share/.share";
            FILE* fp = fopen(share_path.c_str(), "a");
            if (fp == NULL) {
                continue;
            }
            fprintf(fp, "%s %d\n", path.c_str(), mode);
            std::string str_log_message = "Write " + path + " to " + share_path;
            server_log('i', str_log_message.c_str());
            fclose(fp);
        }
    }
    return 0;
}
