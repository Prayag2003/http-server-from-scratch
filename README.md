# HTTP Server

A lightweight, multi-threaded HTTP server written in C that demonstrates fundamental TCP socket programming, HTTP protocol basics, and POSIX thread management.

## Overview

This project implements a lightweight HTTP/1.1 web server using POSIX socket APIs. It listens for incoming connections on port 8000 and responds with dynamic HTML content. The server uses HTTP/1.1 protocol with explicit connection closure for reliable behavior across different clients. Each request is handled in its own thread, allowing the server to process multiple concurrent connections without blocking.


## 📚 Table of Contents

* [Features](#features)
* [Prerequisites](#prerequisites)
* [Installation](#installation)

  * [Clone the Repository](#clone-the-repository)
  * [Build the Server](#build-the-server)
* [Usage](#usage)

  * [Starting the Server](#starting-the-server)
  * [Connecting to the Server](#connecting-to-the-server)
  * [Expected Output](#expected-output)
* [Multi-threading Architecture](#multi-threading-architecture)

  * [Overview](#overview-1)
  * [Thread Creation](#thread-creation)
  * [Dynamic Thread Tracking](#dynamic-thread-tracking)
  * [Graceful Shutdown](#graceful-shutdown)
  * [Thread Function Signature](#thread-function-signature)
  * [Concurrency Considerations](#concurrency-considerations)
* [Screenshots and Demonstrations](#screenshots-and-demonstrations)

  * [HTTP/1.0 vs HTTP/1.1 Comparison](#http10-vs-http11-comparison)
* [Project Structure](#project-structure)

  * [Module Responsibilities](#module-responsibilities)
* [How It Works](#how-it-works)
* [Technical Details](#technical-details)

  * [HTTP Response Format](#http-response-format)
  * [Supported Routes](#supported-routes)
  * [HTTP Status Codes](#http-status-codes)
  * [Socket Options](#socket-options)
  * [Buffer Size](#buffer-size)
* [Compilation Flags](#compilation-flags)
* [Performance Benchmarking](#performance-benchmarking)
* [Memory Testing with Valgrind](#memory-testing-with-valgrind)
* [File Serving](#file-serving)
* [Limitations](#limitations)
* [Development & Extension](#development--extension)
* [Debugging](#debugging)
* [Cleaning Up](#cleaning-up)
* [References](#references)

## Features

- **HTTP/1.1 Server**: Implements HTTP/1.1 protocol with proper headers and status codes
- **Multi-threaded Connection Handling**: Each client connection is handled in a dedicated POSIX thread (`pthread`), enabling concurrent request processing
- **Dynamic Thread Pool**: Thread array grows dynamically via `realloc` as concurrent connections increase
- **Reliable Connection Handling**: Sends explicit `Connection: close` header to prevent client confusion
- **TCP Socket Programming**: Uses standard POSIX socket APIs (`socket`, `bind`, `listen`, `accept`)
- **Connection Management**: Properly handles connection lifecycle with clean termination per thread
- **File Serving**: Serves static files from the web root directory
- **Dynamic Routing**: Supports multiple URL routes with appropriate HTTP status codes
- **Clean C Code**: Well-documented with modern C standards (C17) and compiler flags

## Prerequisites

- **macOS** or **Linux** system
- **GCC** compiler (or compatible C compiler)
- **Make** utility
- Standard C library with socket support
- POSIX threads library (`-lpthread`)

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
gcc -Wall -Wextra -std=c17 -g -o server server.c -lpthread
```

The build process compiles `server.c` into an executable named `server` with optimizations, debugging symbols, and POSIX thread support.

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
- - - - - - - - - - New Connection - - - - - - - - - -
Parsed request line: method=GET, uri=/, version=HTTP/1.1
Closing connection 4
 - - - - - - - - - - - - - - - - - - - -
```

Multiple concurrent connections are handled in parallel, each logging independently:

```
Waiting for connection
Received a connection 5
- - - - - - - - - - New Connection - - - - - - - - - -
Received a connection 6
- - - - - - - - - - New Connection - - - - - - - - - -
Parsed request line: method=GET, uri=/, version=HTTP/1.1
Parsed request line: method=GET, uri=/assets/style.css, version=HTTP/1.1
```

## Multi-threading Architecture

### Overview

The server spawns a new POSIX thread for each incoming client connection, allowing it to handle multiple requests concurrently without blocking the main accept loop.

### Thread Creation

When a client connects, the main loop calls `pthread_create` with `handle_client_connection` as the thread function:

```c
pthread_t thread;
bind_result = pthread_create(&thread, NULL, handle_client_connection, (void *)(intptr_t)client_socket);
```

The client socket file descriptor is passed as a `void *` argument using `intptr_t` casting for portability.

### Dynamic Thread Tracking

Spawned thread IDs are stored in a dynamically allocated array. When the array reaches capacity, it doubles in size via `realloc`:

```c
if (thread_count + 1 > thread_capacity) {
    thread_capacity *= 2;
    pthread_t *new_threads = realloc(threads, thread_capacity * sizeof(pthread_t));
    if (!new_threads) {
        fprintf(stderr, "Warning: Failed to reallocate threads array, but server continues\n");
        thread_count = 0; // Reset to avoid overflow; threads still run
    } else {
        threads = new_threads;
    }
}
```

### Graceful Shutdown

On exit, the main thread joins all tracked threads to ensure clean termination:

```c
for (size_t i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
}
free(threads);
```

### Thread Function Signature

The handler function follows the POSIX thread signature, accepting and returning `void *`:

```c
void *handle_client_connection(void *client_socket_ptr) {
    int client_socket = (int)(intptr_t)client_socket_ptr;
    // ...
    return (void *)(intptr_t)result;
}
```

### Concurrency Considerations

- **No shared mutable state**: Each thread operates on its own client socket and local buffer, so no mutex is required for the current implementation.
- **Thread detachment**: Threads are currently tracked and joined. For fire-and-forget behavior, `pthread_detach` could be used instead.
- **Scalability**: The current model is one-thread-per-connection. Under high concurrency, a thread pool or event-driven model (e.g., `epoll`) would be more efficient.

## Screenshots and Demonstrations

### HTTP/1.0 vs HTTP/1.1 Comparison

| Aspect                   | HTTP/1.0                                                    | HTTP/1.1                                                          |
| ------------------------ | ----------------------------------------------------------- | ----------------------------------------------------------------- |
| **Screenshot**           | ![HTTP/1.0 request handling](assets/03_sending_http1.0.png) | ![HTTP/1.1 persistent connections](assets/04_sending_http1.1.png) |
| **Connection Model**     | Single request per TCP connection                           | Multiple requests per persistent TCP connection                   |
| **Connection Overhead**  | New socket accept and setup for each request                | One socket setup, multiple requests on same connection            |
| **Request Processing**   | Full connection teardown between requests                   | Connection remains open for sequential requests                   |
| **Performance**          | Higher latency due to connection overhead                   | Lower latency with connection reuse                               |
| **System Calls**         | More accept(), close() calls per request                    | Fewer system calls overall                                        |
| **Resource Utilization** | Less efficient, more file descriptors consumed              | More efficient, better resource reuse                             |
| **Keep-Alive Support**   | Limited or no persistent connection support                 | Full support for `Connection: keep-alive` and request pipelining  |
| **Use Case**             | Simple clients, HTTP/1.0 only browsers                      | Modern browsers, typical web clients                              |
| **Advantage**            | Simplicity, guaranteed isolation                            | Performance, efficiency, modern compliance                        |

**Impact**: The HTTP/1.1 upgrade results in fewer system calls, reduced latency, and better resource utilization, as demonstrated by the increased number of requests handled within a single connection.

### 01_strace_executable.png

![strace output showing system calls](assets/01_strace_executable.png)

This screenshot displays system call tracing using `strace` while the server is running. It demonstrates:

- **Socket operations**: `accept()` receiving client connections
- **File operations**: `newfilestat()`, `openat()`, and `sendfile()` system calls for file serving
- **I/O operations**: `read()` and `write()` calls for receiving HTTP requests and sending responses
- **HTTP handling**: Full HTTP request parsing showing headers like `User-Agent`, `Accept-Encoding`, and various browser-specific headers
- **Advanced file operations**: `sendfile()` system call for efficient file transmission to clients

The strace output shows the server processing requests and efficiently serving files using the `sendfile()` system call.

## Project Structure

```
http-server/
├── Makefile                    # Build configuration and targets
├── README.md                   # Project documentation
├── server.c                    # Main HTTP server implementation
├── assets/                     # Static assets and screenshots
│   ├── 00_server_responds_200.png
│   ├── 01_strace_executable.png      # System call tracing demonstration
│   └── 02_server_response.png        # Web interface and performance metrics
└── utils/                      # HTTP library utilities
    ├── http_common.h           # Common HTTP types and enums
    ├── http_request.h          # HTTP request parsing
    ├── http_response.h         # HTTP response generation (header + declarations)
    ├── http_response.c         # HTTP response implementation
    ├── string_ops.h            # String manipulation utilities
    ├── stat.h                  # File metadata retrieval
    └── http_types.h            # Convenience header including all types
```

### Module Responsibilities

- **server.c**: Main event loop, socket setup, thread spawning, and client connection handling
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
2. **Spawn**: Creates a new `pthread` for each connection
3. **Handle**: The thread processes the request independently
4. **Respond**: Sends an HTTP response with appropriate content
5. **Close**: The thread closes the client socket and exits

### 4. Client Handler

The `handle_client_connection()` function runs in its own thread:

- Reads incoming HTTP request data into a 4KB buffer
- Parses the request line (method, URI, version)
- Routes the request and serves the appropriate file or 404 response
- Closes the socket and returns

## Technical Details

### HTTP Response Format

HTTP/1.1 Response:

```
HTTP/1.1 200 OK\r\n
Content-Length: 21\r\n
Connection: close\r\n
\r\n
<h1>Hello, World!</h1>
```

**Response Components:**

- Status line: `HTTP/1.1 200 OK` (indicating HTTP/1.1 protocol compliance)
- Headers:
  - `Content-Length`: Indicates the size of the response body in bytes
  - `Connection: close`: Signals the client that the server will close the connection after sending the response
- Empty line (carriage return + line feed) separating headers from body
- Response body: HTML content or file data

**Connection Management:**

The server sends `Connection: close` header for each response to ensure proper connection termination:

- **Client Clarity**: Explicitly tells the browser when to expect the connection to close
- **Prevents Hanging**: Prevents clients from waiting indefinitely for more data or requests
- **Single Request Per Connection**: Currently, each thread handles one request per connection and closes it, simplifying the implementation
- **Reliable Behavior**: Ensures consistent behavior across different browsers and HTTP clients

### Supported Routes

The server implements basic URL routing:

| Route           | Response      | Status |
| --------------- | ------------- | ------ |
| `/`             | Home page     | 200 OK |
| `/hello`        | Hello message | 200 OK |
| `/assets/*`     | Static files  | 200 OK |
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
| `-lpthread`| Link POSIX threads library                     |
| `-lm`      | Link math library (included for compatibility) |

## Performance Benchmarking

### Siege Load Testing Results

The server was benchmarked using [Siege](https://www.joedog.org/), a regression testing and load testing utility:

```bash
siege -b http://127.0.0.1:8000/
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

The server successfully handled over 300,000 transactions with a 99.65% success rate, demonstrating solid performance characteristics for a multi-threaded HTTP server.

## Memory Testing with Valgrind

The server was tested with Valgrind to detect memory leaks and invalid memory accesses:

```bash
valgrind --leak-check=full --track-origins=yes ./server
```

Key checks performed:

- **Heap leak detection**: Verified all `malloc`/`calloc`/`realloc` allocations are properly freed on exit
- **Thread memory**: Confirmed per-thread stack buffers are correctly scoped and released
- **Invalid reads/writes**: Checked buffer bounds in the 4KB request buffer and `string_view` pointer arithmetic
- **Conditional jumps on uninitialised values**: Validated `memset` usage before `read()` calls

```bash
# Example Valgrind output (clean run)
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 12 allocs, 12 frees, 1,024 bytes allocated
==12345== All heap blocks were freed -- no leaks are possible
```

## File Serving

The server includes file serving capabilities through the `stat.h` module:

### File Metadata Retrieval

The `fs_get_metadata()` function retrieves file information:

```c
typedef struct {
    bool exists;
    ssize_t file_size;
} fs_metadata;

fs_metadata fs_get_metadata(string_view filename);
```

This function:

- Validates filename length and buffer constraints
- Uses POSIX `stat()` to retrieve file metadata
- Returns file existence and size information
- Safely handles `string_view` pointer arithmetic (`end - start`)

## Limitations

- **One-thread-per-connection**: Spawning a thread per connection works well under moderate load but does not scale to very high concurrency; a thread pool or `epoll`-based event loop would be more efficient
- **No SSL/TLS**: Communication is unencrypted
- **No request pipelining**: The server closes the connection after each response

## Development & Extension

### Adding Features

Consider these enhancements:

1. **Thread pool**: Pre-allocate a fixed set of worker threads to avoid per-connection spawn overhead
2. **epoll/kqueue**: Replace blocking `accept` with event-driven I/O for better scalability
3. **Request parsing**: Parse full HTTP headers and handle different request types
4. **Logging**: Add structured logging with timestamps and thread IDs
5. **SSL/TLS**: Add HTTPS support using OpenSSL

### Debugging

Run with GDB:

```bash
make clean
make
gdb ./server
(gdb) run
```

Debug threading issues with Helgrind (Valgrind's thread error detector):

```bash
valgrind --tool=helgrind ./server
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

## References

- [RFC 1945 for HTTP 1.0](https://datatracker.ietf.org/doc/html/rfc1945)
- [RFC 2616 for HTTP 1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [POSIX Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)
- [POSIX Threads (pthreads)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
