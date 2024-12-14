#ifndef __LOG_H__
#define __LOG_H__

/**
 * Log message to file
 * @param level log level [i]nfo, [w]arning, [e]rror
 * @param message log message
 */
void log_message(char level, const char* message);

/**
 * Log message to file but for server
 * @param level log level [i]nfo, [w]arning, [e]rror
 * @param message log message
 */
void server_log(char level, const char* message);

#endif   // !__LOG_H__