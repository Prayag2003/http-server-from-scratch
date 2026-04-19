# HTTP Server

A lightweight, single-threaded HTTP server written in C that demonstrates fundamental TCP socket programming and HTTP protocol basics.

## Overview

This project implements a basic HTTP/1.0 web server using POSIX socket APIs. It listens for incoming connections on port 8000 and responds with a simple HTML response. The server is designed for educational purposes and as a foundation for understanding network programming concepts.

## Features

- **Simple HTTP/1.0 Server**: Implements basic HTTP protocol compliance
- **TCP Socket Programming**: Uses standard POSIX socket APIs (`socket`, `bind`, `listen`, `accept`)
- **Multiple Connection Handling**: Accepts sequential client connections in a loop
- **Clean C Code**: Well-documented with modern C standards (C17) and compiler flags

## Prerequisites

- **macOS** or **Linux** system
- **GCC** compiler (or compatible C compiler)
- **Make** utility
- Standard C library with socket support

## Installation

### Clone the Repository

```bash
git clone https://github.com/Prayag2003/http-server-from-scratch
cd http-server-from-scratch
```

### Build the Server

```bash
make
```

Or build manually:

```bash
gcc -Wall -Wextra -std=c17 -g -o server server.c
```

The build process compiles `server.c` into an executable named `server` with optimizations and debugging symbols included.

## Usage

### Starting the Server

```bash
make run
```

Or start directly:

```bash
./server
```

### Connecting to the Server

Once running, connect using any HTTP client:

**Using curl:**

```bash
curl http://localhost:8000
```

**Using a web browser:**

Navigate to `http://localhost:8000`

**Using netcat:**

```bash
nc localhost 8000
```

### Expected Output

The server will display:

```
Socket created successfully, fd = 3
Listener succeeded
Waiting for connection
Received a connection 4
 - - - - - - - - - - - - - - - - - - - -
Buffer => GET / HTTP/1.1
Host: localhost:8000
...
 - - - - - - - - - - - - - - - - - - - -
Connection closed gracefully
```

The client will receive:

```
HTTP/1.0 200 OK

<h1>Hello, World!</h1>
```

## Project Structure

```
http-server/
├── Makefile                    # Build configuration and targets
├── README.md                   # Project documentation
├── server.c                    # Main HTTP server implementation
├── assets/                     # Static assets
│   └── 00_server_responds_200.png
└── utils/                      # HTTP library utilities
    ├── http_common.h           # Common HTTP types and enums
    ├── http_request.h          # HTTP request parsing
    ├── http_response.h         # HTTP response generation (header + declarations)
    ├── http_response.c         # HTTP response implementation
    ├── string_ops.h            # String manipulation utilities
    └── http_types.h            # Convenience header including all types
```

### Module Responsibilities

- **server.c**: Main event loop, socket setup, client connection handling
- **http_response.h/c**: Response header generation and socket transmission
- **http_request.h**: Request line parsing and validation
- **http_common.h**: HTTP status codes and common type definitions
- **string_ops.h**: String and memory operations for HTTP parsing

## How It Works

### 1. Socket Creation

The server creates a TCP socket using IPv4 addressing:

```c
tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
```

### 2. Socket Configuration

- Enables address reuse to avoid "Address already in use" errors
- Binds to all available network interfaces (`INADDR_ANY`)
- Listens on port **8000**

### 3. Connection Loop

The server runs an infinite loop:

1. **Accept**: Waits for incoming client connections
2. **Handle**: Processes the client request
3. **Respond**: Sends an HTTP 200 OK response with HTML content
4. **Close**: Closes the client connection

### 4. Client Handler

The `handle_client_connection()` function:

- Reads incoming HTTP request data into a 4KB buffer
- Prints the request for debugging
- Sends a hardcoded HTTP response
- Closes the socket

## Technical Details

### HTTP Response Format

```
HTTP/1.0 200 OK\r\n\r\n<h1>Hello, World!</h1>
```

