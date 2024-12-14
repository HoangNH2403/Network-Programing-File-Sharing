#ifndef __VALIDATE_H__
#define __VALIDATE_H__

#define INVALID_IP -1

/**
 * Validate ip address
 * @param ip ip address
 * @return 0 if valid, -1 if invalid
 */
int validate_ip(const char *ip);

/**
 * Check if path is a folder
 * @param path path to check
 * @return 1 if path is a folder, 0 if not
 */
int is_folder(const char *path);

/**
 * validate path is file or directory
 * @param path path to validate
 * @return 0 if valid, -1 if invalid
 */
int validate_file_or_dir(const char *path);

#endif   // !
