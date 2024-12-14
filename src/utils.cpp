#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"
#include "log.h"

int is_client(const char *process) { return strcmp(process, "./client") == 0; }

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    while (getline(tokenStream, token, delim)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}

void zip(const char *folder, const char *compress_path) {
    char command[1000];
    if (compress_path[0] == '/') {
        sprintf(command, "cd %s && zip -r -y %s . > /dev/null 2>&1", folder, compress_path);
    } else {
        sprintf(command, "cd %s && zip -r -y ../%s . > /dev/null 2>&1", folder, compress_path);
    }
    system(command);
}

void unzip(const char *compressed_path, const char *extract_path) {
    char command[1000];
    sprintf(command, "unzip -o %s -d %s > /dev/null 2>&1", compressed_path, extract_path);
    system(command);
}

void print_centered(const char *text) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int len = strlen(text);
    int spaces = (w.ws_col - len) / 2;

    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    printf(BOLD COLOR_BLUE"%s" RESET "\n", text);
}

void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

void print_menu(int current_selection, const char *options[], int num_options) {
    for (int i = 0; i < num_options; i++) {
        if (i == current_selection) {
            printf(BOLD COLOR_YELLOW "> %s" RESET "\n", options[i]);

        } else {
            printf("  %s\n", options[i]);
        }
    }
}

int process_menu(const char *menu_items[], int num_items) {
    int current_selection = 0;

    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);

    enable_raw_mode(&orig_termios);
    print_centered("MINI DRIVE\n");
    print_menu(current_selection, menu_items, num_items);

    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == '\033') {
            char seq[3];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[') {
                if (seq[1] == 'A' && current_selection > 0) {   // Up arrow
                    current_selection--;
                } else if (seq[1] == 'B' && current_selection < num_items - 1) {   // Down arrow
                    current_selection++;
                }
            }

            printf("\033[H\033[J");
            print_centered("MINI DRIVE\n");
            print_menu(current_selection, menu_items, num_items);
        } else if (c == '\n') {
            break;
        }
    }

    disable_raw_mode(&orig_termios);
    return current_selection;
}

char *handle_prompt(char *cur_user, char *user_dir) {
    const std::string app_name = "minidrive";
    std::vector<std::string> tokens = split(std::string(user_dir), '/');
    std::string cur_user_str(cur_user);

    // Find the index of cur_user in the tokens
    size_t user_index = std::string::npos;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == cur_user_str) {
            user_index = i;
            break;
        }
    }

    char *prompt = (char *) malloc(sizeof(char) * SIZE);
    if (user_index == std::string::npos) {
        sprintf(prompt, "[%s@%s ~/]$ ", cur_user, app_name.c_str());
        return prompt;
    }

    // Construct the path from the user's directory onwards
    std::string path = "~/";
    for (size_t i = user_index + 1; i < tokens.size(); ++i) {
        path += tokens[i] + "/";
    }

    sprintf(prompt, "[%s@%s %s]$ ", cur_user, app_name.c_str(), path.c_str());
    return prompt;
}

void trimstr(char *str, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (isspace(str[i]))
            str[i] = 0;
        if (str[i] == '\n')
            str[i] = 0;
    }
}

char *get_username(char *path) {
    char *lastSlash = strrchr(path, '/');

    if (lastSlash != NULL) {
        // Return the substring after the last '/'
        return lastSlash + 1;
    }

    // Return the original path if no '/'
    return path;
}

void read_input(char *user_input, int size) {
    memset(user_input, 0, size);
    int n = read(STDIN_FILENO, user_input, size);
    user_input[n] = '\0';

    /* Remove trailing return and newline characters */
    if (user_input[n - 1] == '\n')
        user_input[n - 1] = '\0';
    if (user_input[n - 1] == '\r')
        user_input[n - 1] = '\0';
}

int create_dir(const char *path) {
    int status = 0;
    struct stat st;

    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 0;
    }
    status = mkdir(path, 0755);

    if (status == 0) {
        return 0;
    } else {
        perror("Error");
        return -1;
    }
}

int create_file(const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return 0;
    }
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        perror("Error creating file");
        return -1;
    }
    fclose(file);
    return 0;
}

void toggle_lock(const char *username, int lock_st) {
    FILE *fp = fopen(ACCOUNTS_FILE, "r");

    if (fp == NULL) {
        perror("Error account file: ");
        return;
    }

    char currentUsername[100];
    char password[100];
    int isLocked;

    FILE *temp = fopen("temp.txt", "w");

    if (temp == NULL) {
        printf("Error creating temporary file!\n");
        fclose(fp);
        return;
    }

    while (fscanf(fp, "%s %s %d", currentUsername, password, &isLocked) == 3) {
        if (strcmp(currentUsername, username) == 0) {
            fprintf(temp, "%s %s %d\n", currentUsername, password, lock_st);
        } else {
            fprintf(temp, "%s %s %d\n", currentUsername, password, isLocked);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(ACCOUNTS_FILE);
    rename("temp.txt", ACCOUNTS_FILE);

    if (lock_st) {
        printf("User \"%s\" logged in!\n", username);
    } else {
        printf("User \"%s\" logged out!\n", username);
    }

    std::string str_log_message =
        "User " + std::string(username) + " has been " + (lock_st == 1 ? "locked" : "unlocked");
    server_log('i', str_log_message.c_str());
}
