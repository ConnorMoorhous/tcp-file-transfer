# CSCI 3550 Project-1-Accio

Name: Connor Moorhous <br/>
NUID: 87038425

## Server Design

### Command-Line Arguments
```
$ server <PORT> <FILE-DIR>
```
<ul>
    <li>&lt;PORT&gt; : the port number on which the server will listen on connections. Must be a valid port number > 1023. </li>
    <li>&lt;FILE-DIR&gt; : the directory path (relative or absolute) where received files are to be saved.</li>
</ul>

### Server Functions

```c++
void receiveFile(int socket, string filePath);
```
The function `receiveFile()` takes a connected socket and file path, creates/opens the file at the path for writing, receives data from the socket, and writes that data to the opened file. If no data is received from the socket for over 10 seconds, a timeout occurs which discards the current contents of opened file, writes an `ERROR` string to the file, and closes the file.

```c++
void signalHandler(int sigNum);
```
The function `signalHandler()` takes a signal code sent to the server. If the signal is `SIGTERM` or `SIGQUIT`, program exits with code zero.

### Main Function

The main function of the server first processes the two command-line arguments, saving them as variables `port` (ERROR ocurs and server stops if `port` is an invalid number) and `path`. Then a TCP socket is created, binded (using `port`), and set to listen mode in the same way as the Project-1-Accio skeleton code. From here the server creates `int connectionID` to distinguish connections in the current session. The server will then accept a new connections, generate a file path of the form `path/connectionID.file`, receive and write the data sent from the connection using `receiveFile()`, and then closes the connection. It repeats this processes for new connections until 10 connections have been processed, a SIGNAL to stop the server is received, or an error occurs.

## Client Design

### Command-Line Arguments
```
$ ./server <HOSTNAME-OR-IP> <PORT> <FILENAME>
```
<ul>
    <li>&lt;HOSTNAME-OR-IP&gt; : the hostname or IP address of the server to connect. </li>
    <li>&lt;PORT&gt; : the port number of the server to connect. Must be a valid port number > 1023. </li>
    <li>&lt;FILENAME&gt; : the name of the file to be transfered to the server.</li>
</ul>

### Client Functions

```c++
void connectServer(int socket, int port, string ip);
```
The function `connectServer()` takes a socket, port number, and IP address, switches the socket to nonblocking mode and attempts to connect to the server, terminating the client with an error if the connection fails or a timeout occurs.

```c++
void sendFile(int socket, string fileName);
```
The function `sendFile()` takes a socket and a file name, opens the file for reading, reads and sends the data from the file over the connection, and then closes the file. If data cannot be sent to the server for over 10 seconds, a timeout occurs which aborts the connection and terminates the client.

```c++
bool validateIP(string ip);
```
The function `validateIP()` takes a string, returning `true` if the string is a valid IP address and `false` otherwise.
### Main Function
The main function of the server first processes (including validation for the first two) the three command-line arguments, saving them as variables `ip`, `port`, and `fileName`. Then a TCP socket is created and attempts to connect to the server using `connectServer()`. If successful, the client then sends the data of `fileName` using `sendFile()`. If successful, the TCP connection is aborted and the client is terminated.

## Problems

I encountered several problems of varying difficulty. The main issues that consumed most of my time were: <br/>
<ul>
    <li>implementing connection timeout using select()
        <ul>
            <li>I solved this with the help of <a href="https://stackoverflow.com/questions/17071338/select-for-timeout-over-tcp-socket">someone on Stack Overflow</a> who mentioned that the socket needed to be in non-blocking mode and how to implement that with fcntl().</li>
        </ul>
    </li>
    <li>figuring out and fixing my server for test cases 10 and 13 (server was not creating files in the right directory)
        <ul>
            <li>I found out that my server was not able to handle absolute paths (fopen() would only work with relative paths). I solved this by switching to fstream for handling my output file. </li>
        </ul>
    </li>
     <li> fixing a segmenation fault early on in the project
        <ul>
            <li>Embarrassingly (but comical enough for me to want to share), I discovered that I had named the input text file "file.txt" on my machine, meaning that it was stored as file.txt.txt. It took me a while to realize this, but when I did, the fix was (clearly) very easy.</li>
        </ul>
    </li>
</ul>

## Additional Libraries

I didn't use any nonstandard additional libraries. However, I did use the following additional (not in skeleton code) headers: 
<br/>
<ul>
    <li>&lt;fcntl.h&gt;</li>
    <li>&lt;fstream&gt;</li>
    <li>&lt;signal.h&gt;</li>
    <li>&lt;sys/select.h&gt;</li>
</ul>

## Acknowledgements

<ul>
    <li><a href="https://stackoverflow.com/questions/47573636/tcp-ip-file-transfer-c">Stack Overflow - TCP/IP file transfer C++</a></li>
    <li><a href="https://stackoverflow.com/questions/17071338/select-for-timeout-over-tcp-socket">Stack Overflow - select for timeout over TCP socket</a></li>
    <li><a href="https://man7.org/linux/man-pages/man2/select.2.html">man7.org - select(2)</a></li>
    <li><a href="https://man7.org/linux/man-pages/man2/fcntl.2.html">man7.org - fcntl(2)</a></li>
    <li><a href="https://www.techiedelight.com/validate-ip-address/#:~:text=The%20idea%20is%20to%20split,and%20255%2C%20we%20return%20fals">Techie Delight - Validate an IP address in C++</a></li>
    <li><a href="https://www.youtube.com/watch?v=RU0ULe2f6hI">Chris Kanich - Signal handling in Linux</a></li>
</ul>
