#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

const char *CRLF = "\r\n";
const char SPACE = ' ';

typedef struct
{
    const char *data;
    size_t len;
} string;

typedef struct
{
    string method;
    string uri;
    string version;
} http_req_line;

bool strings_equal(string a, string b)
{
    if (a.len != b.len)
    {
        return false;
    }
    return strncmp(a.data, b.data, a.len) == 0;
}

typedef struct
{
    char *start;
    char *end;
} string_view;

typedef struct
{
    string_view *splits;
    size_t count;
    size_t capacity;
} string_splits;

static string_splits split_string(const char *buf, size_t len, char split_by);
static void free_splits(string_splits *splits);

string string_from_cstr(const char *cstr)
{
    string result;
    result.data = cstr;
    result.len = strlen(cstr);
    return result;
}

http_req_line http_req_line_init()
{
    http_req_line line;
    memset(&line, 0, sizeof(line));
    return line;
};

typedef enum
{
    HTTP_RES_OK = 200,
    HTTP_RES_INTERNAL_SERVER_ERR = 500,
    HTTP_RES_BAD_REQUEST = 400,
} http_result;

http_result parse_req_line(const char *buf, size_t len, http_req_line *req_line)
{
    if (!buf || !req_line)
    {
        return HTTP_RES_INTERNAL_SERVER_ERR;
    }

    string_splits splits = split_string(buf, len, SPACE);
    if (splits.count != 3)
    {
        printf("Invalid request line, expected 3 parts but got %zu\n", splits.count);
        return HTTP_RES_BAD_REQUEST;
    }
    req_line->method.data = splits.splits[0].start;
    req_line->method.len = splits.splits[0].end - splits.splits[0].start;
    req_line->uri.data = splits.splits[1].start;
    req_line->uri.len = splits.splits[1].end - splits.splits[1].start;
    req_line->version.data = splits.splits[2].start;
    req_line->version.len = splits.splits[2].end - splits.splits[2].start;

    free_splits(&splits);
    return HTTP_RES_OK;
};

static string_splits split_string(const char *buf, size_t len, char split_by)
{
    string_splits result;
    char *start;
    char *end;
    size_t result_i = 0;

    result.capacity = 8;
    result.count = 0;
    result.splits = calloc(sizeof(string_view), result.capacity);

    start = (char *)buf; /* Initialize start to beginning of buffer */

    for (size_t i = 0; i < len; i++)
    {
        if (buf[i] == split_by)
        {
            result.splits[result_i].start = start;
            result.splits[result_i].end = &buf[i];
            result_i++;
            result.count++;
            start = &buf[i] + 1; /* Skip past the delimiter */

            if (result.count == result.capacity)
            {
                result.capacity *= 2;
                string_view *new_splits = realloc(result.splits, sizeof(string_view) * result.capacity);
                if (!new_splits)
                {
                    perror("Failed to reallocate memory for string splits\n");
                    abort();
                }
                result.splits = new_splits;
            }
        }
    }

    /* Capture the final token after the last delimiter */
    if (start < &buf[len])
    {
        result.splits[result_i].start = start;
        result.splits[result_i].end = &buf[len];
        result.count++;
    }

    return result;
}

static void free_splits(string_splits *splits)
{
    if (splits)
    {
        free(splits->splits);
        splits->splits = NULL;
    }
}

/**
 * handle_client_connection: Process incoming client connection
 * @client_socket: File descriptor of the connected client socket
 */
