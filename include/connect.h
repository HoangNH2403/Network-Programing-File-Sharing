#ifndef __CONNECT_H__
#define __CONNECT_H__

#include "message.h"

/**
 * Send message over socket
 * @param sockfd socket to send message
 * @param msg message to send
 * @return 0 if success, -1 if error
 */
int send_message(int sockfd, Message msg);

/**
 * Receive message over socket
 * @param sockfd socket to receive message
 * @param msg message to receive
 * @return 0 if success, -1 if error
 */
int recv_message(int sockfd, Message *msg);

/**
 * Creates new socket and binds it to the specified port.
 *
 * @param port The port number to bind the socket to.
 * @return The file descriptor of the created socket, or -1 if an error occurred.
 */
int socket_create(int port);

/**
 * Accepts a connection on a listening socket.
 *
 * @param sock_listen The listening socket descriptor.
 * @return The socket descriptor for the accepted connection, or -1 if an error occurred.
 */
int socket_accept(int sock_listen);
int client_start_conn(int sock_con);

int socket_connect(int port, char *host);
int server_start_conn(int sock_control);

#endif