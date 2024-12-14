#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "command.h"
#include "common.h"
#include "log.h"
#include "utils.h"

void server_quit(int sockfd, char* cur_user) {
    chdir(root_dir);
    toggle_lock(cur_user, 0);
    char* msg = new char[SIZE];
    sprintf(msg, "User %s logged out!\n", cur_user);
    server_log('i', msg);
    close(sockfd);
    delete[] msg;
}
