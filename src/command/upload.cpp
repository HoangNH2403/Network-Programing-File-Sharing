#include <libgen.h>
#include <sys/socket.h>
#include <utils.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "color.h"
#include "command.h"
#include "common.h"
#include "connect.h"
#include "log.h"
#include "message.h"
#include "validate.h"

char *handle_path(char *dir) {
    char *home = getenv("HOME");
    char *path = (char *) malloc(SIZE);
    strcpy(path, home);
    strcat(path, "/");
    if (dir[0] == '~' || dir[0] == '.') {
        dir += 2;
    }
    strcat(path, dir);
    strcpy(dir, path);
    free(path);
    return dir;
}

void handle_upload(int sock_data, char *dir, int sock_control) {
    dir = handle_path(dir);
    char compress_folder[SIZE];
    int fl = is_folder(dir);
    std::string str_log_message = fl ? "Uploading folder " : "Uploading file ";

    Message msg;
    recv_message(sock_control, &msg);
    if (msg.type == MSG_TYPE_ERROR) {
        printf(COLOR_YELLOW "%s" RESET "\n", msg.payload);
        std::string str_payload(msg.payload);
        log_message('e', str_payload.c_str());
        return;
    }

    if (is_folder(dir)) {
        strcpy(compress_folder, dir);
        char *last_past = basename(compress_folder);
        strcat(compress_folder, last_past);
        strcat(compress_folder, ".zip");
        zip(dir, compress_folder);
        strcpy(dir, compress_folder);

        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FOLDER, NO));
    } else
        send_message(sock_control, create_status_message(MSG_TYPE_DOWNLOAD_FILE, NO));

    std::string str_dir(dir);
    str_log_message += str_dir;
    FILE *fp = fopen(dir, "r");
    if (!fp) {
        send_message(sock_control, create_status_message(MSG_TYPE_ERROR, FILE_NOT_FOUND));
        perror(COLOR_RED "error opening file" RESET);
        str_log_message = str_dir + " not found";
        log_message('e', str_log_message.c_str());
    } else {
        send_message(sock_control, create_status_message(MSG_TYPE_OK, NO));
        printf("Uploading file %s to server!\n", dir);
        str_log_message = "Uploading file " + str_dir + " to server!";
        log_message('i', str_log_message.c_str());
        size_t byte_read;

        Message data;
        bool upload_success = true;
        do {
            byte_read = fread(data.payload, 1, PAYLOAD_SIZE, fp);
            data.type = MSG_TYPE_DOWNLOAD;
            data.length = byte_read;
            if (send_message(sock_data, data) < 0) {
                log_message('e', "ERROR upload file to server, payload too large!");
                printf(COLOR_RED "ERROR upload file to server, try again later!" RESET
                                      "\n");
                send_message(sock_data,
                create_status_message(MSG_TYPE_ERROR, STATUS_MESSAGE_TOO_LARGE));
                upload_success = false;
                break;
            }
            memset(data.payload, 0, SIZE);
        } while (byte_read > 0);

        send_message(sock_data, create_status_message(MSG_TYPE_OK, NO));
        str_log_message =
            upload_success ? "File " + str_dir + " uploaded" : "File " + str_dir + " not uploaded";
        log_message('i', str_log_message.c_str());
        fclose(fp);
        if (fl) {
            remove(compress_folder);
        }
    }
}

int _upload(int sock_control, int data_sock, char *arg, char *cur_dir) {
    Message msg;
    recv_message(sock_control, &msg);
    int is_file = msg.type == MSG_TYPE_DOWNLOAD_FILE ? 1 : 0;

    recv_message(sock_control, &msg);
    if (msg.type == MSG_TYPE_ERROR) {
        printf(COLOR_RED "%s" RESET "\n", msg.payload);
        return -1;
    }

    if (!is_file)
        strcat(arg, ".zip");

    char *last_past = basename(arg);
    char *path = (char *) malloc(SIZE);

    strcpy(path, cur_dir);
    strcat(path, "/");
    strcat(path, last_past);

    std::string str_path(path);
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror(COLOR_RED "ERROR opening file" RESET);
        std::string str_log_message = "ERROR opening file " + str_path;
        server_log('e', str_log_message.c_str());
        return -1;
    }

    bool upload_success = true;
    while (1) {
        recv_message(data_sock, &msg);
        if (msg.type == MSG_TYPE_DOWNLOAD) {
            fwrite(msg.payload, 1, msg.length, fp);
        } else if (msg.type == MSG_TYPE_ERROR) {
            server_log('e', msg.payload);
            upload_success = false;
            break;
        } else if (msg.type == MSG_TYPE_OK) {
            break;
        }
    }
    char *log_msg = (char *) malloc(SIZE);
    if (!is_file && upload_success) {
        sprintf(log_msg, "Folder uploaded to %s", path);
    } else if (upload_success) {
        sprintf(log_msg, "File uploaded to %s", path);
    } else {
        sprintf(log_msg, "File can not uploaded to %s", path);
    }
    server_log('i', log_msg);
    free(log_msg);
    fclose(fp);
    if (!upload_success) {
        remove(path);
    }
    if (!is_file) {
        char *extracted_path = (char *) malloc(SIZE);
        strncpy(extracted_path, path, strlen(path) - 4);
        extracted_path[strlen(path) - 4] = '\0';
        unzip(path, extracted_path);
        remove(path);
    }
    return 0;
}

int server_upload(int sock_control, int data_sock, char *arg, char *cur_dir) {
    char *share_folder_path = (char *) malloc(sizeof(char) * SIZE);
    int share_mode = is_current_share_folder(cur_dir, share_folder_path);
    if (share_mode == -1) {
        char error_msg[] = "You should not upload file to share folder";
        std::string str_error_msg = "In share folder, should not upload file";
        server_log('w', str_error_msg.c_str());
        send_message(sock_control, create_message(MSG_TYPE_ERROR, error_msg));
        return -1;
    } else if (share_mode == 0 || share_mode == 1) {
        char error_msg[] = "Error: You don't have permission to access this folder!";
        std::string str_error_msg =
            "In folder that don't have permission to access, can't upload file";
        server_log('e', str_error_msg.c_str());
        send_message(sock_control, create_message(MSG_TYPE_ERROR, error_msg));
        return -1;
    } else if (share_mode == 3) {
        send_message(sock_control, create_status_message(MSG_TYPE_OK, NO));
        _upload(sock_control, data_sock, arg, cur_dir);
    } else {
        send_message(sock_control, create_status_message(MSG_TYPE_OK, NO));
        _upload(sock_control, data_sock, arg, share_folder_path);
    }
    free(share_folder_path);
    return 0;
}
