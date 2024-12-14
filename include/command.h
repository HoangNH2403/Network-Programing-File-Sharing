#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <string>

/**
 * Run command on server, send result to client over sockfd
 * @param sockfd: socket to send result to
 * @param cmd: command to run
 * @param cur_dir: current directory of user
 */
int process_command(int sockfd, char* cmd, char* cur_dir);

/**
 * Check if directory is subdirectory of base_dir
 * @param base_dir: base directory
 * @param dir: directory to check
 * @return 1 if true, 0 if false
 */
int is_subdir(const char* base_dir, const char* dir);

/**
 * Change directory
 * Return -1 on error, 0 on success
 */
int server_cd(int sock_control, char* folderName, char* user_dir, char* cur_user_dir);

/**
 * @brief List files and directories in current working directory
 * @param sockfd Socket to receive data
 * @return 0 if success, -1 if error
 */
int handle_list(int sockfd);

/**
 * @brief Server handle list files and directories in current working directory
 * @param sockfd Socket to send data
 * @return 0 if success, -1 if error
 */
int server_list(int sockfd);

/**
 * get current directory
 * @param sockfd socket control
 */
const char* handle_pwd(char* cur_user, char* user_dir);

/**
 * Quit server
 * @param sockfd socket
 * @param cur_user current user
 */
void server_quit(int sockfd, char* cur_user);

/**
 * List files in current directory
 * @param sockfd: socket to send result to
 * @param arg: argument of command
 * @return 0 if success, -1 if error
 */
int server_find(int sockfd, char* arg);

/**
 * share file to another user
 * @param sockfd: socket to send result to
 * @param arg: argument of command
 * @param user_dir: current directory of user
 * @return 0 if success, -1 if error
 */
int server_share(int sockfd, char* arg, char* user_dir);
/**
 * Client download file or folder from server
 * @param data_sock socket data
 * @param sock_control socket control
 * @param arg file or folder to download
 * @return 0 on success, -1 on error
 */
int handle_download(int data_sock, int sock_control, char* arg);

/**
 * Download file or folder from server
 * @param sock_control socket control
 * @param sock_data socket data
 * @param dir directory to download
 */
int server_download(int sock_control, int sock_data, char* dir, char* user_dir);
void old_server_download(int sock_control, int sock_data, char* dir);


/**
 * Download list of files from server
 */
int handle_pipe_download(int sockfd, std::string files);
void server_pipe_download(int sockfd, char* output);

void handle_upload(int data_sock, char* filename, int sock_control);
int server_upload(int sock_control, int sock_data, char* filename, char* user_dir);

/**
 * Load shared file to shared folder of each user
 */
int load_shared_file(char* user_dir);

int check_permision(const char* dir, const char* share_path, char* share_folder_path);

int is_current_share_folder(char* dir, char* share_folder_path);
int download_from_share_folder(char *dir, char *share_folder_path);
#endif   // !__COMMAND_H__