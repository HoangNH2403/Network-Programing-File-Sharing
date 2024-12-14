#include "authenticate.h"

#include <openssl/evp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "connect.h"
#include "message.h"
#include "utils.h"

// void sha256(const char *input, char *output) {
//     EVP_MD_CTX *mdctx;
//     const EVP_MD *md;
//     unsigned char md_value[EVP_MAX_MD_SIZE];
//     unsigned int md_len;

//     md = EVP_get_digestbyname("sha256");

//     if (!md) {
//         printf("Unknown message digest %s\n", "sha256");
//         exit(1);
//     }

//     mdctx = EVP_MD_CTX_new();
//     EVP_DigestInit_ex(mdctx, md, NULL);
//     EVP_DigestUpdate(mdctx, input, strlen(input));
//     EVP_DigestFinal_ex(mdctx, md_value, &md_len);
//     EVP_MD_CTX_free(mdctx);

//     // Encode hash in hexadecimal
//     for (unsigned int i = 0; i < md_len; i++) {
//         sprintf(output + 2 * i, "%02x", md_value[i]);
//     }
//     output[2 * md_len] = '\0';
// }

char *get_acc_from_cli() {
    char *payload = (char *) malloc(PAYLOAD_SIZE);

    char user[SIZE];
    memset(user, 0, SIZE);

    printf("Enter username: ");
    fflush(stdout);
    read_input(user, SIZE);
    strcat(payload, user);

    fflush(stdout);
    char *pass = getpass("Enter password: ");
    strcat(payload, " ");
    strcat(payload, pass);

    return payload;
}

int handle_login(int sockfd, char *cur_user) {
    Message msg;
    char *payload = (char *) malloc(PAYLOAD_SIZE);
    memset(payload, 0, PAYLOAD_SIZE);
    payload = get_acc_from_cli();

    msg = create_message(MSG_TYPE_AUTHEN, payload);

    send_message(sockfd, msg);

    Message response;
    recv_message(sockfd, &response);
    switch (response.type) {
        case MSG_TYPE_ERROR:
            printf("%s\n", response.payload);
            return 0;
        case MSG_TYPE_OK:
            printf("%s\n", response.payload);
            strcpy(cur_user, strtok(payload, " "));
            return 1;
        default:
            perror("error reading message from server");
            return 0;
    }
    return 1;
}

int register_acc(int sockfd) {
    Message msg;
    char *payload = (char *) malloc(PAYLOAD_SIZE);
    memset(payload, 0, PAYLOAD_SIZE);
    payload = get_acc_from_cli();
    msg = create_message(MSG_TYPE_REGISTER, payload);

    send_message(sockfd, msg);

    Message response;
    recv_message(sockfd, &response);

    switch (response.type) {
        case MSG_TYPE_ERROR:
            printf("%s\n", response.payload);
            return 0;
        case MSG_TYPE_OK:
            printf("%s\n", response.payload);
            return 1;
        default:
            perror("error reading message from server");
            return 0;
            break;
    }
}

int check_user_acc(char *user, char *pass, char *user_dir) {
    char username[SIZE];
    char password[SIZE];
    char cur_dir[SIZE];
    int is_locked;
    char *pch;
    char buf[SIZE];
    char *line = NULL;
    size_t num_read;
    size_t len = 0;
    FILE *fd;
    int auth = 0;

    fd = fopen(ACCOUNTS_FILE, "r");
    if (fd == NULL) {
        perror("Error: ");
        exit(1);
    }

    while ((num_read = getline(&line, &len, fd)) != (size_t) -1) {
        memset(buf, 0, SIZE);
        strcpy(buf, line);

        pch = strtok(buf, " ");
        strcpy(username, pch);

        if (pch != NULL) {
            pch = strtok(NULL, " ");
            strcpy(password, pch);
            pch = strtok(NULL, " ");
            trimstr(pch, (int) strlen(pch));
            is_locked = atoi(pch);
        }

        // remove end of line and whitespace
        trimstr(password, (int) strlen(password));

        //char outputBuffer[65];
        //sha256(pass, outputBuffer);

        // printf("%d %d %d\n", strcmp(user, username), strcmp(outputBuffer, password), is_locked);
        if ((strcmp(user, username) == 0) &&
            (strcmp(pass, password) == 0 && (is_locked == 0))) {
            auth = 1;
            // Lock user to prevent concurrent login
            toggle_lock(user, 1);

            // Change dir to user root dir
            strcat(user_dir, username);
            chdir(user_dir);

            // Save user root dir to a global variable for future use
            getcwd(cur_dir, sizeof(cur_dir));
            strcpy(user_dir, cur_dir);
            break;
        }
    }
    free(line);
    fclose(fd);
    return auth;
}

int check_username(char *user) {
    char username[SIZE];
    char *pch;
    char buf[SIZE];
    char *line = NULL;
    size_t num_read;
    size_t len = 0;
    FILE *fd;
    int check = 0;

    fd = fopen(ACCOUNTS_FILE, "r");
    if (fd == NULL) {
        perror("Error: ");
        exit(1);
    }

    while ((num_read = getline(&line, &len, fd)) != (size_t) -1) {
        memset(buf, 0, SIZE);
        strcpy(buf, line);

        pch = strtok(buf, " ");
        strcpy(username, pch);

        if (strcmp(user, username) == 0) {
            check = 1;
            break;
        }
    }
    free(line);
    fclose(fd);
    return check;
}

int server_login(Message msg, char *user_dir) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    strcpy(buf, msg.payload);
    strcpy(user, strtok(buf, " "));
    strcpy(pass, strtok(NULL, " "));

    return (check_user_acc(user, pass, user_dir));
}

int server_register(Message msg) {
    char buf[SIZE];
    char user[SIZE];
    char pass[SIZE];
    char user_storage[SIZE] = APP_STORAGE;
    memset(user, 0, SIZE);
    memset(pass, 0, SIZE);
    memset(buf, 0, SIZE);

    strcpy(buf, msg.payload);
    strcpy(user, strtok(buf, " "));
    strcpy(pass, strtok(NULL, " "));

    if (check_username(user)) {
        return 0;
    }

    FILE *fp;

    fp = fopen(ACCOUNTS_FILE, "a");
    if (fp == NULL) {
        perror("Error: ");
        exit(1);
    }
    fprintf(fp, "%s ", user);
    //char hash_pass[65];
    //sha256(pass, hash_pass);
    fprintf(fp, "%s 0\n", pass);

    // Create storage directory for new user
    strcat(user_storage, user);
    create_dir(user_storage);
    strcat(user_storage, "/share");
    mkdir(user_storage, 0777);
    strcat(user_storage, "/.share");
    FILE *share = fopen(user_storage, "w");
    fclose(share);
    fclose(fp);
    return 1;
}