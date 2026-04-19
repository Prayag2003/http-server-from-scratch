#ifndef HTTP_SERVE_H
#define HTTP_SERVE_H

#include "string_ops.h"
#include "http_common.h"
#include <stdbool.h>

/**
 * Serve a file to a client socket
 * @client_socket: Socket file descriptor to send file to
 * @filename: Path to the file to serve
 * @return: true on success, false on failure
 */
bool http_serve_file(int client_socket, string filename);

#endif /* HTTP_SERVE_H */
