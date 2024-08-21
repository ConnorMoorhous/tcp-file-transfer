# TCP File Transfer Server and Client

## Overview
This project implements a basic TCP server and client for file transfer. The server listens on a specified port and saves incoming files to a designated directory, while the client sends files to the server over a TCP connection.

## Installation

To compile the server and client programs, navigate to the project directory and use the following commands:

```bash
g++ -o server server.cpp
g++ -o client client.cpp
```

Ensure that you have `g++` installed on your system. If not, you can install it using your package manager:

```bash
sudo apt-get install g++
```

## Usage

### Running the Server
Start the server by specifying a port number and the directory where files should be saved:

```bash
./server 8080 /path/to/save/files
```

### Running the Client
Use the client to send a file to the server:

```bash
./client 127.0.0.1 8080 /path/to/send/file.txt
```

Replace `127.0.0.1` with the server's IP address if running on a different machine.

## Examples

### Example 1: Sending a Text File

Start the server:
```bash
./server 8080 /tmp/files
```

Send a file from the client:
```bash
./client 127.0.0.1 8080 example.txt
```

The file `example.txt` will be received by the server and saved to `/tmp/files/example.txt`.

## Server Design

### Command-Line Arguments
```
$ ./server <PORT> <FILE-DIR>
```
- `<PORT>`: The port number on which the server will listen for connections.
- `<FILE-DIR>`: The directory path (relative or absolute) where received files are saved.

### Functions

```c++
void receiveFile(int socket, string filePath);
```
- Receives data from the socket and writes it to the specified file path. Handles timeouts and errors gracefully.

```c++
void signalHandler(int sigNum);
```
- Handles signals such as `SIGTERM` or `SIGQUIT` for graceful shutdown.

### Main Function

The `main()` function initializes the server by processing the command-line arguments, creating a TCP socket, and setting it to listen mode. The server then handles connections by receiving files and saving them to the specified directory. The process repeats until a defined number of connections are processed, a stop signal is received, or an error occurs.

## Client Design

### Command-Line Arguments
```
$ ./client <HOSTNAME-OR-IP> <PORT> <FILENAME>
```
- `<HOSTNAME-OR-IP>`: The hostname or IP address of the server.
- `<PORT>`: The port number on which the server is listening.
- `<FILENAME>`: The path to the file to be sent.

### Functions

```c++
void sendFile(int socket, string fileName);
```
- Handles the process of sending the file to the server.

```c++
void signalHandler(int sigNum);
```
- Handles signals such as `SIGTERM` or `SIGQUIT`.