- Status line: `HTTP/1.0 200 OK`
- Empty line (carriage return + line feed)
- Response body: Simple HTML

### Supported Routes

The server implements basic URL routing:

| Route           | Response      | Status |
| --------------- | ------------- | ------ |
| `/`             | Home page     | 200 OK |
| `/hello`        | Hello message | 200 OK |
| `*` (any other) | 404 Not Found | 404    |

### HTTP Status Codes

Supported status codes defined in `http_common.h`:

```c
typedef enum http_status {
    HTTP_RES_OK = 200,
    HTTP_RES_INTERNAL_SERVER_ERR = 500,
    HTTP_RES_BAD_REQUEST = 400,
    HTTP_RES_NOT_FOUND = 404,
} http_status;
```

### Socket Options

- `SO_REUSEADDR`: Allows rapid server restarts by reusing the listening port

### Buffer Size

- Request buffer: 4096 bytes (4 KB)

## Compilation Flags

| Flag       | Purpose                                        |
| ---------- | ---------------------------------------------- |
| `-Wall`    | Enable all common compiler warnings            |
| `-Wextra`  | Enable extra compiler warnings                 |
| `-std=c17` | Use C17 standard                               |
| `-g`       | Include debugging symbols                      |
| `-lm`      | Link math library (included for compatibility) |

## Performance Benchmarking

### Siege Load Testing Results

The server was benchmarked using [Siege](https://www.joedog.org/), a regression testing and load testing utility:

```bash
siege -b http://127.0.0.1:8000/hello
```

**Results:**

```json
{
	"transactions": 300683,
	"availability": 99.65,
	"elapsed_time": 32.24,
	"data_transferred": 6.31,
	"response_time": 0.0,
	"transaction_rate": 9326.4,
	"throughput": 0.2,
	"concurrency": 24.44,
	"successful_transactions": 300683,
	"failed_transactions": 1048,
	"longest_transaction": 0.68,
	"shortest_transaction": 0.0
}
```

**Performance Metrics:**

| Metric                     | Value     |
| -------------------------- | --------- |
| **Transactions/sec**       | 9,326.40  |
| **Availability**           | 99.65%    |
| **Avg Response Time**      | 0.00 sec  |
| **Data Transferred**       | 6.31 MB   |
| **Elapsed Time**           | 32.24 sec |
| **Concurrent Connections** | 24.44     |
| **Longest Transaction**    | 0.68 sec  |

The server successfully handled over 300,000 transactions with a 99.65% success rate, demonstrating solid performance characteristics for a single-threaded HTTP server.

## Limitations

- **Single-threaded**: Handles one client at a time sequentially
- **Hardcoded response**: Always returns the same "Hello, World!" message
- **HTTP/1.0 only**: No support for HTTP/1.1 features like keep-alive
- **No SSL/TLS**: Communication is unencrypted
- **No static file serving**: Cannot serve files from the filesystem

## Development & Extension

### Adding Features

Consider these enhancements:

1. **Multi-threading**: Use pthreads to handle multiple simultaneous connections
2. **Request parsing**: Parse HTTP headers and handle different request types
3. **Routing**: Implement URL routing to serve different content
4. **Static files**: Serve files from a directory
5. **Logging**: Add structured logging
6. **SSL/TLS**: Add HTTPS support using OpenSSL

### Debugging

Run with GDB:

```bash
make clean
make
gdb ./server
(gdb) run
```

Monitor network activity:

```bash
netstat -an | grep 8000
```

## Cleaning Up

Remove compiled objects and executable:

```bash
make clean
```

## License

This project is provided as-is for educational purposes.

## References

- [POSIX Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)
- [HTTP/1.0 Specification](https://tools.ietf.org/html/rfc1945)
- [C Standard Library](https://en.cppreference.com/w/c)

## Notes

This is a foundational project for learning network programming concepts. The current implementation is intentionally simple for clarity. Production use would require significant enhancements including error handling, security measures, and performance optimizations.