ssize_t handle_client_connection(int client_socket)
{
    ssize_t n = 0;
    char buffer[4096];
    // Request line - CRLF - Entity Body - CRLF
    const char *response_from_server = "HTTP/1.0 200 OK\r\n\r\n<h1>Hello, World!</h1>";

    for (;;)
    {
        memset(buffer, 0, sizeof(buffer));

        /* Receive incoming data from client socket */
        n = read(client_socket, buffer, sizeof(buffer) - 1);
        if (n < 0)
        {
            perror("Failed to read the data from the client socket\n");
            return -1;
        }
        if (n == 0)
        {
            printf("Connection closed gracfully\n");
            return 0;
        }
        printf(" - - - - - - - - - - - - - - - - - - - -\n");
        printf("REQUEST: \n%s\n", buffer);

        /* Find the first line (request line) terminated by \r\n or \n */
        size_t req_line_len = 0;
        for (size_t i = 0; i < (size_t)n - 1; i++)
        {
            if (buffer[i] == '\r' && buffer[i + 1] == '\n')
            {
                req_line_len = i;
                break;
            }
            if (buffer[i] == '\n')
            {
                req_line_len = i;
                break;
            }
        }

        if (req_line_len == 0)
        {
            printf("Invalid HTTP request, could not find request line terminator\n");
            return -1;
        }

        http_req_line req_line = http_req_line_init();
        http_result result = parse_req_line(buffer, req_line_len, &req_line);
        if (result != HTTP_RES_OK)
        {
            printf("Failed to parse request line\n");
            return -1;
        }

        string route_hello = string_from_cstr("/hello");
        string route_goodbye = string_from_cstr("/goodbye");
        if (strings_equal(req_line.uri, route_hello))
        {
            response_from_server = "HTTP/1.0 200 OK\r\n\r\n<h1>Hello, World!</h1>";
        }
        else if (strings_equal(req_line.uri, route_goodbye))
        {
            response_from_server = "HTTP/1.0 200 OK\r\n\r\n<h1>Goodbye, World!</h1>";
        }
        else
        {
            response_from_server = "HTTP/1.0 404 Not Found\r\n\r\n<h1>Not Found</h1>";
        }

        (void)write(client_socket, response_from_server, strlen(response_from_server));
        close(client_socket);
        break;
    }
    printf(" - - - - - - - - - - - - - - - - - - - -\n");

    return 0;
}

/**
 * Creates a TCP socket, binds to port 8000, and accepts incoming connections.
 * Returns 0 on success, 1 on error.
 */
int main()
{
    int ret = 0;
    int rc = 0;
    int tcp_socket = 0;
    int ls = 0;
    int client_socket = 0;
    struct sockaddr_in bind_addr;
    int optionsEnabled = 1;

    memset(&bind_addr, 0, sizeof(bind_addr));

    /* Create TCP socket (AF_INET=IPv4, SOCK_STREAM=TCP) */
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcp_socket < 0)
    {
        perror("Failed to create a socket, fd = -1\n");
        return 0;
    }

    printf("Socket created successfully, fd = %d\n", tcp_socket);

    (void)setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &optionsEnabled, sizeof(optionsEnabled));

    /* Configure socket address for binding to port 8000 */
    bind_addr.sin_family = AF_INET;         /* IPv4 */
    bind_addr.sin_port = htons(8000);       /* Port 8000 */
    bind_addr.sin_addr.s_addr = INADDR_ANY; /* Listen on all interfaces */

    rc = bind(tcp_socket, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (rc < 0)
    {
        perror("Failed to bind the socket\n");
        ret = 1;
        goto exit;
    }

    /* Mark socket as passive to accept incoming connections */
    ls = listen(tcp_socket, SOMAXCONN);
    if (ls < 0)
    {
        perror("Failed to create the listener\n");
        ret = 1;
        goto exit;
    }
    printf("Listener succeeded\n");

    /* Accept and handle incoming client connections */
    for (;;)
    {
        printf("Waiting for connection\n");
        client_socket = accept(tcp_socket, NULL, NULL);
        if (client_socket < 0)
        {
            perror("Failed to create the client connection\n");
            ret = 1;
            goto exit;
        }
        printf("Received a connection %d\n", client_socket);
        rc = handle_client_connection(client_socket);
    }

exit:
    close(tcp_socket);
    return ret;
}