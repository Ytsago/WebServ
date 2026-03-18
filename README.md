*this project has been created as part of the 42 curriculum by halnuma , secros*
# WebServ
## Description
**webserv** is a high-performance, non-blocking HTTP/1.1 server written entirely in C++ 98. The core objective of this project is to implement the fundamental mechanics of the HTTP protocol and socket programming from scratch, without reliance on modern frameworks.
We used nginx as a reference to build our Webserv

The architecture is built upon an asynchronous, single-process event loop utilizing `epoll` (via the custom `AEventHandler` class). This allows the server to multiplex I/O operations and handle multiple concurrent client connections efficiently. The server maps virtual URIs to physical file paths, parses complex configuration files, serves static assets, dynamically generates directory listings (autoindex), and acts as a gateway for external CGI scripts.

### Key Features
* **Asynchronous I/O Multiplexing:** Utilizes `epoll_ctl` and `epoll_wait` to monitor read/write readiness across all active sockets, ensuring $O(1)$ event scalability.
* **HTTP/1.1 Compliance:** Adheres to the semantic standards defined in **RFC 9110**, robustly processing GET, POST, and DELETE requests.
* **Dynamic Autoindex:** Automatically generates an HTML directory listing (with optimized $O(n)$ string buffering) when a requested folder lacks a default index file.
* **CGI Execution:** Interfaces with external scripts (e.g., Python, PHP) via the Common Gateway Interface to serve dynamic content.
* **Resilience & Timeout Management:** Implements strict connection lifecycle management within `AEventHandler` to track idle connections and prevent resource exhaustion.

---

## Instructions

### Compilation
The project relies on a standard `Makefile` and requires a Unix-based system with a C++98 compliant compiler (e.g., `g++` or `clang++`).

```bash
# Compile the executable
make

# Remove intermediate object files
make clean

# Remove all compiled binaries
make fclean

# Recompile from scratch
make re
```


## Architectural design


![[Pasted image 20260318151658.png]]

## Resources

**Epoll**
see man

**Http Protocol**
https://developer.mozilla.org/fr/docs/Web/HTTP
https://www.rfc-editor.org/rfc/rfc9110.html (and 9111 and 9112)

**Nginx**
https://blog.nginx.org/blog/inside-nginx-how-we-designed-for-performance-scale
https://aosabook.org/en/v2/nginx.html
https://www.alimnaqvi.com/blog/webserv
