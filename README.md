# 🌐 webserv

A lightweight, non-blocking HTTP/1.1 web server written in C++98.  
This project is part of **[42 Paris](https://42.fr/)** curriculum and aims to help students understand **network programming**, **HTTP protocol**, and **server architecture** by building their own web server from scratch.  

---

## 📖 Project Goals

- Build a **minimal HTTP server** supporting `GET`, `POST`, and `DELETE`
- Handle multiple clients simultaneously using **non-blocking I/O**
- Implement a **configuration file parser** (similar to NGINX)
- Support **CGI execution** (e.g., PHP scripts)
- Manage errors gracefully and return proper HTTP status codes

---

## ▶️ Usage
Run the server with a configuration file:  
./webserv path/to/config.conf

Example:  
./webserv configs/default.conf  

Then open your browser and go to:  
http://localhost:8080

---

### 🔌 Sockets & TCP/IP
A socket is an endpoint for communication between two machines.  
Steps involved in a basic server:
1. **Create** a socket (AF_INET, SOCK_STREAM for TCP)
2. **Bind** the socket to an IP + port
3. **Listen** and **accept** connections
4. **Send/receive** data
5. **Close** the socket when done  

📚 [Socket Programming in C++ (GeeksforGeeks)](https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/)

---

### ⏳ Non-Blocking I/O & Multiplexing
To handle many clients, the server must avoid blocking calls.  
- **select()** – simple, portable, but O(n) complexity (needs to scan all file descriptors every time)  
- **epoll()** – Linux-specific, much more scalable (register FDs once, O(1) notifications)

📚 [Blocking vs Non-Blocking Sockets](https://dev.to/vivekyadav200988/understanding-blocking-and-non-blocking-sockets-in-c-programming-a-comprehensive-guide-2ien)  
📚 [select vs poll vs epoll](https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/)  
📚 [Epoll In-Depth](https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642)

---

### ⚙️ Configuration File Parsing
Your server should:
- Parse a configuration file inspired by NGINX
- Validate brackets, server blocks, ports (no duplicates)
- Verify root paths & locations
- Load only minimal necessary directives  

📚 [Nginx vs Apache (Performance Overview)](https://djangodeployment.com/2016/11/15/why-nginx-is-faster-than-apache-and-why-you-neednt-necessarily-care/)

---

### 📄 HTTP/1.1 Requests & Responses
- Only handle `GET`, `POST`, and `DELETE`
- Properly parse request line, headers, and body
- Return appropriate status codes (e.g. 200, 404, 405, 500)

📚 [MDN HTTP Status Codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)  
📚 [RFC 9112 – HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc9112)

---

### 🖥️ CGI (Common Gateway Interface)
CGI allows the server to run external scripts (e.g., PHP, Python) and return their output as HTTP responses.

Key system calls:
- `fork()` → create child process  
- `execve()` → execute script (php-cgi, python…)  
- `waitpid()` → wait for child process to finish  
- `kill()` → terminate script if it hangs  
- `signal()` → handle child termination and server shutdown  

📚 [RFC 3875 – CGI](https://datatracker.ietf.org/doc/html/rfc3875)  
📚 [Wikipedia – CGI](https://en.wikipedia.org/wiki/Common_Gateway_Interface)

---

## 📌 Roadmap

- [x] Research configuration files & NGINX basics  
- [x] Implement minimal parser & validation  
- [x] Create basic TCP server  
- [x] Add select() event loop  
- [x] Parse HTTP requests & send basic responses  
- [ ] Implement CGI handling  
- [ ] Handle errors & edge cases gracefully  
- [ ] Add autoindex & redirection support  

---

## 📚 Learning Resources

### 🎥 Videos
- [How Webservers Work](https://www.youtube.com/watch?v=9J1nJOivdyw)
- [APIs Explained](https://www.youtube.com/watch?v=ByGJQzlzxQg)
- [Networking Basics – OSI Model](https://www.youtube.com/watch?v=26jazyc7VNk)
- [Understanding Ports](https://www.youtube.com/watch?v=YSl6bordSh8)
- [Concurrency & Processes](https://www.youtube.com/watch?v=RlM9AfWf1WU)

### 📖 Articles & Documentation
- [Building a Simple HTTP Server in C++](http://ncona.com/2019/04/building-a-simple-server-with-cpp/)
- [Blocking vs Non-Blocking I/O](https://eklitzke.org/blocking-io-nonblocking-io-and-epoll)
- [HTTP: The Definitive Guide (Book)](https://dl.ebooksworld.ir/books/HTTP.The.Definitive.Guide.Brian.Totty.David.Gourley.OReilly.9781565925090.EBooksWorld.ir.pdf)
- [Minimal Webserv Introduction](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7)

---

## 👩‍💻 Authors

**Nina Dabboussi and Maxime Prokosch** – student at [42 Paris](https://42.fr/), graduating in March 2026.  



