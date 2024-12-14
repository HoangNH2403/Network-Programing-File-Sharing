#ifndef __UTILS_H__
#define __UTILS_H__

#include <sstream>
#include <string>
#include <vector>

#include "color.h"

#define UP_ARROW   "\033[A"
#define DOWN_ARROW "\033[B"

/**
 * Check if a process is a client
 * @param process process name
 * @return 1 if true, 0 if false
*/
int is_client(const char *process);

void read_input(char *user_input, int size);

/**
 * get username from path
 * @param path path to user storage
 * @return char pointer to username
 */
char *get_username(char *path);

/**
 * create user storage
 * @param path path to user storage
 * @return 0 if success, -1 if error
 */
int create_dir(const char *path);

/**
 * create file if not exist
 * @param path path to file
 * @return 0 if success, -1 if error
 */
int create_file(const char *path);

// Function to lock or unlock a user in the .auth file (0 for unlock, 1 for lock)
void toggle_lock(const char *username, int lockStatus);

/**
 * print a string centered on the screen
 @param text the string to print
*/
void print_centered(const char *text);

/**
 * Print menu and get user selection
 * @param menu_items array of options to display
 * @param num_items number of options
 */
int process_menu(const char *menu_items[], int num_items);

/**
 * get prompt string from current user and current directory
 * @param cur_user current user
 * @param user_dir current directory
 * @return prompt string
 */
char *handle_prompt(char *cur_user, char *user_dir);

/**
 * Trim whitespace and line ending
 * characters from a string
 */
void trimstr(char *str, int n);

/**
 * Split a string by a delimiter
 * @param s string to split
 * @param delim delimiter
 * @return vector of strings
 */
std::vector<std::string> split(const std::string &s, char delim);

/**
 * compress a folder
 * @param folder path to folder
 * @param compress_path path to compressed file
 */
void zip(const char *folder, const char *compress_path);

/**
 * decompress a folder
 * @param compressed_path path to compressed folder
 * @param extract_path path to extract folder
 */
void unzip(const char *compressed_path, const char *extract_path);

#endif   // !__UTILS