#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <cstring>
#include <string>

std::string get_log_level(char level) {
    switch (level) {
        case 'i':
            return "INFO";
        case 'w':
            return "WARNING";
        case 'e':
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void log_message(char level, const char* message) {
    char* home = getenv("HOME");
    char* path = (char*) malloc(256);
    strcpy(path, home);
    strcat(path, "/log.txt");
    FILE* fp = fopen(path, "a");
    if (fp == NULL) {
        printf("Error: Failed to open file %s\n", path);
        exit(1);
    }
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);
    fprintf(fp, "[%s] [%s] %s\n", time_str, get_log_level(level).c_str(), message);
    fclose(fp);
}

void server_log(char level, const char* message) {
    char* home = getenv("HOME");
    char* path = (char*) malloc(256);
    strcpy(path, home);
    strcat(path, "/server_log.txt");
    FILE* fp = fopen(path, "a");
    if (fp == NULL) {
        printf("Error: Failed to open file %s\n", path);
        exit(1);
    }
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);
    fprintf(fp, "[%s] [%s] %s\n", time_str, get_log_level(level).c_str(), message);
    fclose(fp);
}